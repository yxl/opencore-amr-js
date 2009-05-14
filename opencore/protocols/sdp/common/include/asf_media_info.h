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
#ifndef ASF_MEDIAINFO_H
#define ASF_MEDIAINFO_H

#include "sdp_memory.h"
#include "media_info.h"

// NOTE.... this information really needs to exist for only one media object...
// i.e., there is no reason to have arrays of these fields, since we have one of these
// mediaInfo elements for each media object... the trick is how to get these from the SDP
// file into each of the media elements...

class asf_mediaInfo : public mediaInfo
{
    private:

        //int profileLevelID;
        OSCL_HeapString<SDPParserAlloc> lang;
        unsigned char *asfHeaderPtr;
        int asfHeaderSize;

        //for both audio and video
        unsigned char *decoderSpecificInfo;
        int decoderSpecificInfoSize;
        //unsigned char *VOLHeaderPtr;
        //int VOLHeaderSize;
        int streamID;
        int streamBitrate;
        //int numSampleEntries;

    public:
        asf_mediaInfo()
        {
            lang = NULL;
            asfHeaderSize = 0;
            asfHeaderPtr = NULL;
            decoderSpecificInfoSize = 0;
            decoderSpecificInfo = NULL;
            //VOLHeaderSize = 0;
            //VOLHeaderPtr = NULL;
            streamBitrate = 0;
            //profileLevelID = -1;
        };
        asf_mediaInfo(const asf_mediaInfo & pSource) : mediaInfo(pSource)
        {
            setLang(pSource.lang);
            //setProfileLevelID(pSource.profileLevelID);
            asfHeaderSize = pSource.asfHeaderSize;
            decoderSpecificInfoSize = pSource.decoderSpecificInfoSize;
            //VOLHeaderSize = pSource.VOLHeaderSize;

        };

        ~asf_mediaInfo()
        {
            if (asfHeaderPtr != NULL)
            {
                oscl_free(asfHeaderPtr);
                asfHeaderPtr = NULL;
            }
            if (decoderSpecificInfo != NULL)
            {
//            oscl_free(decoderSpecificInfo); //added
                decoderSpecificInfo = NULL;
            }
            /*
            if(VOLHeaderPtr!=NULL) {
                oscl_free(VOLHeaderPtr);
                VOLHeaderPtr = NULL;
            }
            */
        };
        inline void setLang(char* language)
        {
            lang = language;
        };
        inline void setLang(const OSCL_HeapString<SDPParserAlloc>& language)
        {
            lang = language;
        };
        /*
        inline void setProfileLevelID(int pID)
        {
        	profileLevelID = pID;
        };
        */
        inline void setStreamID(int ID)
        {
            streamID = ID;
        };
        inline void setStreamBitrate(int bitrate)
        {
            streamBitrate = bitrate;
        };
        inline void setAsfHeader(unsigned char* ASCPtr, int ASCLen)
        {
            asfHeaderPtr = ASCPtr;
            asfHeaderSize = ASCLen;
        };

        inline void setDecoderSpecificInfo(unsigned char* ASCPtr, int ASCLen)
        {
            decoderSpecificInfo = ASCPtr;
            decoderSpecificInfoSize = ASCLen;
        };

        /*
        inline void setVOLHeader(unsigned char* ASCPtr, int ASCLen)
        {
            VOLHeaderPtr = ASCPtr;
            VOLHeaderSize = ASCLen;
        };
        */
        inline int GetTotalStringLength()
        {
            int size = 0;
            size += mediaInfo::GetTotalStringLength();
            if (lang.get_size() > 0)
            {
                size += oscl_strlen("a=lang:") + lang.get_size() + SDP_CR_LF;
            }
            return size;
        }
        inline const char *getLang()
        {
            return lang.get_cstr();
        };
//	inline int getProfileLevelID(){return profileLevelID;};

        inline const unsigned char *getAsfHeader(int*size)
        {
            return 0;
        };

        inline int getVOLLength()
        {
            //	return VOLHeaderSize;
            return 	decoderSpecificInfoSize;
        };

        inline unsigned char *getVOLHeader()
        {
            return decoderSpecificInfo;
            //return VOLHeaderPtr;
        };

        inline const unsigned char *getAudioSpecificConfig(int*size)
        {
            *size = decoderSpecificInfoSize;
            return decoderSpecificInfo;
        };

        inline int getStreamID()
        {
            return streamID;
        };
        inline int getStreamBitrate()
        {
            return streamBitrate;
        };
};
#endif
