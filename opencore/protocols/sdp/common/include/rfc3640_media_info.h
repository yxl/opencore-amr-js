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

#ifndef RFC3640_MEDIAINFO_H_INCLUDED
#define RFC3640_MEDIAINFO_H_INCLUDED

#ifndef MEDIAINFO_H
#include "media_info.h"
#endif

class rfc3640_mediaInfo : public mediaInfo
{
    private:

        uint32 streamType;
        OSCL_HeapString<SDPParserAlloc> mode;

    public:
        //For now, just default to AAC high bit-rate.
        rfc3640_mediaInfo()
        {
            mediaInfo();
            mode = NULL;

        };

        rfc3640_mediaInfo(const rfc3640_mediaInfo &pSource) : mediaInfo(pSource)
        {

        }
        const rfc3640_mediaInfo & operator=(const rfc3640_mediaInfo &pSource)
        {
            if (this != &pSource)
            {
                ((mediaInfo&) *this) = pSource;
                setStreamType(pSource.streamType);
                setModeType(pSource.mode);
            }
            return *this;
        }

        ~rfc3640_mediaInfo() {};

        inline void setStreamType(uint32 stype)
        {
            streamType = stype;
        };

        inline uint32 getStreamType()
        {
            return streamType;
        };
        inline void setMode(char* md)
        {
            mode = md;
        };
        inline void setModeType(const OSCL_HeapString<SDPParserAlloc>& md)
        {
            mode = md;
        };
        inline void setModeType(const OsclMemoryFragment memFrag)
        {
            mode.set((const char*)(memFrag.ptr), memFrag.len);
        };
        inline char* getMode(void)
        {
            return mode.get_str();
        };



};

#endif
