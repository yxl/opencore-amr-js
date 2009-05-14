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
#ifndef MP3_DEC_H_INCLUDED
#define MP3_DEC_H_INCLUDED

#ifndef OMX_Component_h
#include "OMX_Component.h"
#endif


#ifndef PVMP3_DECODER_H
#include "pvmp3_decoder.h"
#endif


class Mp3Decoder
{
    public:

        Mp3Decoder();

        OMX_BOOL Mp3DecInit(OMX_AUDIO_CONFIG_EQUALIZERTYPE* aEqualizerType);
        void Mp3DecDeinit();

        Int Mp3DecodeAudio(OMX_S16* aOutBuff,
                           OMX_U32* aOutputLength, OMX_U8** aInputBuf,
                           OMX_U32* aInBufSize,
                           OMX_S32* aFrameCount,
                           OMX_AUDIO_PARAM_PCMMODETYPE* aAudioPcmParam,
                           OMX_AUDIO_PARAM_MP3TYPE* aAudioMp3Param,
                           OMX_BOOL aMarkerFlag,
                           OMX_BOOL* aResizeFlag);

        void ResetDecoder(); // for repositioning

        OMX_S32 iInputUsedLength;
        OMX_S32 iInitFlag;

    private:

        CPvMP3_Decoder* iAudioMp3Decoder;
        tPVMP3DecoderExternal* iMP3DecExt;

};



#endif	//#ifndef MP3_DEC_H_INCLUDED

