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

#ifndef PAYLOAD_INFO_H
#define PAYLOAD_INFO_H

class PayloadSpecificInfoTypeBase
{
    public:
        PayloadSpecificInfoTypeBase()
        {
            payloadNumber = 0;
            sampleRate = 0;
            noOfChannels = 0;
            maxBufferSize = 0;
            profileLevelID = -1;
            configSize = 0;
        };

        virtual ~PayloadSpecificInfoTypeBase()
        {};

        inline void setPayloadNumber(int pNumber)
        {
            payloadNumber = pNumber;
        };

        inline void setSampleRate(int sRate)
        {
            sampleRate = sRate;
        };

        inline void setNoOfChannels(int channels)
        {
            noOfChannels = channels;
        };

        inline void setMaxBufferSize(int maxBuff)
        {
            maxBufferSize = maxBuff;
        };

        inline void setProfileLevelID(int pID)
        {
            profileLevelID = pID;
        };

        inline void setDecoderSpecificInfo(OsclSharedPtr<uint8> Info)
        {
            configHeader = Info;
        };

        OsclSharedPtr<uint8> getConfigHeader()
        {
            return configHeader;
        }

        inline void setDecoderSpecificInfoSize(uint32 size)
        {
            configSize = size;
        };

        uint32 getConfigSize()
        {
            return configSize;
        };
        uint32 getPayloadNumber()
        {
            return payloadNumber;
        };

        uint32 getSampleRate()
        {
            return sampleRate;
        };

        uint32 getNumberOfChannels()
        {
            return noOfChannels;
        };

        inline int getMaxBufferSize()
        {
            return maxBufferSize;
        };

        inline int getProfileLevelID()
        {
            return profileLevelID;
        };

        uint32	payloadNumber;
        uint32	sampleRate;
        uint32	noOfChannels;
        int		maxBufferSize;
        int		profileLevelID;
        OsclSharedPtr<uint8> configHeader;
        uint32	configSize;

};

#endif
