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
	@file omx_mpeg4enc_component.h
	OpenMax encoder_component component.

*/

#ifndef OMX_MPEG4ENC_COMPONENT_H_INCLUDED
#define OMX_MPEG4ENC_COMPONENT_H_INCLUDED

#ifndef PV_OMXCOMPONENT_H_INCLUDED
#include "pv_omxcomponent.h"
#endif

#ifndef MPEG4_ENC_H_INCLUDED
#include "mpeg4_enc.h"
#endif


#define INPUT_BUFFER_SIZE_MP4ENC 38016			//(176 * 144 * 1.5) for YUV 420 format.
#define OUTPUT_BUFFER_SIZE_MP4ENC 8192


#define NUMBER_INPUT_BUFFER_MP4ENC  5
#define NUMBER_OUTPUT_BUFFER_MP4ENC  2



class OmxComponentMpeg4EncAO : public OmxComponentVideo
{
    public:

        OmxComponentMpeg4EncAO();
        ~OmxComponentMpeg4EncAO();

        OMX_ERRORTYPE ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy);
        OMX_ERRORTYPE DestroyComponent();

        OMX_ERRORTYPE ComponentInit();
        OMX_ERRORTYPE ComponentDeInit();

        static void ComponentGetRolesOfComponent(OMX_STRING* aRoleString);

        void SetEncoderMode(OMX_S32 aMode);
        void ProcessInBufferFlag();

        void ProcessData();

        OMX_ERRORTYPE SetConfig(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_IN  OMX_PTR pComponentConfigStructure);

    private:

        OMX_BOOL CopyDataToOutputBuffer();

        Mpeg4Encoder_OMX* ipMpegEncoderObject;
        OMX_S32			  iEncMode;

        OMX_BOOL		  iBufferOverRun;
        OMX_U8*			  ipInternalOutBuffer;
        OMX_U32			  iInternalOutBufFilledLen;
        OMX_TICKS		  iOutputTimeStamp;
        OMX_BOOL		  iSyncFlag;
};

#endif // OMX_MPEG4ENC_COMPONENT_H_INCLUDED
