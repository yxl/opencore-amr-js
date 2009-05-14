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
#ifndef AAC_TIMESTAMP_H_INCLUDED
#define AAC_TIMESTAMP_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#define DEFAULT_SAMPLING_FREQ 44100
#define DEFAULT_SAMPLES_PER_FRAME 1024

class AacTimeStampCalc
{
    public:

        AacTimeStampCalc()
        {
            iSamplingFreq = DEFAULT_SAMPLING_FREQ;
            iCurrentTs = 0;
            iCurrentSamples = 0;
            iSamplesPerFrame = DEFAULT_SAMPLES_PER_FRAME;
        };

        void SetParameters(uint32 aFreq, uint32 aSamples);

        void SetFromInputTimestamp(uint32 aValue);

        void UpdateTimestamp(uint32 aValue);

        uint32 GetConvertedTs();

        uint32 GetCurrentTimestamp();

        uint32 GetFrameDuration();

    private:
        uint32 iSamplingFreq;
        uint32 iCurrentTs;
        uint32 iCurrentSamples;
        uint32 iSamplesPerFrame;
};

#endif	//#ifndef AAC_TIMESTAMP_H_INCLUDED
