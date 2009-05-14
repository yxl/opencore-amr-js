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

#include "aac_timestamp.h"


//Initialize the parameters
void AacTimeStampCalc::SetParameters(uint32 aFreq, uint32 aSamples)
{
    if (0 != aFreq)
    {
        iSamplingFreq = aFreq;
    }

    iSamplesPerFrame = aSamples;
}


//Set the current timestamp equal to the input buffer timestamp
void AacTimeStampCalc::SetFromInputTimestamp(uint32 aValue)
{
    iCurrentTs = aValue;
    iCurrentSamples = 0;
}


void AacTimeStampCalc::UpdateTimestamp(uint32 aValue)
{
    iCurrentSamples += aValue;
}

//Convert current samples into the output timestamp
uint32 AacTimeStampCalc::GetConvertedTs()
{
    uint32 Value = iCurrentTs;

    //Formula used: TS in ms = (samples * 1000/sampling_freq)
    //Rounding added (add 0.5 to the result), extra check for divide by zero
    if (0 != iSamplingFreq)
    {
        Value = iCurrentTs + (iCurrentSamples * 1000 + (iSamplingFreq / 2)) / iSamplingFreq;
    }

    iCurrentTs = Value;
    iCurrentSamples = 0;

    return (Value);
}


/* Do not update iCurrentTs value, just calculate & return the current timestamp */
uint32 AacTimeStampCalc::GetCurrentTimestamp()
{
    uint32 Value = iCurrentTs;

    if (0 != iSamplingFreq)
    {
        Value = iCurrentTs + (iCurrentSamples * 1000 + (iSamplingFreq / 2)) / iSamplingFreq;
    }

    return (Value);
}

//Calculate the timestamp of single frame
uint32 AacTimeStampCalc::GetFrameDuration()
{
    uint32 Value = 0;

    if (0 != iSamplingFreq)
    {
        Value = (iSamplesPerFrame * 1000 + (iSamplingFreq / 2)) / iSamplingFreq;
    }

    return (Value);
}
