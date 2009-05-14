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
#ifndef RTP_PAYLOAD_PARSER_BASE_H
#define RTP_PAYLOAD_PARSER_BASE_H

#include "rtp_payload_parser_include.h"
#include "rtppp_media_frag_group.h"

#include "pvmf_media_data.h"
#include "pvmf_media_frag_group.h"

#include "oscl_mem_mempool.h"
#include "oscl_vector.h"

enum RTP_PAYLOAD_PARSER_RET_CODE
{
    RTP_PAYLOAD_PARSER_FAILURE = 0,
    RTP_PAYLOAD_PARSER_SUCCESS = 1,
    RTP_PAYLOAD_PARSER_INPUT_NOT_EXHAUSTED = 2,
    RTP_PAYLOAD_PARSER_MEMORY_ALLOC_FAILURE = 3,
    RTP_PAYLOAD_PARSER_DATA_NOT_READY = 4,
    RTP_PAYLOAD_PARSER_INTERNAL_QUEUE_IS_EMPTY = 5
};

#define RTP_PAYLOAD_PARSER_MAX_NUM_MEDIA_DATA  64

typedef OsclMemAllocator PoolMemAlloc;

class RTPPayloadParser
{
    public:
        RTPPayloadParser();
        virtual ~RTPPayloadParser();

        /* single payload parse */
        OSCL_IMPORT_REF virtual RTP_PAYLOAD_PARSER_RET_CODE parseRTPPayload(PVMFSharedMediaDataPtr& rtpPacket,
                PVMFSharedMediaDataPtr& accessUnit) = 0;

        /* multiple payload parsing - need not be implemented in deriving class */
        OSCL_IMPORT_REF virtual RTP_PAYLOAD_PARSER_RET_CODE parseRTPPayload(PVMFSharedMediaDataPtr& rtpPacket,
                Oscl_Vector<PVMFSharedMediaDataPtr, OsclMemAllocator>*& accessUnits)
        {
            return RTP_PAYLOAD_PARSER_FAILURE;
        }

        virtual uint numBuffersRequired(PVMFSharedMediaDataPtr& rtpPacket)
        {
            return 1;
        }


        OSCL_IMPORT_REF virtual bool IsOutputBufferAvailable(uint& available, int count = 1);

        void notifyfreechunkavailable(OsclMemPoolFixedChunkAllocatorObserver& obs, int numChunks = 1)
        {
            if (iMediaDataGroupAlloc)
                iMediaDataGroupAlloc->notifyfreechunkgroupavailable(obs, numChunks, NULL);
        }

        virtual void setRepositionFlag()
        {
            ibRepositionFlag = true;
        }
        virtual uint32 getMinCurrTimestamp()
        {
            return 0;
        }

    protected:
        RTPPPMediaFragGroupCombinedAlloc<PoolMemAlloc>* iMediaDataGroupAlloc;
        OsclMemPoolFixedChunkAllocator* iMediaDataImplMemPool;
        bool ibRepositionFlag;
};


#endif // RTP_PAYLOAD_PARSER_BASE_H
