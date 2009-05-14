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
#ifndef H264_MEDIAINFO_H
#define H264_MEDIAINFO_H
#include "media_info.h"

class h264_mediaInfo : public mediaInfo
{
    public:
        h264_mediaInfo()
        {
            mediaInfo();
        };

        h264_mediaInfo(const h264_mediaInfo &pSource) : mediaInfo(pSource)
        {
        }
        ~h264_mediaInfo() {};
};

#endif
