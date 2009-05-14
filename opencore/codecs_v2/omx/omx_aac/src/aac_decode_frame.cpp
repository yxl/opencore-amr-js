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

#include "aac_dec.h"

OmxAacDecoder::OmxAacDecoder()
{
    iAacInitFlag = 0;
    iInputUsedLength = 0;
}


OMX_BOOL OmxAacDecoder::AacDecInit(OMX_U32 aDesiredChannels)
{
    Int                         Status;

    iMemReq = PVMP4AudioDecoderGetMemRequirements();
    ipMem = oscl_malloc(iMemReq);

    if (0 == ipMem)
    {
        return OMX_FALSE;
    }

    oscl_memset(&iExt, 0, sizeof(tPVMP4AudioDecoderExternal));

    iExt.inputBufferCurrentLength = 0;
    iExt.remainderBits        = 0;      // Not needed anymore.
    iExt.inputBufferMaxLength = PVMP4AUDIODECODER_INBUFSIZE;
    iExt.outputFormat         = OUTPUTFORMAT_16PCM_INTERLEAVED;
    iExt.desiredChannels      = aDesiredChannels;
    iExt.aacPlusEnabled		  = TRUE;
    iAacInitFlag = 0;
    iInputUsedLength = 0;
    //This var is required to do init again inbetween
    iNumOfChannels			 = aDesiredChannels;


    Status = PVMP4AudioDecoderInitLibrary(&iExt, ipMem);
    return OMX_TRUE;
}

void OmxAacDecoder::AacDecDeinit()
{
    oscl_free(ipMem);
    ipMem = NULL;
}

void OmxAacDecoder::ResetDecoder()
{

    if (ipMem && (iAacInitFlag != 0))
    {
        PVMP4AudioDecoderResetBuffer(ipMem);
    }

}

Int OmxAacDecoder::AacDecodeFrames(OMX_S16* aOutputBuffer,
                                   OMX_U32* aOutputLength, OMX_U8** aInBuffer,
                                   OMX_U32* aInBufSize, OMX_S32* aIsFirstBuffer,
                                   OMX_AUDIO_PARAM_PCMMODETYPE* aAudioPcmParam,
                                   OMX_AUDIO_PARAM_AACPROFILETYPE* aAudioAacParam,
                                   OMX_U32* aSamplesPerFrame,
                                   OMX_BOOL* aResizeFlag)
{
    Int   Status;
    Int32 StreamType;


    *aResizeFlag = OMX_FALSE;

    if (0 == iAacInitFlag)
    {
        //Initialization is required again when the client inbetween rewinds the input bitstream
        //Added to pass khronous conformance tests
        if (*aIsFirstBuffer != 0)
        {
            /* When the input file is reset to the begining by the client,
             * this module should again be called.
             */
            oscl_memset(ipMem, 0, iMemReq);
            oscl_memset(&iExt, 0, sizeof(tPVMP4AudioDecoderExternal));

            iExt.inputBufferCurrentLength = 0;
            iExt.remainderBits        = 0;
            iExt.inputBufferMaxLength = PVMP4AUDIODECODER_INBUFSIZE;
            iExt.outputFormat         = OUTPUTFORMAT_16PCM_INTERLEAVED;
            iExt.desiredChannels      = iNumOfChannels;
            iExt.aacPlusEnabled		  = TRUE;
            iInputUsedLength = 0;

            PVMP4AudioDecoderInitLibrary(&iExt, ipMem);
        }
    }

    iExt.pInputBuffer         = *aInBuffer + iInputUsedLength;
    iExt.pOutputBuffer        = &aOutputBuffer[0];

#ifdef AAC_PLUS
    iExt.pOutputBuffer_plus   = &aOutputBuffer[2048];
#endif

    iExt.inputBufferCurrentLength = *aInBufSize;

    //Decode the config buffer
    if (0 == iAacInitFlag)
    {
        Status = PVMP4AudioDecoderConfig(&iExt, ipMem);
        if (MP4AUDEC_SUCCESS == Status)
        {
            iAacInitFlag = 1;
        }

        iConfigUpSamplingFactor = iExt.aacPlusUpsamplingFactor;

        if (2 == iExt.aacPlusUpsamplingFactor)
        {
            *aSamplesPerFrame = 2 * AACDEC_PCM_FRAME_SAMPLE_SIZE;
        }
        else
        {
            *aSamplesPerFrame = AACDEC_PCM_FRAME_SAMPLE_SIZE;
        }

        *aInBufSize -= iExt.inputBufferUsedLength;

        if (0 == *aInBufSize)
        {
            iInputUsedLength = 0;
        }
        else
        {
            iInputUsedLength = iExt.inputBufferUsedLength;
        }

        return Status;
    }

    iExt.inputBufferUsedLength = 0;
    Status = PVMP4AudioDecodeFrame(&iExt, ipMem);

    if (MP4AUDEC_SUCCESS == Status || SUCCESS == Status)
    {

        *aInBufSize -= iExt.inputBufferUsedLength;
        if (0 == *aInBufSize)
        {
            iInputUsedLength = 0;
        }
        else
        {
            iInputUsedLength += iExt.inputBufferUsedLength;
        }

        *aOutputLength = iExt.frameLength * iExt.desiredChannels;

#ifdef AAC_PLUS
        if (2 == iExt.aacPlusUpsamplingFactor)
        {
            if (1 == iExt.desiredChannels)
            {
                oscl_memcpy(&aOutputBuffer[1024], &aOutputBuffer[2048], (*aOutputLength * 2));
            }
            *aOutputLength *= 2;

        }
#endif
        (*aIsFirstBuffer)++;

        //After decoding the first frame, modify all the input & output port settings
        if (1 == *aIsFirstBuffer)
        {
            StreamType = (Int32) RetrieveDecodedStreamType();

            if ((0 == StreamType) && (2 == iConfigUpSamplingFactor))
            {
                PVMP4AudioDecoderDisableAacPlus(&iExt, &ipMem);
                *aSamplesPerFrame = AACDEC_PCM_FRAME_SAMPLE_SIZE;
                aAudioAacParam->eAACProfile = OMX_AUDIO_AACObjectMain;
                aAudioAacParam->nFrameLength = AACDEC_PCM_FRAME_SAMPLE_SIZE;
            }

            //Output Port Parameters
            aAudioPcmParam->nSamplingRate = iExt.samplingRate;
            aAudioPcmParam->nChannels = iExt.desiredChannels;

            //Input Port Parameters
            aAudioAacParam->nSampleRate = iExt.samplingRate;

            //Set the Resize flag to send the port settings changed callback
            *aResizeFlag = OMX_TRUE;
        }

        return Status;

    }
    else if (MP4AUDEC_INCOMPLETE_FRAME == Status)
    {
        *aInBuffer += iInputUsedLength;
        iInputUsedLength = 0;
    }
    else
    {
        *aInBufSize = 0;
        iInputUsedLength = 0;
    }

    return Status;
}


//Retrieve the Stream Type of AAC input-bitstream
Int OmxAacDecoder::RetrieveDecodedStreamType()
{

    if ((iExt.extendedAudioObjectType == MP4AUDIO_AAC_LC) ||
            (iExt.extendedAudioObjectType == MP4AUDIO_LTP))
    {
        return AAC;   /*  AAC */
    }
    else if (iExt.extendedAudioObjectType == MP4AUDIO_SBR)
    {
        return AACPLUS;   /*  AAC+ */
    }
    else if (iExt.extendedAudioObjectType == MP4AUDIO_PS)
    {
        return ENH_AACPLUS;   /*  AAC++ */
    }

    return -1;   /*  Error evaluating the stream type */
}


//Change the AACEnable flag, according to the AAC profile set by the client in SetParameter API
void OmxAacDecoder::UpdateAACPlusEnabled(OMX_BOOL flag)
{
    //Mark this flag as false if client sets any non HE AAC profile in SetParameter call
    iExt.aacPlusEnabled = (OMX_TRUE == flag) ? true : false;
}
