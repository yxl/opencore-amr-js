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
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#include "mp3_dec.h"
#include "pvmp3decoder_api.h"

Mp3Decoder::Mp3Decoder()
{
    iMP3DecExt = NULL;
    iAudioMp3Decoder = NULL;
    iInputUsedLength = 0;
    iInitFlag = 0;
}

OMX_BOOL Mp3Decoder::Mp3DecInit(OMX_AUDIO_CONFIG_EQUALIZERTYPE* aEqualizerType)
{
    //Default equalization type
    e_equalization EqualizType = (e_equalization) aEqualizerType->sBandIndex.nValue;

    iAudioMp3Decoder = CPvMP3_Decoder::NewL();

    if (!iAudioMp3Decoder)
    {
        return OMX_FALSE;
    }

    // create iMP3DecExt
    if (!iMP3DecExt)
    {
        iMP3DecExt = OSCL_NEW(tPVMP3DecoderExternal, ());
        if (!iMP3DecExt)
        {
            return OMX_FALSE;
        }
        iMP3DecExt->inputBufferCurrentLength = 0;
    }

    // Initialize the decoder
    //Input/output buffer allocation and CrcEnabler has been kept as false
    iAudioMp3Decoder->StartL(iMP3DecExt, false, false, false, EqualizType);
    iMP3DecExt->inputBufferMaxLength = 512;

    iInitFlag = 0;
    iInputUsedLength = 0;

    return OMX_TRUE;
}

void Mp3Decoder::ResetDecoder()
{
    if (iAudioMp3Decoder)
    {
        iAudioMp3Decoder->ResetDecoderL();
    }
}

void Mp3Decoder::Mp3DecDeinit()
{
    if (iAudioMp3Decoder)
    {
        iAudioMp3Decoder->TerminateDecoderL();
        delete iAudioMp3Decoder;
        iAudioMp3Decoder = NULL;

        if (iMP3DecExt)
        {
            OSCL_DELETE(iMP3DecExt);
            iMP3DecExt = NULL;
        }
    }
}


Int Mp3Decoder::Mp3DecodeAudio(OMX_S16* aOutBuff,
                               OMX_U32* aOutputLength, OMX_U8** aInputBuf,
                               OMX_U32* aInBufSize, OMX_S32* aFrameCount,
                               OMX_AUDIO_PARAM_PCMMODETYPE* aAudioPcmParam,
                               OMX_AUDIO_PARAM_MP3TYPE* aAudioMp3Param,
                               OMX_BOOL aMarkerFlag,
                               OMX_BOOL* aResizeFlag)
{

    int32 Status = MP3DEC_SUCCESS;
    *aResizeFlag = OMX_FALSE;


    if (iInitFlag == 0)
    {
        //Initialization is required again when the client inbetween rewinds the input bitstream
        //Added to pass khronous conformance tests
        if (*aFrameCount != 0)
        {
            e_equalization EqualizType = iMP3DecExt->equalizerType;
            iMP3DecExt->inputBufferCurrentLength = 0;
            iInputUsedLength = 0;
            iAudioMp3Decoder->StartL(iMP3DecExt, false, false, false, EqualizType);
        }

        iInitFlag = 1;
    }

    iMP3DecExt->pInputBuffer = *aInputBuf + iInputUsedLength;
    iMP3DecExt->pOutputBuffer = &aOutBuff[0];

    iMP3DecExt->inputBufferCurrentLength = *aInBufSize;
    iMP3DecExt->inputBufferUsedLength = 0;

    if (OMX_FALSE == aMarkerFlag)
    {
        //If the input buffer has finished off, do not check the frame boundaries just return from here
        //This will detect the EOS for without marker test case.
        if (0 == iMP3DecExt->inputBufferCurrentLength)
        {
            iInputUsedLength = 0;
            return MP3DEC_INCOMPLETE_FRAME;
        }
        //If the marker flag is not set, find out the frame boundaries
        else
        {
            Status = iAudioMp3Decoder->SeekMp3Synchronization(iMP3DecExt);

            if (1 == Status)
            {
                if (0 == iMP3DecExt->inputBufferCurrentLength)
                {
                    //This indicates the case of corrupt frame, discard input bytes equal to inputBufferMaxLength
                    *aInBufSize -= iMP3DecExt->inputBufferMaxLength;
                    iInputUsedLength += iMP3DecExt->inputBufferMaxLength;
                    iMP3DecExt->inputBufferUsedLength += iMP3DecExt->inputBufferMaxLength;;

                    //return sucess so that we can continue decoding with rest of the buffer,
                    //after discarding the corrupted bit-streams
                    return MP3DEC_SUCCESS;
                }
                else
                {
                    *aInputBuf += iInputUsedLength;
                    iMP3DecExt->inputBufferUsedLength = 0;
                    iInputUsedLength = 0;
                    return MP3DEC_INCOMPLETE_FRAME;
                }
            }
        }
    }

    Status = iAudioMp3Decoder->ExecuteL(iMP3DecExt);

    if (MP3DEC_SUCCESS == Status)
    {
        *aInBufSize -= iMP3DecExt->inputBufferUsedLength;

        if (0 == *aInBufSize)
        {
            iInputUsedLength = 0;
        }
        else
        {
            iInputUsedLength += iMP3DecExt->inputBufferUsedLength;
        }

        *aOutputLength = iMP3DecExt->outputFrameSize * iMP3DecExt->num_channels;

        //After decoding the first frame, update all the input & output port settings
        if (0 == *aFrameCount)
        {

            //Output Port Parameters
            aAudioPcmParam->nSamplingRate = iMP3DecExt->samplingRate;
            aAudioPcmParam->nChannels = iMP3DecExt->num_channels;

            //Input Port Parameters
            aAudioMp3Param->nSampleRate = iMP3DecExt->samplingRate;

            //Set the Resize flag to send the port settings changed callback
            *aResizeFlag = OMX_TRUE;
        }

        (*aFrameCount)++;

        return Status;

    }
    else if (Status == MP3DEC_INVALID_FRAME)
    {
        *aInBufSize = 0;
        iInputUsedLength = 0;
    }
    else if (Status == MP3DEC_INCOMPLETE_FRAME)
    {
        *aInputBuf += iInputUsedLength;
        iMP3DecExt->inputBufferUsedLength = 0;
        iInputUsedLength = 0;
    }
    else
    {
        *aInputBuf += iInputUsedLength;
        iInputUsedLength = 0;
    }

    (*aFrameCount)++;

    return Status;

}
