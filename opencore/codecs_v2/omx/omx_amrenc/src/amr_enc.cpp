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

/*----------------------------------------------------------------------------
; INCLUDES
----------------------------------------------------------------------------*/

#include "amr_enc.h"


OmxAmrEncoder::OmxAmrEncoder()
{
    // Codec and encoder setting structure
    ipGsmEncoder = NULL;
    ipEncProps   = NULL;
    iNextStartTime = 0;

    iOutputFormat			  = PVMF_MIME_AMR_IETF;
    ipSizeArrayForOutputFrames = NULL;
    iNextStartTime			  = 0;

    iMaxNumOutputFramesPerBuffer = MAX_NUM_OUTPUT_FRAMES_PER_BUFFER;
    iOneInputFrameLength		 = 320;
    iMaxInputSize				 = 0;

    iAmrInitFlag = 0;
}


/* Decoder Initialization function */
OMX_BOOL OmxAmrEncoder::AmrEncInit(OMX_AUDIO_PARAM_PCMMODETYPE aPcmMode,
                                   OMX_AUDIO_PARAM_AMRTYPE aAmrParam,
                                   OMX_U32* aInputFrameLength,
                                   OMX_U32* aMaxNumberOutputFrames)
{
    OMX_U32	MaxOutputBufferSize;


    iAmrInitFlag = 0;

    ipGsmEncoder = OSCL_NEW(CPvGsmAmrEncoder, ());
    if (!ipGsmEncoder)
    {
        return OMX_FALSE;
    }

    // for encoder properties
    ipEncProps = OSCL_NEW(TEncodeProperties, ());
    if (!ipEncProps)
    {
        return OMX_FALSE;
    }

    //Extracting the output format information
    if (OMX_AUDIO_AMRFrameFormatFSF == aAmrParam.eAMRFrameFormat)
    {
        iOutputFormat = PVMF_MIME_AMR_IETF;
    }
    else if (OMX_AUDIO_AMRFrameFormatIF2 == aAmrParam.eAMRFrameFormat)
    {
        iOutputFormat = PVMF_MIME_AMR_IF2;
    }
    else
    {
        //unsupported format
        return OMX_FALSE;
    }


    if (OMX_AUDIO_AMRBandModeNB0 == aAmrParam.eAMRBandMode)
    {
        ipEncProps->iMode = ipGsmEncoder->GSM_AMR_4_75;
    }
    else if (OMX_AUDIO_AMRBandModeNB1 == aAmrParam.eAMRBandMode)
    {
        ipEncProps->iMode = ipGsmEncoder->GSM_AMR_5_15;
    }
    else if (OMX_AUDIO_AMRBandModeNB2 == aAmrParam.eAMRBandMode)
    {
        ipEncProps->iMode = ipGsmEncoder->GSM_AMR_5_90;
    }
    else if (OMX_AUDIO_AMRBandModeNB3 == aAmrParam.eAMRBandMode)
    {
        ipEncProps->iMode = ipGsmEncoder->GSM_AMR_6_70;
    }
    else if (OMX_AUDIO_AMRBandModeNB4 == aAmrParam.eAMRBandMode)
    {
        ipEncProps->iMode = ipGsmEncoder->GSM_AMR_7_40;
    }
    else if (OMX_AUDIO_AMRBandModeNB5 == aAmrParam.eAMRBandMode)
    {
        ipEncProps->iMode = ipGsmEncoder->GSM_AMR_7_95;
    }
    else if (OMX_AUDIO_AMRBandModeNB6 == aAmrParam.eAMRBandMode)
    {
        ipEncProps->iMode = ipGsmEncoder->GSM_AMR_10_2;
    }
    else if (OMX_AUDIO_AMRBandModeNB7 == aAmrParam.eAMRBandMode)
    {
        ipEncProps->iMode = ipGsmEncoder->GSM_AMR_12_2;
    }
    else
    {
        //unsupported mode
        return OMX_FALSE;
    }

    //Adding all the param verification here before allocating them into ipEncProps
    if ((16 != aPcmMode.nBitPerSample) ||
            (8000 != aPcmMode.nSamplingRate) ||
            (1 != aPcmMode.nChannels))
    {
        return OMX_FALSE;
    }


    ipEncProps->iInBitsPerSample  = aPcmMode.nBitPerSample;
    ipEncProps->iInSamplingRate   = aPcmMode.nSamplingRate;
    ipEncProps->iInClockRate      = 1000;
    ipEncProps->iInNumChannels    = (uint8)aPcmMode.nChannels;
    ipEncProps->iInInterleaveMode = TEncodeProperties::EINTERLEAVE_LR;

    ipEncProps->iBitStreamFormat = (iOutputFormat == PVMF_MIME_AMR_IF2);
    ipEncProps->iAudioObjectType    = 0; // only for AAC encoder
    ipEncProps->iOutSamplingRate    = aPcmMode.nSamplingRate;
    ipEncProps->iOutNumChannels     = (uint8)aPcmMode.nChannels;
    ipEncProps->iOutClockRate       = ipEncProps->iInClockRate;

    // initialize the amr encoder
    MaxOutputBufferSize = iMaxNumOutputFramesPerBuffer * MAX_AMR_FRAME_SIZE;
    if (ipGsmEncoder->InitializeEncoder(MaxOutputBufferSize, ipEncProps) < 0)
    {
        return OMX_FALSE;
    }

    ipSizeArrayForOutputFrames = (int32*) oscl_malloc(iMaxNumOutputFramesPerBuffer * sizeof(int32));
    oscl_memset(ipSizeArrayForOutputFrames, 0, iMaxNumOutputFramesPerBuffer * sizeof(int32));

    iOneInputFrameLength = AMR_FRAME_LENGTH_IN_TIMESTAMP * ipEncProps->iInSamplingRate * ipEncProps->iInBitsPerSample / 8000;
    iMaxInputSize = iMaxNumOutputFramesPerBuffer * iOneInputFrameLength;

    *aInputFrameLength = iOneInputFrameLength;

    *aMaxNumberOutputFrames = iMaxNumOutputFramesPerBuffer;

    return OMX_TRUE;
}


/* Decoder De-Initialization function */
void OmxAmrEncoder::AmrEncDeinit()
{
    if (ipGsmEncoder)
    {
        ipGsmEncoder->CleanupEncoder();
        OSCL_DELETE(ipGsmEncoder);
        ipGsmEncoder = NULL;
    }

    if (ipEncProps)
    {
        OSCL_DELETE(ipEncProps);
        ipEncProps = NULL;
    }

    if (ipSizeArrayForOutputFrames)
    {
        oscl_free(ipSizeArrayForOutputFrames);
        ipSizeArrayForOutputFrames = NULL;
    }
}


/* Decode function for all the input formats */
OMX_BOOL OmxAmrEncoder::AmrEncodeFrame(OMX_U8*    aOutputBuffer,
                                       OMX_U32*   aOutputLength,
                                       OMX_U8*    aInBuffer,
                                       OMX_U32    aInBufSize,
                                       OMX_TICKS  aInTimeStamp,
                                       OMX_TICKS* aOutTimeStamp)
{

    TInputAudioStream StreamInput;
    TOutputAudioStream StreamOutput;
    int32 InputFrameNum;

    StreamOutput.iBitStreamBuffer = (uint8*) aOutputBuffer;
    StreamOutput.iNumSampleFrames = 0;
    StreamOutput.iSampleFrameSize = ipSizeArrayForOutputFrames;

    //Calculate the number of input frames to be encoded
    InputFrameNum = aInBufSize / iOneInputFrameLength;

    StreamInput.iSampleBuffer = (uint8*) aInBuffer;
    StreamInput.iSampleLength = (int32) aInBufSize;
    StreamInput.iMode		  = ipEncProps->iMode;
    StreamInput.iStartTime	  = (iNextStartTime >= aInTimeStamp  ? iNextStartTime : aInTimeStamp);
    StreamInput.iStopTime	  = StreamInput.iStartTime + AMR_FRAME_LENGTH_IN_TIMESTAMP * InputFrameNum;
    iNextStartTime			  = StreamInput.iStopTime; // for the next encoding

    // Do encoding at one time for multiple frame input
    if (ipGsmEncoder->Encode(StreamInput, StreamOutput) < 0 || StreamOutput.iNumSampleFrames != InputFrameNum)
    {
        return OMX_FALSE;
    }

    // For IETF, make a conversion from WMF
    uint8* TmpBuffer = StreamOutput.iBitStreamBuffer;
    uint32 ii;

    for (ii = 0; ii < (uint32)StreamOutput.iNumSampleFrames; ii++)
    {
        // for IETF format, we need to make change
        if (!ipEncProps->iBitStreamFormat)
        {
            // non-IF2 => IETF format, not WMF format
            TmpBuffer[0] = (uint8)(((TmpBuffer[0] << 3) | 0x4) & 0x7C); // IETF frame header: P(1) + FT(4) + Q(1) + P(2) , Q=1 for good frame, P=padding bit, 0
            TmpBuffer += StreamOutput.iSampleFrameSize[ii];
        }

        // Set fragment length
        *aOutputLength += StreamOutput.iSampleFrameSize[ii];
    }

    //Set the output buffer timestamp equal to the input buffer start time.
    *aOutTimeStamp = StreamInput.iStartTime;

    return OMX_TRUE;
}



