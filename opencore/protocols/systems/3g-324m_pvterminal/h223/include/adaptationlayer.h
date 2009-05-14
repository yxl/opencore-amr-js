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
#if !defined(ADAPTATION_LAYER_H)
#define ADAPTATION_LAYER_H
#include "oscl_types.h"
#include "oscl_error_codes.h"
#include "pvt_params.h"
#include "h245def.h"
#include "crccheck_cpp.h"
#include "h324utils.h"

#ifndef PVMF_MEDIA_FRAG_GROUP_H_INCLUDED
#include "pvmf_media_frag_group.h"
#endif

#ifndef PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED
#include "pvmf_pool_buffer_allocator.h"
#endif

#define DEF_AL2_SN_POS 1
#define DEF_AL3_SN_POS 2

class IncomingALPduInfo
{
    public:
        IncomingALPduInfo() : sdu_size(0), crc_error(0), seq_num_error(0) {}
        int16 sdu_size;
        bool crc_error;
        int32 seq_num_error;
};

class AdaptationLayer
{
    public:
        AdaptationLayer(TPVDirection dir, uint32 sdu_size, uint32 max_num_sdus): iLogger(NULL)
        {
            OSCL_UNUSED_ARG(dir);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "AdaptationLayer::AdaptationLayer Dir=%d, Sdu Size=%d, Max num sdus=%d", dir, sdu_size, max_num_sdus));
            iSduSize = sdu_size;
            iMaxNumSdus = max_num_sdus;
            iPktNum = 0;
        }
        virtual ~AdaptationLayer() {}
        virtual unsigned GetHdrSz() = 0;
        virtual unsigned GetTrlrSz() = 0;
        virtual PVMFStatus StartPacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt) = 0;
        virtual PVMFStatus CompletePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt) = 0;
        virtual void ParsePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt, IncomingALPduInfo& info) = 0;
        unsigned GetSduSize()
        {
            return iSduSize;
        }
        unsigned GetPduSize()
        {
            return iSduSize + GetHdrSz() + GetTrlrSz();
        }
        virtual void Construct() = 0;
    protected:
        uint32 iSduSize;
        uint32 iMaxNumSdus;
        int32 iPktNum;
        PVLogger* iLogger;
};

class AdaptationLayer1 : public AdaptationLayer
{
    public:
        AdaptationLayer1(TPVDirection dir, uint32 sdu_size, uint32 max_num_sdus) : AdaptationLayer(dir, sdu_size, max_num_sdus)
        {
        }
        void Construct();

        unsigned GetHdrSz()
        {
            return 0;
        }
        unsigned GetTrlrSz()
        {
            return 0;
        }
        PVMFStatus StartPacket(OsclSharedPtr<PVMFMediaDataImpl>& /*pkt*/)
        {
            return PVMFSuccess;
        }

        PVMFStatus CompletePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt);
        void ParsePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt, IncomingALPduInfo& info);
};

class AdaptationLayer2 : public AdaptationLayer
{
    public:
        AdaptationLayer2(TPVDirection dir, uint32 sdu_size, uint32 max_num_sdus, bool seqno = false)
                : AdaptationLayer(dir, sdu_size, max_num_sdus),
                iSNPos(0),
                iHdrSz(0),
                iTrlrSz(1)
        {
            SetSeqnum(seqno);
            iSeqNum = 0;
        }
        void Construct();

        ~AdaptationLayer2()
        {
        }
        inline unsigned GetHdrSz()
        {
            return iHdrSz;
        }
        inline unsigned GetTrlrSz()
        {
            return iTrlrSz;
        }
        PVMFStatus StartPacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt);
        PVMFStatus CompletePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt);

        void ParsePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt, IncomingALPduInfo& info);

        void SetSeqnum(bool on_off);
    private:
        unsigned iSNPos;
        unsigned iHdrSz;
        unsigned iTrlrSz;
        unsigned iSeqNum;
        PVMFBufferPoolAllocator iMemFragmentAlloc;
        CRC crc;

};

class AdaptationLayer3 : public AdaptationLayer
{
    public:
        AdaptationLayer3(TPVDirection dir, uint32 sdu_size, uint32 max_num_sdus, uint16 snpos)
                : AdaptationLayer(dir, sdu_size, max_num_sdus),
                iSNPos(snpos),
                iHdrSz(snpos),
                iTrlrSz(2)
        {
            iSeqNum = 0;
        }

        ~AdaptationLayer3()
        {
        }

        void Construct();

        inline unsigned GetHdrSz()
        {
            return iHdrSz;
        }
        inline unsigned GetTrlrSz()
        {
            return iTrlrSz;
        }
        void SetSeqnumSz(uint16 sz);
        PVMFStatus StartPacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt);
        PVMFStatus CompletePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt);

        void ParsePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt, IncomingALPduInfo& info);
    private:
        void DRTXSend(int , uint16)
        {
        }

        void AlDrtxInd()
        {
        }

        unsigned iSNPos;
        unsigned iHdrSz;
        unsigned iTrlrSz;
        unsigned iSeqNum;
        PVMFBufferPoolAllocator iMemFragmentAlloc;

        CRC crc;
};



class AdaptationLayerAlloc
{
    public:
        static int GetAlIndex(ErrorProtectionLevel_t epl)
        {
            switch (epl)
            {
                case E_EP_LOW:
                    return 1;
                case E_EP_MEDIUM:
                    return 2;
                case E_EP_HIGH:
                    return 3;
            }
            return 0;
        }

        static OsclSharedPtr<AdaptationLayer> GetAdaptationLayer(TPVDirection dir,
                PS_H223LogicalChannelParameters lcnParams,
                uint32 sdu_size,
                uint32 max_num_sdus)
        {
            uint16 al_index = 0;
            uint16 hint = 0;
            switch (lcnParams->adaptationLayerType.index)
            {
                case 1:
                    al_index = 1;
                    break;
                case 2:
                    al_index = 1;
                    break;
                case 3:
                    al_index = 2;
                    hint = 0;
                    break;
                case 4:
                    al_index = 2;
                    hint = 1;
                    break;
                case 5:
                    al_index = 3;
                    hint = lcnParams->adaptationLayerType.al3->controlFieldOctets;
                    break;
            }
            return GetAdaptationLayer(dir, al_index, sdu_size, max_num_sdus, hint);
        }

        static OsclSharedPtr<AdaptationLayer> GetAdaptationLayer(TPVDirection dir,
                uint16 index,
                uint32 sdu_size,
                uint32 max_num_sdus,
                uint16 hint)
        {
            AdaptationLayer* al = NULL;
            void* ptr = NULL;
            switch (index)
            {
                case 1:
                    ptr = OSCL_DEFAULT_MALLOC(sizeof(AdaptationLayer1));
                    al = new(ptr)AdaptationLayer1(dir, sdu_size, max_num_sdus);
                    break;
                case 2:
                    ptr = OSCL_DEFAULT_MALLOC(sizeof(AdaptationLayer2));
                    al = new(ptr)AdaptationLayer2(dir, sdu_size, max_num_sdus, (hint ? true : false));
                    break;
                case 3:
                    ptr = OSCL_DEFAULT_MALLOC(sizeof(AdaptationLayer3));
                    al = new(ptr)AdaptationLayer3(dir, sdu_size, max_num_sdus, hint);
                    break;
                default:
                    OSCL_LEAVE(OsclFailure);
            }
            al->Construct();
            OsclRefCounterSA<Oscl_TAlloc<AdaptationLayer, BasicAlloc> > *alRefCounter =
                new OsclRefCounterSA<Oscl_TAlloc<AdaptationLayer, BasicAlloc> >(ptr);

            OsclSharedPtr<AdaptationLayer> alPtr(al, alRefCounter);

            return alPtr;
        }

        static uint16 GetHdrSz(int index)
        {
            switch (index)
            {
                case 0:
                case 1:
                case 2:
                case 3:
                    return 0;
                case 4:
                    return 1;
                case 5:
                    return 2;
            }
            return 0;
        }
        static int GetTrlrSz(int index)
        {
            switch (index)
            {
                case 0:
                case 1:
                case 2:
                    return 0;
                case 3:
                case 4:
                    return 1;
                case 5:
                    return 2;
            }
            return 0;
        }
};

#endif
