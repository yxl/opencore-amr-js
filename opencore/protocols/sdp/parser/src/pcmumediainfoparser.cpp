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
#include "pcmu_media_info_parser.h"
#include "oscl_string_utils.h"
#include "oscl_string_containers.h"

SDP_ERROR_CODE
SDPPCMUMediaInfoParser::parseMediaInfo(const char *buff, const int index, SDPInfo *sdp, payloadVector payload_vec, bool isSipSdp, int alt_id, bool alt_def_id)
{
    OSCL_UNUSED_ARG(alt_id);
    OSCL_UNUSED_ARG(alt_def_id);

    const char *current_start = buff; //Pointer to the beginning of the media text
    const char *end = buff + index;   //Pointer to the end of the media text

    (void) current_start;
    (void) end;

    //Allocate media info class here
    void *memory = sdp->alloc(sizeof(pcmu_mediaInfo), false);
    if (NULL == memory)
    {
        return SDP_NO_MEMORY;
    }
    else
    {
        pcmu_mediaInfo *pcmuA = OSCL_PLACEMENT_NEW(memory, pcmu_mediaInfo());

        pcmuA->setMediaInfoID(sdp->getMediaObjectIndex());

        // Allocate memory to the payload specific objects
        for (uint32 ii = 0; ii < payload_vec.size(); ii++)
        {
            void* mem = pcmuA->alloc(sizeof(PcmuPayloadSpecificInfoType));
            if (mem == NULL)
            {
                return SDP_NO_MEMORY;
            }
            else
            {
                PcmuPayloadSpecificInfoType* payload = OSCL_PLACEMENT_NEW(mem, PcmuPayloadSpecificInfoType(payload_vec[ii]));
                (void) payload;
            }
        }


        SDP_ERROR_CODE status = baseMediaInfoParser(buff, pcmuA, index , false, false, isSipSdp);
        if (status != SDP_SUCCESS)
        {
            return status;
        }

        // payloadNumber is present in the mediaInfo. get the payload
        // Specific pointer corresponding to this payload
        PcmuPayloadSpecificInfoType* payloadPtr = NULL;

        for (uint32 jj = 0; jj < payload_vec.size(); jj++)
        {
            payloadPtr = (PcmuPayloadSpecificInfoType*)pcmuA->getPayloadSpecificInfoTypePtr(payload_vec[jj]);
            if (payloadPtr == NULL)
                return SDP_PAYLOAD_MISMATCH;

            payloadPtr->setSampleRate(8000);
        }
    }
    return SDP_SUCCESS;
}

//EndOfFile

