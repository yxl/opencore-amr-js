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
#ifndef AMR_ENC_H_INCLUDED
#define AMR_ENC_H_INCLUDED

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OMX_Component_h
#include "OMX_Component.h"
#endif

#ifndef GSMAMR_ENCODER_H_INCLUDED
#include "gsmamr_encoder_wrapper.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif


#define AMR_FRAME_LENGTH_IN_TIMESTAMP 20
#define MAX_AMR_FRAME_SIZE 32
#define MAX_NUM_OUTPUT_FRAMES_PER_BUFFER 10


class OmxAmrEncoder
{
    public:
        OmxAmrEncoder();

        OMX_BOOL AmrEncInit(OMX_AUDIO_PARAM_PCMMODETYPE aPcmMode,
                            OMX_AUDIO_PARAM_AMRTYPE aAmrParam,
                            OMX_U32* aInputFrameLength,
                            OMX_U32* aMaxNumberOutputFrames);

        void AmrEncDeinit();

        OMX_BOOL AmrEncodeFrame(OMX_U8* aOutputBuffer,
                                OMX_U32* aOutputLength,
                                OMX_U8* aInBuffer,
                                OMX_U32 aInBufSize,
                                OMX_TICKS aInTimeStamp,
                                OMX_TICKS* aOutTimeStamp);

    private:

        //Codec and encoder settings
        CPvGsmAmrEncoder*  ipGsmEncoder;
        TEncodeProperties* ipEncProps;

        //Encoding Settings parameters

        int32*			 ipSizeArrayForOutputFrames;
        uint32			 iNextStartTime;
        uint32			 iMaxNumOutputFramesPerBuffer;
        uint32			 iOneInputFrameLength;
        uint32			 iMaxInputSize;
        PVMFFormatType	 iOutputFormat;

        OMX_S32 iAmrInitFlag;

};



#endif	//#ifndef AMR_ENC_H_INCLUDED

