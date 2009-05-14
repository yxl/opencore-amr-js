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
/*! \file   decoder_aac.h
 *  \brief  Header file for the AAC MDF wrapper
 *
 */

#ifndef __DECODER_AAC_H
#define __DECODER_AAC_H

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#include "oscl_base.h"
#include "pvmp4audiodecoder_api.h"

#include "e_tmp4audioobjecttype.h"
extern Int PVMP4SetAudioConfig(tPVMP4AudioDecoderExternal *pExt, void *pMem, Int upsamplingFactor, Int samp_rate, Int num_ch, tMP4AudioObjectType audioObjectType);

/*!
 *  \brief  number of samples per frame (decoded frames)
 */

#define KAAC_NUM_SAMPLES_PER_FRAME      1024
#define KAAC_MAX_STREAMING_BUFFER_SIZE  (PVMP4AUDIODECODER_INBUFSIZE * 1)

#define KCAI_CODEC_NO_MEMORY -1
#define KCAI_CODEC_INIT_FAILURE -2



/*!
 *  \class  CDecoder_AAC "include/decoder_aac.h"
 *  \brief  AAC Decoder class
 */


// CDecoder_AAC
class CDecoder_AAC
{
    public:
        OSCL_IMPORT_REF void ConstructL();
        OSCL_IMPORT_REF static CDecoder_AAC *NewL();
        OSCL_IMPORT_REF ~CDecoder_AAC();

        OSCL_IMPORT_REF int32 StartL(tPVMP4AudioDecoderExternal * pExt,
                                     uint8  num_channels,
                                     bool aAllocateInputBuffer = false,
                                     bool aAllocateOutputBuffer = false,
                                     Int  upsamplingFactor = 1,
                                     Int  samp_rate = 44100,
                                     tMP4AudioObjectType  audioObjectType = MP4AUDIO_AAC_LC
                                    );

        OSCL_IMPORT_REF int32 StartL(tPVMP4AudioDecoderExternal * pExt,
                                     uint8  num_channels,
                                     bool aAllocateInputBuffer = false,
                                     bool aAllocateOutputBuffer = false,
                                     bool aAacplusEnabler = true
                                    );
        OSCL_IMPORT_REF int32 ExecuteL(tPVMP4AudioDecoderExternal * pExt);
        OSCL_IMPORT_REF void ResetDecoderL(void);
        OSCL_IMPORT_REF void StopL(void);

        OSCL_IMPORT_REF void TerminateDecoderL();
        OSCL_IMPORT_REF int32 RetrieveDecodedStreamTypeL(tPVMP4AudioDecoderExternal * pExt);

        OSCL_IMPORT_REF void DisableSbrDecodingL(tPVMP4AudioDecoderExternal * pExt);

    private:
        // number of samples per frame per channel (decoded)
        uint32 iNumSamplesPerFrame;
        bool iFirstFrame;
        uint8 *pMem;

        bool iAllocateInputBuffer;
        bool iAllocateOutputBuffer;

        uint8* iInputBuf;
        int16* iOutputBuf;
};


#endif

