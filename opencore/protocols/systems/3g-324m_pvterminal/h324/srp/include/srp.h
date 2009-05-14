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

#ifndef SRP_H
#define SRP_H

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#ifndef PVMF_PORT_INTERFACE_H_INCLUDED
#include "pvmf_port_interface.h"
#endif

#ifndef PVMF_MEDIA_FRAG_GROUP_H_INCLUDED
#include "pvmf_media_frag_group.h"
#endif

#ifndef PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED
#include "pvmf_pool_buffer_allocator.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#include "crccheck_cpp.h"


/* SRP frame information */
#define SRP_HEADER_SIZE		1
#define SRP_SEQUENCE_SIZE	1
#define SRP_FCS_SIZE		2
#define CCSRL_HEADER_SIZE	1
#define SRP_COMMAND_HEADER	0xF9
#define SRP_RESPONSE_HEADER	0xFB
#define NSRP_RESPONSE_HEADER 0xF7
#define MINIMUM_FRAME_SIZE  SRP_HEADER_SIZE+SRP_FCS_SIZE
#define DEFAULT_CCSRL_SIZE	256
#define INTERMEDIATE_CCSRL_CHUNK 0x00
#define LAST_CCSRL_CHUNK 0xFF
#define DEFAULT_SEND_QUEUE_SIZE 24
#define MAX_SEND_FRAGS_PER_MSG 4 //Header + CCSRL + data + FCS frags
#define MAX_RESP_MSGS 24

/* CRC definition */
#define  GX_CRC_12    0x1021  /* CRC Generate matrix ( G(X) = X^16 + X^12 + X^5 + 1Åj */


/* Fragment pool definition */
#define MAX_SIZE_OF_SRP_HEADER (SRP_HEADER_SIZE+SRP_SEQUENCE_SIZE+SRP_FCS_SIZE)
#define MAX_SIZE_OF_SRP_PDU 1024 /* Thie is the AL1 SDU size */

/* WNSRP definitions */
#define WNSRP_COMMAND_HEADER 0xF1
#define WNSRP_RESPONSE_HEADER 0xF3
#define DEFAULT_WNSRP_WINDOW 5
#define N402_DEFAULT_MAX_COUNTER 3 /* Number of SRP/NSRP frames to accept before dropping out of WNSRP mode */

/* Timer definition */
#define N400_DEFAULT_MAX_COUNTER 100
#define T401_PRIMARY 	WNSRP_WINDOW /* Primary SRP timer id, set it to the WNSRP window, id for the WNSRP timers will be the array index */
#define T401_RESOLUTION 10  // Set SRP timeout to 100ms
#define T401_VALUE_DEFAULT 2 // Sets the default T401 timeout to 200ms

/* Multiplex table entry definitions */
#define SRP_NSRP_MT_ENTRY_NUMBER 0
#define WNSRP_MT_ENTRY_NUMBER 15
#define NUM_MT_ENTRY_NUMBERS 2 //Number of possible MT entry numbers SRP can send on (SRP/NSRP = 0, WNSRP = 15)
#define MT_ENTRY_NUMBER_SIZE sizeof(uint8)

/* TBD */
#define SRP_INPUT_PORT_TAG 0
#define SRP_OUTPUT_PORT_TAG 1

class SRPStats
{
    public:
        SRPStats() : totalFramesRecv(0),
                totalInvalidSizeFramesRecv(0),
                totalCRCErrorsRecv(0),
                totalCommandFramesRecv(0),
                totalWNSRPCommandFramesRecv(0),
                totalInvalidCCSRLChunkRecv(0),
                totalSRPRespRecv(0),
                totalNSRPRespRecv(0),
                totalWNSRPRespRecv(0),
                totalH245MessagesRecv(0),
                totalCCSRLChunksRecv(0),
                totalCommandFramesSent(0),
                totalH245MessagesToSend(0),
                totalH245MessagesFragmented(0),
                totalCCSRLChunksSent(0),
                totalNumTimeouts(0),
                totalBytesRecv(0),
                totalBytesSent(0),
                totalInvalidFrames(0),
                minRespTime(0xFFFFFFFF),
                maxRespTime(0),
                aveRespTime(0) {};

        ~SRPStats() {};

        void Reset()
        {
            totalFramesRecv = 0;
            totalInvalidSizeFramesRecv = 0;
            totalCRCErrorsRecv = 0;
            totalCommandFramesRecv = 0;
            totalWNSRPCommandFramesRecv = 0;
            totalInvalidCCSRLChunkRecv = 0;
            totalSRPRespRecv = 0;
            totalNSRPRespRecv = 0;
            totalWNSRPRespRecv = 0;
            totalH245MessagesRecv = 0;
            totalCCSRLChunksRecv = 0;
            totalCommandFramesSent = 0;
            totalH245MessagesToSend = 0;
            totalH245MessagesFragmented = 0;
            totalCCSRLChunksSent = 0;
            totalNumTimeouts = 0;
            totalBytesRecv = 0;
            totalBytesSent = 0;
            totalInvalidFrames = 0;
            minRespTime = 0xFFFFFFFF;
            maxRespTime = 0;
            aveRespTime = 0;
        }

        SRPStats &operator=(SRPStats &a)
        {
            totalFramesRecv = a.totalFramesRecv;
            totalInvalidSizeFramesRecv = a.totalInvalidSizeFramesRecv;
            totalCRCErrorsRecv = a.totalCRCErrorsRecv;
            totalCommandFramesRecv = a.totalCommandFramesRecv;
            totalWNSRPCommandFramesRecv = a.totalWNSRPCommandFramesRecv;
            totalInvalidCCSRLChunkRecv = a.totalInvalidCCSRLChunkRecv;
            totalSRPRespRecv = a.totalSRPRespRecv;
            totalNSRPRespRecv = a.totalNSRPRespRecv;
            totalWNSRPRespRecv = a.totalWNSRPRespRecv;
            totalH245MessagesRecv = a.totalH245MessagesRecv;
            totalCCSRLChunksRecv = a.totalCCSRLChunksRecv;
            totalCommandFramesSent = a.totalCommandFramesSent;
            totalH245MessagesToSend = a.totalH245MessagesToSend;
            totalH245MessagesFragmented = a.totalH245MessagesFragmented;
            totalCCSRLChunksSent = a.totalCCSRLChunksSent;
            totalNumTimeouts = a.totalNumTimeouts;
            totalBytesRecv = a.totalBytesRecv;
            totalBytesSent = a.totalBytesSent;
            totalInvalidFrames = a.totalInvalidFrames;
            minRespTime = a.minRespTime;
            maxRespTime = a.maxRespTime;
            aveRespTime = a.aveRespTime;
            return *this;
        }

        uint32 totalFramesRecv;
        uint32 totalInvalidSizeFramesRecv;
        uint32 totalCRCErrorsRecv;
        uint32 totalCommandFramesRecv;
        uint32 totalWNSRPCommandFramesRecv;
        uint32 totalInvalidCCSRLChunkRecv;
        uint32 totalSRPRespRecv;
        uint32 totalNSRPRespRecv;
        uint32 totalWNSRPRespRecv;
        uint32 totalH245MessagesRecv;
        uint32 totalCCSRLChunksRecv;
        uint32 totalCommandFramesSent;
        uint32 totalH245MessagesToSend;
        uint32 totalH245MessagesFragmented;
        uint32 totalCCSRLChunksSent;
        uint32 totalNumTimeouts;
        uint32 totalBytesRecv;
        uint32 totalBytesSent;
        uint32 totalInvalidFrames;
        uint32 minRespTime;
        uint32 maxRespTime;
        uint32 aveRespTime;
};


class SRPObserver
{
    public:
        virtual ~SRPObserver() {}
        virtual void TransmissionFailure() = 0;
        virtual void UseWNSRP(bool aUse) = 0;
};

typedef OsclSharedPtr<PVMFMediaDataImpl> PVMFSharedMediaDataImplPtr;

class SRP : public OsclTimerObserver
{
    public:
        /* structures used by SRP class */

        enum SrpStatus {STS_IDLE = 0, STS_WAIT_RESPONSE, MAX_STATUS};
        enum SrpEvent {EV_NOP = 0,			/* Unknown Event */
                       EV_PDU,				/* PDU message */
                       EV_COMMAND,			/* PRIMITIVE(DATA-COMMAND) */
                       EV_RESPONSE,			/* PRIMITIVE(DATA-RESPONSE) */
                       EV_NSRP_RESPONSE,	/* PRIMITIVE NSRP(DATA-RESPONSE) */
                       EV_TIMEOUT,			/* Response wait timeout */
                       EV_WNSRP_COMMAND,	/* WNSRP(DATA-COMMAND) */
                       EV_WNSRP_RESPONSE,	/* WNSRP(DATA-RESPONSE) */
                       MAX_EVENT
                  };
        enum SrpAction {ACTION_NOP = 0, ACTION_0_1, ACTION_0_2, ACTION_1_1, ACTION_1_3, ACTION_1_4};

        enum WnsrpStatus {NO_WNSRP_SUPPORT, WNSRP_TX_SUPPORT, WNSRP_FULL_SUPPORT};



        /* methods */
        OSCL_IMPORT_REF SRP() ;
        OSCL_IMPORT_REF virtual ~SRP() ;

        OSCL_IMPORT_REF void SetObserver(SRPObserver *pObserver)
        {
            iObserver = pObserver;
        }

        /* srp_main.cpp */
        OSCL_IMPORT_REF void SrpInitL(void);   /* Initialization method */
        OSCL_IMPORT_REF void SrpReset(void);   /* Shutdown method */

        OSCL_IMPORT_REF void SrpStart(void);   /* Restart SRP operations after stop. */
        OSCL_IMPORT_REF void SrpStop(void);   /* Halt SRP operations, stop timers, flush queue, do not shutdown. */

        /* srp_tim.cpp */
        void TimeoutOccurred(int32 timerID, int32 timeoutInfo);

        void LowerLayerRx(PVMFSharedMediaDataPtr aData);
        void UpperLayerRx(PVMFSharedMediaDataPtr aData);

        void SetWNSRPTxWindow(uint32 window)
        {
            iWNSRPTxWindow = window;
        }
        void SetWNSRPRxWindow(uint32 window)
        {
            iWNSRPRxWindow = window;
        }
        void SetCCSRLSduSize(int size)
        {
            iCcsrlSduSize = size;
        }

        void UseNSRP(bool aUseNsrp = true);
        void DisableWNSRPSupport();

        void SetNumTxMsgs(uint32 numMsgs)
        {
            iNumTxMsgs = numMsgs;
        }

        void SrpResetStats();
        void SrpGetStats(SRPStats &aStats);

        //Number or times to retry sending SRP command before notifying upper layer of failure.
        void SetNumSRPRetries(int maxRetries)
        {
            iN400MaxCounter = maxRetries;
        }
        //Actual timeout is (resolution * value).
        void SetSRPTimeoutValue(int value)
        {
            iT401TimerValue = value;
        }



        PVMFPortInterface * RequestLLPort(const int32 aPortTag)
        {
            if (aPortTag == SRP_INPUT_PORT_TAG)
            {
                return iLLPortIn;
            }
            if (aPortTag == SRP_OUTPUT_PORT_TAG)
            {
                return iLLPortOut;
            }

            return NULL;
        }

        PVMFPortInterface * RequestULPort(const int32 aPortTag)
        {
            if (aPortTag == SRP_INPUT_PORT_TAG)
            {
                return iULPortIn;
            }
            if (aPortTag == SRP_OUTPUT_PORT_TAG)
            {
                return iULPortOut;
            }

            return NULL;
        }

        WnsrpStatus WnsrpStatusGet(void)
        {
            return(iCurWnsrpStatus);
        }
    private:

        class SRPRxData
        {
            public:
                SRPRxData() : seqNum(0),
                        next(NULL)
                {};

                uint8 seqNum;
                PVMFSharedMediaDataPtr data;
                SRPRxData *next;
        };

        bool AllocateRxPacket(OsclSharedPtr<PVMFMediaDataImpl>& data, SRPRxData* rxData);
        bool CreateMediaData(SRPRxData* rxData, OsclSharedPtr<PVMFMediaDataImpl> data);
        bool Allocate(OsclSharedPtr<PVMFMediaDataImpl>& data, OsclRefCounterMemFrag& CCSRLFrag);
        bool CreateMediaData(PVMFSharedMediaDataPtr& srpPkt,
                             OsclSharedPtr<PVMFMediaDataImpl> data);

        class SRPRespTimer
        {
            public:
                SRPRespTimer() : T401Timer("SRPTimer"),
                        timerID(0),
                        N400Counter(0),
                        isTimerActive(false)
                {};

                ~SRPRespTimer() {};

                OsclTimer<OsclMemAllocator> T401Timer;
                int32 timerID;
                int	N400Counter;
                bool isTimerActive;
                PVMFSharedMediaDataPtr pPkt;
        };

        /* srp_cnt.cpp */
        int SrpN400Check(SRPRespTimer &timer);

        /* WNSRP counter */
        void SrpN402Initialize(void);
        void SrpN402Clear(void)
        {
            iN402Counter = 0;
        }
        void SrpN402Count(void)
        {
            ++iN402Counter;
        }
        int SrpN402Check();

        /* srp_crc.cpp */
        void CrcClear(void)
        {
            iCrcData = 0;
        }
        void CrcCalc(uint8 *pData, int Size);
        uint16 CrcResultGet(void);
        void CrcSetToFrame(uint16 crc, uint8 *pFcs);

        /* srp_edit.cpp */
        PVMFSharedMediaDataPtr SrpCommandCreate(PVMFSharedMediaDataPtr pPkt, uint8 header);
        void SrpCommandClear(uint32 seqNum);
        PVMFSharedMediaDataPtr SrpResponseCreate(uint8 seqNum, uint8 header);

        /* srp_main.cpp */
        SrpEvent SrpGetEventNoFromAL1(PVMFSharedMediaDataPtr pPkt);

        bool SrpMsgCopy(PVMFSharedMediaDataPtr inPkt);

        /* srp_prot.cpp */
        int SrpNop()
        {
            return 1;
        }
        int Srp_0_1();
        int Srp_0_2(PVMFSharedMediaDataPtr pPkt, int event);
        int Srp_1_1();
        int Srp_1_3(PVMFSharedMediaDataPtr pPkt, int event);
        int Srp_1_4(int32 id);

        /* srp_que.cpp */
        void SrpSendWaitSet(PVMFSharedMediaDataPtr pPkt)
        {
            iSrpWaitQueue.push_back(pPkt);
        }
        PVMFSharedMediaDataPtr SrpSendWaitGet(uint32 index);
        int SrpSendWaitRemove(uint32 index);

        /* srp_seq.cpp */
        void SrpSendSeqClear(void)
        {
            iSendSeqNumber = 0;
        }
        void SrpSendSeqCount(void);
        uint32 SrpSendSeqGet(void)
        {
            return(iSendSeqNumber);
        }
        void SrpRecvSeqClear(void)
        {
            iRecvSeqNumber = 0;
            iOldestWNSRPRetransSeqNum = (iRecvSeqNumber - ((256 - iWNSRPRxWindow) / 2)) % 256;
        }
        void SrpRecvSeqCount(void);
        uint32 SrpRecvSeqGet(void)
        {
            return(iRecvSeqNumber);
        }
        void SrpRecvSeqSet(uint8 seqNo)
        {
            iRecvSeqNumber = seqNo;
        }

        /* srp_sts.cpp */
        void SrpStatusSet(SrpStatus newStatus) ;
        SrpStatus SrpStatusGet(void)
        {
            return(iStatus);
        }

        /* WNSRP status */
        void WnsrpStatusSet(WnsrpStatus newStatus) ;

        /* srp_tim.cpp */
        void SrpT401Start(SRPRespTimer &timer, PVMFSharedMediaDataPtr pPkt);
        void SrpT401Stop(SRPRespTimer &timer);


        /* Layer sendto interfaces.*/
        void LowerLayerTx(PVMFSharedMediaDataPtr pPkt)
        {
            if (iLLPortOut)
            {
                iStats.totalBytesSent += pPkt->getFilledSize();

                PVMFSharedMediaMsgPtr mediaMsg;
                convertToPVMFMediaMsg(mediaMsg, pPkt);
                iLLPortOut->QueueOutgoingMsg(mediaMsg);
                iLLPortOut->Send();
            }
        }
        void UpperLayerTx(PVMFSharedMediaDataPtr pPkt)
        {
            if (iULPortOut)
            {
                iStats.totalBytesSent += pPkt->getFilledSize();

                PVMFSharedMediaMsgPtr mediaMsg;
                convertToPVMFMediaMsg(mediaMsg, pPkt);
                iULPortOut->QueueOutgoingMsg(mediaMsg);
                iULPortOut->Send();
            }
        }

        int SrpStateChange(int status, int event, void *data);

        void UpdateRespStats();

        SRPRespTimer *GetFreeWnsrpTimer();
        void FreeWnsrpTimer(SRPRespTimer *timer)
        {
            iFreeWnsrpCommandSave.push_back(timer);
        }
        void AddActiveWnsrpTimer(SRPRespTimer *timer)
        {
            iActiveWnsrpCommandSave.push_back(timer);
        }
        SRPRespTimer *FindActiveWnsrpTimer(int32 timerID);
        void RemoveActiveWnsrpTimer(SRPRespTimer *timer);

        void FreeRxFrag(SRPRxData *frag);
        SRPRxData *GetRxFrag();

        void AddPendingRxFrag(SRPRxData *frag);
        bool CheckRxSeqNum(uint32 seqNum, uint8 header);
        bool CheckPendingRxList(uint32 seqNum);
        bool CheckWNSRPRetrans(uint32 seqNum);

        static const SRP::SrpAction iSrpStateTable[MAX_STATUS][MAX_EVENT];

        /* Counter data */
        int	iN400MaxCounter;

        /* WNSRP counter data */
        int	iN402MaxCounter;
        int	iN402Counter;

        /* CRC data */
        CRC iCrc;
        uint32	iCrcData;

        /* Queue data */
        Oscl_Vector<PVMFSharedMediaDataPtr, OsclMemAllocator> iSrpWaitQueue;

        /* Seq data */
        uint32 iSendSeqNumber;
        uint32 iRecvSeqNumber;

        /* Status data */
        SrpStatus	iStatus;
        WnsrpStatus iCurWnsrpStatus;

        /* Timer data */
        int	iT401TimerValue;

        /* Commands that have not been acknowledged yet */
        SRPRespTimer iSrpCommandSave;
        SRPRespTimer *iWnsrpCommandSave;
        Oscl_Vector<SRPRespTimer *, OsclMemAllocator> iFreeWnsrpCommandSave;
        Oscl_Vector<SRPRespTimer *, OsclMemAllocator> iActiveWnsrpCommandSave;

        /* CCSRL data */
        unsigned int iCcsrlSduSize;

        /* Received packet data */
        SRPRxData *iRxFrags;
        SRPRxData *iFreeRxFragList;
        SRPRxData *iPendingRxFragList;


        /* WWU_CCSRL: add end */
        bool iFirstCmd;	/* WWU_RB8 */

        PVMFPortInterface *iLLPortOut;
        PVMFPortInterface *iLLPortIn;
        PVMFPortInterface *iULPortOut;
        PVMFPortInterface *iULPortIn;

        OsclMemAllocator iMemAllocator;

        OsclMemPoolFixedChunkAllocator* iTxMediaMsgPoolAlloc;
        OsclMemPoolFixedChunkAllocator* iTxMediaDataImplMemAlloc;
        PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>* iTxPacketAlloc;

        OsclMemPoolFixedChunkAllocator* iOutstandingTxMediaMsgPoolAlloc;
        OsclMemPoolFixedChunkAllocator* iOutstandingTxMediaDataImplMemAlloc;

        OsclMemPoolFixedChunkAllocator* iRxMediaMsgPoolAlloc;
        OsclMemPoolFixedChunkAllocator* iRxMediaDataImplMemAlloc;
        PVMFSimpleMediaBufferCombinedAlloc *iRxPacketAlloc;

        OsclMemPoolFixedChunkAllocator* iRespMediaMsgPoolAlloc;
        OsclMemPoolFixedChunkAllocator* iRespMemAlloc;
        PVMFSimpleMediaBufferCombinedAlloc *iRespPacketAlloc;
        OsclMemPoolFixedChunkAllocator* iNsrpRespMemAlloc;
        PVMFSimpleMediaBufferCombinedAlloc *iNsrpRespPacketAlloc;

        PVMFBufferPoolAllocator iHdrAllocator;
        PVMFBufferPoolAllocator iCcsrlAllocator;
        PVMFBufferPoolAllocator iFCSAllocator;
        PVMFBufferPoolAllocator iMTEntryNumAllocator;

        OsclRefCounterMemFrag iSrpNsrpEntryNumFrag;
        OsclRefCounterMemFrag iWnsrpEntryNumFrag;

        uint32 iNumTxMsgs;

        bool iUseNSRP;
        //If first NSRP response message was received.
        bool iFirstNSRPResp;

        //For testing purposes.  Always should support receiving WNSRP messages.
        bool iHandleWNSRP;
        uint32 iWNSRPTxWindow;
        uint32 iWNSRPRxWindow;
        uint32 iOldestWNSRPRetransSeqNum;

        int iT401Resolution;
        SRPObserver* iObserver;

        SRPStats iStats;
        uint32 iTotalRespTime;
        TimeValue iInitialTimeChunkSent;

        PVLogger *iLogger;
};

#endif

