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
#include "tscsrpbuffer.h"
#define MAX_TSCSRP_BUFFER_SZ 768
#define NUM_TSCSRP_FRAGS 24
#define PV_TSC_SRP_BUFFER_TIMER_ID 1
#define TSC_SRP_BUFFER_TIMEOUT 50


TscSrpBufferLLPortIn *TscSrpBufferLLPortIn::NewL(TscSrpBuffer *aTscSrpBuffer)
{
    TscSrpBufferLLPortIn *self = OSCL_NEW(TscSrpBufferLLPortIn, (aTscSrpBuffer));
    if (self)
    {
        ConstructSelf(self);
    }
    else
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    return self;
}

void TscSrpBufferLLPortIn::ConstructSelf(TscSrpBufferLLPortIn* self)
{
    int error;
    OSCL_TRY(error, self->ConstructL(););
    if (error)
    {
        OSCL_DELETE(self);
        OSCL_LEAVE(error);
    }
}


TscSrpBufferLLPortIn::~TscSrpBufferLLPortIn()
{
    if (iFrag)
    {
        iMediaFragAlloc->deallocate_fragment(iFrag);
        iFrag = NULL;
    }

    if (iPkt)
    {
        iPkt->Clear();
        iMediaPktAlloc.deallocate_packet(iPkt);
        iPkt = NULL;
    }

    if (iMediaFragAlloc)
    {
        OSCL_DELETE(iMediaFragAlloc);
        iMediaFragAlloc = NULL;
    }
}

PVMFStatus TscSrpBufferLLPortIn::Receive(PVMFSharedMediaMsgPtr aMsg)
{
    OsclRefCounterMemFrag frag;
    PVMFSharedMediaDataPtr mediaData;
    convertToPVMFMediaData(mediaData, aMsg);
    uint32 fullH245Msg = mediaData->getMarkerInfo();

    if (iFrag == NULL)
    {
        iFrag = iMediaFragAlloc->allocate_fragment(MAX_TSCSRP_BUFFER_SZ);
        if (iFrag == NULL) return PVMFFailure;
    }

    for (uint32 i = 0; i < mediaData->getNumFragments(); i++)
    {
        if (mediaData->getMediaFragment(i, frag))
        {
            if ((frag.getMemFragSize() + iCurSize) > MAX_TSCSRP_BUFFER_SZ)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iTscSrpBuffer->iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "TscSrpBufferLLPortIn::PutData MAX_TSCSRP_BUFFER_SZ size exceeded"));
                fullH245Msg = true;
                break;
            }
            else
            {
                oscl_memcpy(iFrag->GetPtr() + iCurSize, frag.getMemFragPtr(),
                            frag.getMemFragSize());
                iCurSize += frag.getMemFragSize();
            }
        }
    }

    if (fullH245Msg)
    {
        iFrag->GetFragment()->len = iCurSize;
        iPkt->AddMediaFragment(iFrag);
        iMediaFragAlloc->deallocate_fragment(iFrag);
        iFrag = NULL;
        iCurSize = 0;

        iTscSrpBuffer->ProcessIncomingSrpPacket(iPkt);
        iPkt->Clear();
    }

    return PVMFSuccess;
}

void TscSrpBufferLLPortIn::ConstructL()
{
    iMediaFragAlloc = OSCL_NEW(PoolFragmentAllocator, (NUM_TSCSRP_FRAGS,
                               MAX_TSCSRP_BUFFER_SZ));
    OsclError::LeaveIfNull(iMediaFragAlloc);
    iPkt = iMediaPktAlloc.allocate_packet();
}

TscSrpBuffer *TscSrpBuffer::NewL()
{
    TscSrpBuffer *self = OSCL_NEW(TscSrpBuffer, ());
    if (self)
    {
        ConstructSelf(self);
    }
    else
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }

    return self;
}

void TscSrpBuffer::ConstructSelf(TscSrpBuffer* self)
{
    int error;
    OSCL_TRY(error, self->ConstructL(););
    if (error)
    {
        OSCL_DELETE(self);
        OSCL_LEAVE(error);
    }
}


TscSrpBuffer::~TscSrpBuffer()
{
    iTxData.Unbind();

    if (iLLPortIn)
    {
        OSCL_DELETE(iLLPortIn);
        iLLPortIn = NULL;
    }

    if (iLLPortOut)
    {
        OSCL_DELETE(iLLPortOut);
        iLLPortOut = NULL;
    }

    if (iTimer)
    {
        iTimer->Clear();
        OSCL_DELETE(iTimer);
        iTimer = NULL;
    }

    if (iTxPacketAlloc)
    {
        OSCL_DELETE(iTxPacketAlloc);
        iTxPacketAlloc = NULL;
    }

    if (iTxMediaDataImplMemAlloc)
    {
        OSCL_DELETE(iTxMediaDataImplMemAlloc);
        iTxMediaDataImplMemAlloc = NULL;
    }

    if (iTxMediaMsgPoolAlloc)
    {
        OSCL_DELETE(iTxMediaMsgPoolAlloc);
        iTxMediaMsgPoolAlloc = NULL;
    }
}

void TscSrpBuffer::ProcessIncomingSrpPacket(MediaPacket* pPkt)
{
    iH245Interface.Dispatch(pPkt);
}

void TscSrpBuffer::ProcessOutgoingH245Packet(MediaPacket* pPkt)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TscSrpBuffer::ProcessOutgoingH245Packet instance=%x, pPkt=%x",
                     this, pPkt));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TscSrpBuffer::ProcessOutgoingH245Packet pPkt length=%d",
                     pPkt->GetLength()));

    if (iState != TscSrpBufferStarted)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TscSrpBuffer::ProcessOutgoingH245Packet - Error: Message received in stopped state"));
        return;
    }

    //Allocate message and data
    if (iTxData.GetRep() == NULL)
    {
        OsclSharedPtr<PVMFMediaDataImpl> data;

        data = iTxPacketAlloc->allocate(MAX_TSCSRP_BUFFER_SZ);
        if (!data)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                            PVLOGMSG_ERR, (0,
                                           "TscSrpBuffer::ProcessOutgoingH245Packet - Unable allocate tx message impl"));
            return;
        }

        iTxData = PVMFMediaData::createMediaData(data, iTxMediaMsgPoolAlloc);
        if (!iTxData)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger,
                            PVLOGMSG_ERR, (0,
                                           "TscSrpBuffer::ProcessOutgoingH245Packet - Unable allocate tx message"));
            return;
        }
    }

    iNumMsgs++;

    /* Just add the fragments to our fragment */
    MediaFragment frag;
    OsclRefCounterMemFrag osclFrag;
    uint32 curSize = iTxData->getFilledSize();
    iTxData->getMediaFragment(0, osclFrag);

    for (int frag_num = 0; frag_num < pPkt->GetNumFrags(); frag_num++)
    {
        pPkt->GetMediaFragment(frag_num, frag);

        if ((curSize + frag.GetLen()) > MAX_TSCSRP_BUFFER_SZ)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "TscSrpBuffer::ProcessOutgoingH245Packet - Error: Buffer size exceeded"));
            /* reset the buffer */
            Reset();
            return;
        }
        oscl_memcpy((uint8 *)osclFrag.getMemFragPtr() + curSize, frag.GetPtr(),
                    frag.GetLen());
        curSize += frag.GetLen();
    }

    iTxData->setMediaFragFilledLen(0, curSize);

    if (iEnableBuffering)
    {
        if (iNumMsgs == 1)
        {
            // start the timer
            iTimer->Request(PV_TSC_SRP_BUFFER_TIMER_ID,
                            PV_TSC_SRP_BUFFER_TIMER_ID , 1, this);
        }
    }
    else
    {
        // Cancel the timer
        iTimer->Cancel(PV_TSC_SRP_BUFFER_TIMER_ID);
        // Call the dispatch routine immediately
        TimeoutOccurred(PV_TSC_SRP_BUFFER_TIMER_ID, 0);
    }
}

void TscSrpBuffer::TimeoutOccurred(int32 timerID, int32 timeoutInfo)
{
    OSCL_UNUSED_ARG(timerID);
    OSCL_UNUSED_ARG(timeoutInfo);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TscSrpBuffer::TimeoutOccurred- iNumMsgs(%d), size(%d)\n",
                     iNumMsgs, iTxData->getFilledSize()));
    OSCL_ASSERT(iNumMsgs);

    /* Dispatch message to SRP */
    if (iNumMsgs)
    {
        PVMFSharedMediaMsgPtr msg;

        convertToPVMFMediaMsg(msg, iTxData);

        iNumMsgs = 0;
        iTxData.Unbind();
        iLLPortOut->QueueOutgoingMsg(msg);
        iLLPortOut->Send();
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TscSrpBuffer::TimeoutOccurred- done"));

}

void TscSrpBuffer::Start()
{
    iState = TscSrpBufferStarted;
}

void TscSrpBuffer::Stop()
{
    iState = TscSrpBufferStopped;
    iTimer->Clear();
    Reset();
}

void TscSrpBuffer::Reset()
{
    iTxData.Unbind();
    iNumMsgs = 0;
}

void TscSrpBuffer::EnableBuffering(bool enable)
{
    iEnableBuffering = enable;
}

void TscSrpBuffer::ConstructL()
{
    iLogger = PVLogger::GetLoggerObject("3g324m.srp.tscsrpbuffer");

    iTxMediaMsgPoolAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator,
                                    (NUM_TSCSRP_FRAGS, 0, &iMemAllocator));
    OsclError::LeaveIfNull(iTxMediaMsgPoolAlloc);
    iTxMediaDataImplMemAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator,
                                        (NUM_TSCSRP_FRAGS, 0, &iMemAllocator));
    OsclError::LeaveIfNull(iTxMediaDataImplMemAlloc);
    iTxPacketAlloc = OSCL_NEW(PVMFSimpleMediaBufferCombinedAlloc,
                              (iTxMediaDataImplMemAlloc));
    OsclError::LeaveIfNull(iTxPacketAlloc);

    iTimer = new OsclTimer<OsclMemAllocator>("TSCSRPBufferTimer");
    OsclError::LeaveIfNull(iTimer);
    iTimer->SetFrequency(1000 / TSC_SRP_BUFFER_TIMEOUT); // make timer work in 20ms intervals
    iTimer->SetObserver(this);

    iLLPortOut = OSCL_NEW(TscSrpBufferLLPortOut, ());
    OsclError::LeaveIfNull(iLLPortOut);
    iLLPortIn = TscSrpBufferLLPortIn::NewL(this);

    iH245Interface = this;
}



