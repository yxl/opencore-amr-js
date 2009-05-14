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

#ifndef M4V_MEDIAINFO_H
#define M4V_MEDIAINFO_H
#include "media_info.h"


class m4v_mediaInfo : public mediaInfo
{
    private:

        double frameRate;
        int iFrameInterval;

    public:
        m4v_mediaInfo()
        {
            mediaInfo();
            frameRate = 0.0;
            iFrameInterval = -1;

        };

        m4v_mediaInfo(const m4v_mediaInfo &pSource) : mediaInfo(pSource)
        {

            setFrameRate(pSource.frameRate);
            setIFrameInterval(pSource.iFrameInterval);
        }
        const m4v_mediaInfo & operator=(const m4v_mediaInfo &pSource)
        {
            if (this != &pSource)
            {
                ((mediaInfo&) *this) = pSource;

                setFrameRate(pSource.frameRate);
                setIFrameInterval(pSource.iFrameInterval);
            }
            return *this;
        }

        ~m4v_mediaInfo() {};

        inline void setFrameRate(double fRate)
        {
            frameRate = fRate;
        };
        inline void setIFrameInterval(int IFI)
        {
            iFrameInterval = IFI;
        };

        inline double getFrameRate()
        {
            return frameRate;
        };
        inline int getIFrameInterval()
        {
            return iFrameInterval;
        };


};

#endif
