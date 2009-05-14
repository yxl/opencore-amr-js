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
#include "still_image_media_info_parser.h"
#include "oscl_string_utils.h"

SDP_ERROR_CODE
SDPStillImageMediaInfoParser::parseMediaInfo(const char *buff, const int index, SDPInfo *sdp, payloadVector payload_vec, bool isSipSdp, int alt_id, bool alt_def_id)
{

    OSCL_UNUSED_ARG(buff);
    OSCL_UNUSED_ARG(index);
    OSCL_UNUSED_ARG(payload_vec);
    OSCL_UNUSED_ARG(isSipSdp);
    OSCL_UNUSED_ARG(alt_id);
    OSCL_UNUSED_ARG(alt_def_id);

    void *memory = sdp->alloc(sizeof(still_image_mediaInfo), 0);
    if (NULL == memory)
    {
        PVMF_SDP_PARSER_LOGERROR((0, "SDPStillImageMediaInfoParser::parseMediaInfo - Unable to allocate memory"));
        return SDP_NO_MEMORY;
    }
    else
    {
        still_image_mediaInfo *still_imageI = OSCL_PLACEMENT_NEW(memory, still_image_mediaInfo());

        still_imageI->setMediaInfoID(sdp->getMediaObjectIndex());

        if (NULL == still_imageI)
            return SDP_FAILURE;
        return SDP_SUCCESS;
    }

}

