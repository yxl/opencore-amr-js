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

#ifndef H263_PAYLOAD_INFO_H
#define H263_PAYLOAD_INFO_H

#include "payload_info.h"

#define PVMF_SDP_DEFAULT_H263_SAMPLE_RATE 90000

class H263PayloadSpecificInfoType : public PayloadSpecificInfoTypeBase
{
    public:
        H263PayloadSpecificInfoType(int payload)
        {
            payloadNumber = payload;

            h263_codecProfile = -1;
            h263_codecLevel = -1;
            frameWidth = -1;
            frameHeight = -1;
        };

        inline void setCodecProfile(int cProfile)
        {
            h263_codecProfile = cProfile;
        };

        inline void setCodecLevel(int cLevel)
        {
            h263_codecLevel = cLevel;
        };

        inline void setFrameWidth(int fWidth)
        {
            frameWidth = fWidth;
        };
        inline void setFrameHeight(int fHeight)
        {
            frameHeight = fHeight;
        };

        int getCodecProfile()
        {
            return h263_codecProfile;
        };

        int getCodecLevel()
        {
            return h263_codecLevel;
        };

        inline int getFrameWidth()
        {
            return frameWidth;
        }

        inline int getFrameHeight()
        {
            return frameHeight;
        }

    private:
        int h263_codecProfile;
        int h263_codecLevel;
        int frameWidth;
        int frameHeight;
};


#endif
