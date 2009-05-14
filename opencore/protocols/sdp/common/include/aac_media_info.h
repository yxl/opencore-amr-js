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

#ifndef AAC_MEDIAINFO_H
#define AAC_MEDIAINFO_H

#include "sdp_memory.h"
#include "media_info.h"


// NOTE.... this information really needs to exist for only one media object...
// i.e., there is no reason to have arrays of these fields, since we have one of these
// mediaInfo elements for each media object... the trick is how to get these from the SDP
// file into each of the media elements...



class aac_mediaInfo : public mediaInfo
{
    private:
        int profileLevelID;
        OSCL_HeapString<SDPParserAlloc> lang;
        int numSampleEntries;

    public:
        aac_mediaInfo()
        {
            lang = NULL;
            profileLevelID = -1;
        };
        aac_mediaInfo(const aac_mediaInfo & pSource) : mediaInfo(pSource)
        {
            setLang(pSource.lang);
            setProfileLevelID(pSource.profileLevelID);
        }

        ~aac_mediaInfo()
        {
        };
        inline void setLang(char* language)
        {
            lang = language;
        };
        inline void setLang(const OSCL_HeapString<SDPParserAlloc>& language)
        {
            lang = language;
        };
        inline void setLang(const OsclMemoryFragment memFrag)
        {
            lang.set((const char*)(memFrag.ptr), memFrag.len);
        };
        inline void setProfileLevelID(int pID)
        {
            profileLevelID = pID;
        };

        inline void setNumSampleEntries(int inNumSampleEntries)
        {
            numSampleEntries = inNumSampleEntries;
        };


        inline const char *getLang()
        {
            return lang.get_cstr();
        };
        inline int getProfileLevelID()
        {
            return profileLevelID;
        };
        inline int getNumSampleEntries()
        {
            return numSampleEntries;
        };

};
#endif
