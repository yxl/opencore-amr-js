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
/**
	@file omx_mpeg4_component.h
	OpenMax decoder_component component.

*/

#ifndef OMX_MPEG4_COMPONENT_H_INCLUDED
#define OMX_MPEG4_COMPONENT_H_INCLUDED

#ifndef PV_OMXCOMPONENT_H_INCLUDED
#include "pv_omxcomponent.h"
#endif

#ifndef MPEG4_DEC_H_INCLUDED
#include "mpeg4_dec.h"
#endif


#define INPUT_BUFFER_SIZE_MP4 16000
#define OUTPUT_BUFFER_SIZE_MP4 152064

#define NUMBER_INPUT_BUFFER_MP4  10
#define NUMBER_OUTPUT_BUFFER_MP4  2

#define MINIMUM_H263_SHORT_HEADER_SIZE 12
/**
 * The structure for port Type.
 */
enum
{
    MODE_H263 = 0,
    MODE_MPEG4
};


class OpenmaxMpeg4AO : public OmxComponentVideo
{
    public:

        OpenmaxMpeg4AO();
        ~OpenmaxMpeg4AO();

        OMX_ERRORTYPE ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy);
        OMX_ERRORTYPE DestroyComponent();

        OMX_ERRORTYPE ComponentInit();
        OMX_ERRORTYPE ComponentDeInit();

        static void ComponentGetRolesOfComponent(OMX_STRING* aRoleString);

        void SetDecoderMode(int);
        void ComponentBufferMgmtWithoutMarker();
        void ProcessData();
        void DecodeWithoutMarker();
        void DecodeWithMarker();

        OMX_ERRORTYPE GetConfig(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_INOUT OMX_PTR pComponentConfigStructure);

        OMX_ERRORTYPE ReAllocatePartialAssemblyBuffers(OMX_BUFFERHEADERTYPE* aInputBufferHdr);

    private:

        OMX_BOOL DecodeH263Header(OMX_U8* aInputBuffer, OMX_U32* aBufferSize);

        void ReadBits(OMX_U8* aStream, uint8 aNumBits, uint32* aOutData);

        OMX_BOOL				iUseExtTimestamp;
        Mpeg4Decoder_OMX* ipMpegDecoderObject;
        OMX_S32 iDecMode;

        //Parameters required for H.263 source format parsing
        OMX_U32 iH263DataBitPos;
        OMX_U32	iH263BitPos;
        OMX_U32 iH263BitBuf;
};




#endif // OMX_MPEG4_COMPONENT_H_INCLUDED
