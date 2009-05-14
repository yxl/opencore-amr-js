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

#ifndef STILL_IMAGE_MEDIAINFO_H
#define STILL_IMAGE_MEDIAINFO_H

#include "sdp_memory.h"
#include "media_info.h"

class still_image_mediaInfo : public mediaInfo
{
    private:
        int bitRate;
        OSCL_HeapString<SDPParserAlloc> payload;
    public:
        still_image_mediaInfo()
        {
            mediaInfo();
            setMIMEType("X-MP4V-IMAGE");
            bitRate = -1;
            payload = NULL;
            totalStringLength = oscl_strlen("m=application 0 RTSP/GET_PARAMETER IMAGE");
        };

        still_image_mediaInfo(const still_image_mediaInfo &pSource) : mediaInfo(pSource)
        {
            setBitRate(pSource.bitRate);
            setPayload(pSource.payload);
        }

        ~still_image_mediaInfo() {};

        inline void setBitRate(int bRate)
        {
            bitRate = bRate;
        };
        inline void setPayload(char* pload)
        {
            payload = pload;
        };
        inline void setPayload(OSCL_HeapString<SDPParserAlloc> pload)
        {
            payload = pload;
        };

        inline int getBitRate()
        {
            return bitRate;
        };
        inline char* getPayload()
        {
            return (char *)payload.get_cstr();
        };
};

#endif
