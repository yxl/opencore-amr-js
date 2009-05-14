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
#ifndef TSC_SRP_BUFFER_H
#define TSC_SRP_BUFFER_H

#include "oscl_stdstring.h"
#include "oscl_types.h"
#include "oscl_timer.h"
#include "oscl_mem.h"

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#ifndef PVMF_PORT_INTERFACE_H_INCLUDED
#include "pvmf_port_interface.h"
#endif

#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

#ifndef PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED
#include "pvmf_pool_buffer_allocator.h"
#endif

#include "layer.h"
#include "pvt_params.h"

#include "media_packet.h"

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif


#define TSCSRPBUFFER_INPUT_PORT_TAG 0
#define TSCSRPBUFFER_OUTPUT_PORT_TAG 1

class TscSrpBuffer;

class TscSrpBufferLLPortOut : public PvmfPortBaseImpl
{
    public:
        TscSrpBufferLLPortOut() : PvmfPortBaseImpl(TSCSRPBUFFER_OUTPUT_PORT_TAG, NULL) {};

        ~TscSrpBufferLLPortOut() {};
};

class TscSrpBufferLLPortIn : public PvmfPortBaseImpl
{
    public:
        static TscSrpBufferLLPortIn* NewL(TscSrpBuffer* aTscSrpBuffer);

        virtual ~TscSrpBufferLLPortIn();

        virtual PVMFStatus Receive(PVMFSharedMediaMsgPtr aMsg);

    private:
        TscSrpBufferLLPortIn(TscSrpBuffer* aTscSrpBuffer) : PvmfPortBaseImpl(TSCSRPBUFFER_INPUT_PORT_TAG, NULL),
                iCurSize(0),
                iPkt(NULL),
                iFrag(NULL),
                iMediaFragAlloc(NULL),
                iTscSrpBuffer(aTscSrpBuffer)
        {};
        static void ConstructSelf(TscSrpBufferLLPortIn* self);

        void ConstructL();

        uint32 iCurSize;
        MediaPacket* iPkt;
        MediaFragment* iFrag;
        MediaPacketAllocator iMediaPktAlloc;
        PoolFragmentAllocator* iMediaFragAlloc;
        TscSrpBuffer* iTscSrpBuffer;
};

class TscSrpBuffer : /*public SimpleStackElement, */public OsclTimerObserver
{
    public:
        typedef enum
        {
            TscSrpBufferStopped,
            TscSrpBufferStarted
        }TscSrpBufferState;

        static TscSrpBuffer* NewL();

        virtual ~TscSrpBuffer();

        Layer* GetUpperLayer()
        {
            return &iH245Interface;
        }

        PVMFPortInterface* GetLLPort(const int32 aPortTag)
        {
            if (aPortTag == TSCSRPBUFFER_INPUT_PORT_TAG)
            {
                return iLLPortIn;
            }
            if (aPortTag == TSCSRPBUFFER_OUTPUT_PORT_TAG)
            {
                return iLLPortOut;
            }

            return NULL;
        }

        void Start();
        void Stop();

        void Reset();
        void EnableBuffering(bool enable);

        // Receive packet from H245
        void ProcessOutgoingH245Packet(MediaPacket* pPkt);
        // Receive packet from SRP
        void ProcessIncomingSrpPacket(MediaPacket* pPkt);

        void TimeoutOccurred(int32 timerID, int32 timeoutInfo);

    private:

        static void ConstructSelf(TscSrpBuffer* self);
        TscSrpBuffer() : iNumMsgs(0),
                iTxMediaMsgPoolAlloc(NULL),
                iTxMediaDataImplMemAlloc(NULL),
                iTxPacketAlloc(NULL),
                iTimer(NULL),
                iH245Interface(NULL),
                iLLPortOut(NULL),
                iLLPortIn(NULL),
                iLogger(NULL),
                iState(TscSrpBufferStarted),
                iEnableBuffering(true)
        {};

        void ConstructL();

        uint32 iNumMsgs;
        OsclMemAllocator iMemAllocator;
        OsclMemPoolFixedChunkAllocator* iTxMediaMsgPoolAlloc;
        OsclMemPoolFixedChunkAllocator* iTxMediaDataImplMemAlloc;
        PVMFSimpleMediaBufferCombinedAlloc* iTxPacketAlloc;
        OsclTimer<OsclMemAllocator>* iTimer;

        // Interface with H245
        class UpperLayer : public Layer, public PacketInput, public PacketOutput
        {
            public:
                UpperLayer(TscSrpBuffer* parent)
                {
                    iParent = parent;
                }
                PacketInput* GetPacketInput()
                {
                    return this;
                }
                PacketOutput* GetPacketOutput()
                {
                    return this;
                }

                // Implement PAcketInput: process incoming packet
                void PacketIn(Packet* pack)
                {
                    if (iParent)
                    {
                        iParent->ProcessOutgoingH245Packet((MediaPacket*) pack);
                    }
                }
                void Dispatch(Packet* pack)
                {
                    if (pPktOutput) pPktOutput->PacketIn(pack);
                }

            private:
                TscSrpBuffer* iParent;
        };

        // Interface with SRP
        class LowerLayer : public Layer, public PacketInput, public PacketOutput
        {
            public:
                LowerLayer(TscSrpBuffer* parent)
                {
                    iParent = parent;
                }
                PacketInput* GetPacketInput()
                {
                    return this;
                }
                PacketOutput* GetPacketOutput()
                {
                    return this;
                }

                // Implement PAcketInput: process incoming packet
                void PacketIn(Packet* pack)
                {
                    if (iParent)
                    {
                        iParent->ProcessIncomingSrpPacket((MediaPacket*) pack);
                    }
                }
                void Dispatch(Packet* pack)
                {
                    if (pPktOutput) pPktOutput->PacketIn(pack);
                }
            private:
                TscSrpBuffer* iParent;
        };


        // Data sendto interfaces.
        // When sending to a lower layer, you are calling that object's upper layer rx,
        // when sending to an upper layer, you are calling that object's lower layer rx, got it?
        UpperLayer iH245Interface;
        PVMFPortInterface* iLLPortOut;
        PVMFPortInterface* iLLPortIn;
        PVLogger* iLogger;
        PVMFSharedMediaDataPtr iTxData;
        TscSrpBufferState iState;
        bool iEnableBuffering;
        friend class TscSrpBufferLLPortIn;
};
#endif
