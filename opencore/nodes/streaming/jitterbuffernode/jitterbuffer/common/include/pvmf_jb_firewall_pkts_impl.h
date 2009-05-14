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
#define PVMF_JB_FIREWALL_PKTS_IMPL_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

#ifndef PVMF_MEDIA_MSG_H_INCLUDED
#include "pvmf_media_msg.h"
#endif

#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVMF_SM_TUNABLES_H_INCLUDED
#include "pvmf_sm_tunables.h"
#endif

#ifndef PVMF_JITTER_BUFFER_COMMON_TYPES_H_INCLUDED
#include "pvmf_jitter_buffer_common_types.h"
#endif

#ifndef PVMF_JB_EVENT_NOTIFIER_H
#include "pvmf_jb_event_notifier.h"
#endif

/* Firewall packet related */
#define PVMF_DEFAULT_FIREWALL_PKT_ATTEMPTS 3
#define PVMF_MAX_FIREWALL_PKT_SIZE       8
#define PVMF_MAX_RTP_FIREWALL_PKT_SIZE   12
#define PVMF_FIREWALL_PKT_MEMPOOL_SIZE   256
#define PVMF_FIREWALL_PKT_DEFAULT_SERVER_RESPONSE_TIMEOUT_IN_MS 1000
class PVMFJBEventNotifier;
///////////////////////////////////////////////////////////////////////////////
//PVMFFireWallPacketFormat
///////////////////////////////////////////////////////////////////////////////
enum PVMFFireWallPacketFormat
{
    PVMF_FW_PKT_FORMAT_RTP,
    PVMF_FW_PKT_FORMAT_PV
};

///////////////////////////////////////////////////////////////////////////////
//PVMFJitterBufferFireWallPacketInfo
///////////////////////////////////////////////////////////////////////////////

class PVFirewallPacketExchanger
{
    public:
        static PVFirewallPacketExchanger* New(const RTPSessionInfoForFirewallExchange& aRTPSessionInfo);
        ~PVFirewallPacketExchanger();
        bool ComposeFirewallPacket(PVMFJitterBufferFireWallPacketFormat aFormat, uint32 aPacketCnt, PVMFPortInterface*& aRTPJitterBufferPort, PVMFSharedMediaMsgPtr& aSharedMediaMsg);
        bool IsFirewallPacketResponseValid(PVMFSharedMediaMsgPtr& aMsg);//not implementated as of now
        bool DecodeFirewallPacketResponse(PVMFSharedMediaMsgPtr& aMsg);//not implementated as of now
    private:
        PVFirewallPacketExchanger(const RTPSessionInfoForFirewallExchange& aRTPSessionInfo)
        {
            iRTPSessionInfoForFirewallExchange.ipRTPDataJitterBufferPort = aRTPSessionInfo.ipRTPDataJitterBufferPort;
            iRTPSessionInfoForFirewallExchange.iSSRC = aRTPSessionInfo.iSSRC;
            ipMediaDataAlloc	=	NULL;
            ipMediaDataImplAlloc	=	NULL;
            ipMediaMsgAlloc	=	NULL;
        }
        void Construct();
        void CreateMemAllocators();
        void DestroyMemoryAllocators();
        bool Allocate(PVMFSharedMediaDataPtr& aFireWallPkt, OsclSharedPtr<PVMFMediaDataImpl>& aMediaDataImpl, const int aSize);

        //Mem allocators
        /* allocator for memory fragment */
        OsclMemPoolFixedChunkAllocator* ipMediaDataAlloc;
        /* allocator for media data impl */
        PVMFSimpleMediaBufferCombinedAlloc* ipMediaDataImplAlloc;
        /* Memory pool for simple media data */
        OsclMemPoolFixedChunkAllocator* ipMediaMsgAlloc;

        RTPSessionInfoForFirewallExchange	iRTPSessionInfoForFirewallExchange;
};

///////////////////////////////////////////////////////////////////////////////
//PVFirewallPacketExchanger
///////////////////////////////////////////////////////////////////////////////
class PVMFJitterBufferMiscObserver;

class PVFirewallPacketExchangeImpl: public PVMFJBEventNotifierObserver
{
    public:
        OSCL_IMPORT_REF static PVFirewallPacketExchangeImpl* New(PVMFJitterBufferFireWallPacketInfo& aFireWallPacketExchangeInfo, PVMFJBEventNotifier& aEventNotifier, PVMFJitterBufferMiscObserver* aObserver);
        OSCL_IMPORT_REF ~PVFirewallPacketExchangeImpl();
        OSCL_IMPORT_REF void SetRTPSessionInfoForFirewallExchange(const RTPSessionInfoForFirewallExchange& aRTPSessionInfo);
        OSCL_IMPORT_REF PVMFStatus InitiateFirewallPacketExchange();
        OSCL_IMPORT_REF PVMFStatus CancelFirewallPacketExchange();
        void ProcessCallback(CLOCK_NOTIFICATION_INTF_TYPE aClockNotificationInterfaceType, uint32 aCallBkId, const OsclAny* aContext, PVMFStatus aStatus);
    private:
        PVFirewallPacketExchangeImpl(PVMFJitterBufferFireWallPacketInfo& aFireWallPacketExchangeInfo, PVMFJBEventNotifier& aEventNotifier, PVMFJitterBufferMiscObserver* aObserver)
                : irEventNotifier(aEventNotifier)
                , ipObserver(aObserver)
        {
            iFireWallPacketExchangeInfo.iServerRoundTripDelayInMS = aFireWallPacketExchangeInfo.iServerRoundTripDelayInMS;
            iFireWallPacketExchangeInfo.iNumAttempts = aFireWallPacketExchangeInfo.iNumAttempts;
            iFireWallPacketExchangeInfo.iFormat = aFireWallPacketExchangeInfo.iFormat;
            iCallBackId = 0;
            iCallBackPending = false;
            iNumAttemptsDone = 0;
        }
        void Construct();
        void SendFirewallPackets();

        uint32 iCallBackId;
        bool iCallBackPending;
        uint32 iNumAttemptsDone;

        PVMFJBEventNotifier&	irEventNotifier;
        PVMFJitterBufferMiscObserver*	ipObserver;
        PVMFJitterBufferFireWallPacketInfo iFireWallPacketExchangeInfo;
        Oscl_Vector<PVFirewallPacketExchanger*, OsclMemAllocator> iFirewallPacketExchangers;
        PVLogger*	ipDataPathLoggerFireWall;
};

#endif//end of PVMF_JB_FIREWALL_PKTS_IMPL_H_INCLUDED
