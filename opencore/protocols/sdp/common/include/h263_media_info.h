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

#ifndef H263_MEDIAINFO_H
#define H263_MEDIAINFO_H
#include "media_info.h"

class h263_mediaInfo : public mediaInfo
{
    private:

    public:
        h263_mediaInfo()
        {
            mediaInfo();

        };

        h263_mediaInfo(const h263_mediaInfo &pSource) : mediaInfo(pSource)
        {

        }

        ~h263_mediaInfo() {};


};

#endif
