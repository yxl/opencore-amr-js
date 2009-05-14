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
#ifndef PVMF_JB_FIREWALL_PKTS_IMPL_H_INCLUDED
#include "pvmf_jb_firewall_pkts_impl.h"
#endif

#ifndef OSCL_EXCEPTION_H_INCLUDED
#include "oscl_exception.h"
#endif

#ifndef OSCL_BIN_STREAM_H_INCLUDED
#include "oscl_bin_stream.h"
#endif

#ifndef PVMF_JB_JITTERBUFFERMISC_H_INCLUDED
#include "pvmf_jb_jitterbuffermisc.h"
#endif

///////////////////////////////////////////////////////////////////////////////
//PVFirewallPacketExchanger
///////////////////////////////////////////////////////////////////////////////
PVFirewallPacketExchanger* PVFirewallPacketExchanger::New(const RTPSessionInfoForFirewallExchange& aRTPSessionInfo)
{
    int32 err = OsclErrNone;
    PVFirewallPacketExchanger* pExchanger = NULL;
    OSCL_TRY(err,
             pExchanger = OSCL_NEW(PVFirewallPacketExchanger, (aRTPSessionInfo));
             pExchanger->Construct();
            );

    if (pExchanger && OsclErrNone != err)
    {
        OSCL_DELETE(pExchanger);
        pExchanger = NULL;
    }

    return pExchanger;
}

void PVFirewallPacketExchanger::Construct()
{
    CreateMemAllocators();
}

void PVFirewallPacketExchanger::CreateMemAllocators()
{
    ipMediaDataAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (PVMF_JITTER_BUFFER_NODE_FIREWALL_PKT_MEMPOOL_SIZE));
    ipMediaDataImplAlloc = OSCL_NEW(PVMFSimpleMediaBufferCombinedAlloc, (ipMediaDataAlloc));
    ipMediaMsgAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (PVMF_JITTER_BUFFER_NODE_FIREWALL_PKT_MEMPOOL_SIZE, PVMF_JITTER_BUFFER_NODE_MEDIA_MSG_SIZE));

    if (!(ipMediaDataAlloc && ipMediaDataImplAlloc && ipMediaMsgAlloc))
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
}

PVFirewallPacketExchanger::~PVFirewallPacketExchanger()
{
    DestroyMemoryAllocators();
}

void PVFirewallPacketExchanger::DestroyMemoryAllocators()
{
    if (ipMediaMsgAlloc)
    {
        OSCL_DELETE(ipMediaMsgAlloc);
        ipMediaMsgAlloc = NULL;
    }

    if (ipMediaDataImplAlloc)
    {
        OSCL_DELETE(ipMediaDataImplAlloc);
        ipMediaDataImplAlloc = NULL;
    }

    if (ipMediaDataAlloc)
    {
        OSCL_DELETE(ipMediaDataAlloc);
        ipMediaDataAlloc = NULL;
    }
}

bool PVFirewallPacketExchanger::Allocate(PVMFSharedMediaDataPtr& aFireWallPkt, OsclSharedPtr<PVMFMediaDataImpl>& aMediaDataImpl, const int aSize)
{
    int32 err = OsclErrNone;
    OSCL_TRY(err,
             aMediaDataImpl = ipMediaDataImplAlloc->allocate(aSize);
             aFireWallPkt = PVMFMediaData::createMediaData(aMediaDataImpl,
                            ipMediaMsgAlloc);
            );
    if (err != OsclErrNone)
    {
        return false;
    }
    return true;
}

bool PVFirewallPacketExchanger::ComposeFirewallPacket(PVMFJitterBufferFireWallPacketFormat aFormat, uint32 aPacketCnt, PVMFPortInterface*& aRTPJitterBufferPort, PVMFSharedMediaMsgPtr& aSharedMediaMsg)
{
    PVMFSharedMediaMsgPtr fireWallMsg;
    PVMFSharedMediaDataPtr fireWallPkt;
    OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl;

    if (aFormat == PVMF_JB_FW_PKT_FORMAT_PV)
    {
        bool retval = Allocate(fireWallPkt, mediaDataImpl, PVMF_JITTER_BUFFER_NODE_MAX_FIREWALL_PKT_SIZE);

        if (retval == false)
        {
            return retval;
        }

        fireWallPkt->setMediaFragFilledLen(0, PVMF_JITTER_BUFFER_NODE_MAX_FIREWALL_PKT_SIZE);

        OsclRefCounterMemFrag refCntMemFrag;
        mediaDataImpl->getMediaFragment(0, refCntMemFrag);

        OsclMemoryFragment memFrag = refCntMemFrag.getMemFrag();
        OsclBinOStreamBigEndian outstream;

        outstream.Attach(1, &memFrag);

        outstream << aPacketCnt;
        outstream << iRTPSessionInfoForFirewallExchange.iSSRC;
    }
    else
    {
        bool retval = Allocate(fireWallPkt, mediaDataImpl, PVMF_JITTER_BUFFER_NODE_MAX_RTP_FIREWALL_PKT_SIZE);

        if (retval == false)
        {
            return retval;
        }

        fireWallPkt->setMediaFragFilledLen(0, PVMF_JITTER_BUFFER_NODE_MAX_RTP_FIREWALL_PKT_SIZE);

        OsclRefCounterMemFrag refCntMemFrag;
        mediaDataImpl->getMediaFragment(0, refCntMemFrag);

        OsclMemoryFragment memFrag = refCntMemFrag.getMemFrag();
        oscl_memset(memFrag.ptr, 0, memFrag.len);

        OsclBinOStreamBigEndian outstream;
        outstream.Attach(1, &memFrag);

        //Skip to start of SSRC
        outstream.seekFromCurrentPosition(8);

        //fill in the SSRC
        outstream << iRTPSessionInfoForFirewallExchange.iSSRC;
    }

    convertToPVMFMediaMsg(aSharedMediaMsg, fireWallPkt);

    aRTPJitterBufferPort = iRTPSessionInfoForFirewallExchange.ipRTPDataJitterBufferPort;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//PVFirewallPacketExchangeImpl
///////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVFirewallPacketExchangeImpl* PVFirewallPacketExchangeImpl::New(PVMFJitterBufferFireWallPacketInfo& aFireWallPacketExchangeInfo, PVMFJBEventNotifier& aEventNotifier, PVMFJitterBufferMiscObserver* aObserver)
{
    int32 err = OsclErrNone;
    PVFirewallPacketExchangeImpl* pFirewallpacketExchangeImpl = NULL;
    OSCL_TRY(err,
             pFirewallpacketExchangeImpl = OSCL_NEW(PVFirewallPacketExchangeImpl, (aFireWallPacketExchangeInfo, aEventNotifier, aObserver));
             pFirewallpacketExchangeImpl->Construct();
            );
    if (OsclErrNone != err && pFirewallpacketExchangeImpl)
    {
        OSCL_DELETE(pFirewallpacketExchangeImpl);
    }
    return pFirewallpacketExchangeImpl;
}

void PVFirewallPacketExchangeImpl::Construct()
{
    ipDataPathLoggerFireWall = PVLogger::GetLoggerObject("PVFirewallPacketExchangeImpl");
}

OSCL_EXPORT_REF PVFirewallPacketExchangeImpl::~PVFirewallPacketExchangeImpl()
{
    Oscl_Vector<PVFirewallPacketExchanger*, OsclMemAllocator>::iterator iter;
    for (iter = iFirewallPacketExchangers.begin(); iter != iFirewallPacketExchangers.end(); iter++)
    {
        OSCL_DELETE(*iter);
        *iter = NULL;
    }
}

OSCL_EXPORT_REF void PVFirewallPacketExchangeImpl::SetRTPSessionInfoForFirewallExchange(const RTPSessionInfoForFirewallExchange& aRTPSessionInfo)
{
    PVFirewallPacketExchanger* pFirewallPacketExchanger = PVFirewallPacketExchanger::New(aRTPSessionInfo);
    if (pFirewallPacketExchanger)
        iFirewallPacketExchangers.push_back(pFirewallPacketExchanger);
}

OSCL_EXPORT_REF PVMFStatus PVFirewallPacketExchangeImpl::InitiateFirewallPacketExchange()
{
    iNumAttemptsDone = 0;
    if (iNumAttemptsDone < iFireWallPacketExchangeInfo.iNumAttempts)
    {
        SendFirewallPackets();
    }
    else
    {
        ipObserver->MediaReceivingChannelPrepared(true);
    }
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVFirewallPacketExchangeImpl::CancelFirewallPacketExchange()
{
    if (iCallBackPending)
    {
        PVMFJBEventNotificationRequestInfo requestInfo(CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING, this, NULL);
        irEventNotifier.CancelCallBack(requestInfo, iCallBackId);
        iCallBackId = 0;
        iCallBackPending = false;
    }
    iNumAttemptsDone = 0;
    return PVMFSuccess;
}

void PVFirewallPacketExchangeImpl::ProcessCallback(CLOCK_NOTIFICATION_INTF_TYPE aClockNotificationInterfaceType, uint32 aCallBkId, const OsclAny* aContext, PVMFStatus aStatus)
{
    OSCL_UNUSED_ARG(aClockNotificationInterfaceType);
    OSCL_UNUSED_ARG(aContext);
    if (PVMFSuccess == aStatus)
    {
        if (aCallBkId == iCallBackId)
        {
            iCallBackPending = false;
            SendFirewallPackets();
        }
    }
    else
    {
        //Log it <Assert?>
    }
}

void PVFirewallPacketExchangeImpl::SendFirewallPackets()
{
    Oscl_Vector<PVFirewallPacketExchanger*, OsclMemAllocator>::iterator iter;
    for (iter = iFirewallPacketExchangers.begin(); iter != iFirewallPacketExchangers.end(); iter++)
    {
        PVFirewallPacketExchanger* pFirewallpacketExchanger = *iter;
        PVMFPortInterface* pPortInterface = NULL;
        PVMFSharedMediaMsgPtr sharedMediaMsgPtr;
        bool packetComposed = pFirewallpacketExchanger->ComposeFirewallPacket(iFireWallPacketExchangeInfo.iFormat, iNumAttemptsDone, pPortInterface, sharedMediaMsgPtr);
        if (packetComposed)
        {
            ipObserver->MessageReadyToSend(pPortInterface, sharedMediaMsgPtr);
        }
        else
        {
            PVMF_JB_LOG_FW((0, "PVFirewallPacketExchangeImpl::SendFirewallPackets - packet composition failed"));
            OSCL_LEAVE(PVMFErrNoResources);
        }
    }
    ++iNumAttemptsDone;

    if (iNumAttemptsDone < iFireWallPacketExchangeInfo.iNumAttempts)
    {
        PVMFJBEventNotificationRequestInfo requestInfo(CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING, this, NULL);
        irEventNotifier.RequestCallBack(requestInfo, iFireWallPacketExchangeInfo.iServerRoundTripDelayInMS, iCallBackId);
        iCallBackPending = true;
    }
    else
    {
        ipObserver->MediaReceivingChannelPrepared(true);
    }
}
