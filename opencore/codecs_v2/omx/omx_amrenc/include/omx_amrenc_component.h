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
	@file omx_amrenc_component.h
	OpenMax encoder component file.
*/

#ifndef OMX_AMRENC_COMPONENT_H_INCLUDED
#define OMX_AMRENC_COMPONENT_H_INCLUDED

#ifndef PV_OMXCOMPONENT_H_INCLUDED
#include "pv_omxcomponent.h"
#endif

#ifndef AMR_DEC_H_INCLUDED
#include "amr_enc.h"
#endif


#define INPUT_BUFFER_SIZE_AMR_ENC (320 * MAX_NUM_OUTPUT_FRAMES_PER_BUFFER)	//3200

#define OUTPUT_BUFFER_SIZE_AMR_ENC (MAX_NUM_OUTPUT_FRAMES_PER_BUFFER * MAX_AMR_FRAME_SIZE)	//10 * 32 = 320

#define NUMBER_INPUT_BUFFER_AMR_ENC  5
#define NUMBER_OUTPUT_BUFFER_AMR_ENC  2


#define omx_min(a, b)  ((a) <= (b) ? (a) : (b));

class OmxComponentAmrEncoderAO : public OmxComponentAudio
{
    public:

        OmxComponentAmrEncoderAO();
        ~OmxComponentAmrEncoderAO();


        OMX_ERRORTYPE ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy);
        OMX_ERRORTYPE DestroyComponent();

        OMX_ERRORTYPE ComponentInit();
        OMX_ERRORTYPE ComponentDeInit();

        static void ComponentGetRolesOfComponent(OMX_STRING* aRoleString);

        void ProcessData();

        void ProcessInBufferFlag();
        void SyncWithInputTimestamp();

    private:

        OMX_U32			iInputFrameLength;
        OMX_U32			iMaxNumberOutputFrames;
        OMX_U32			iActualNumberOutputFrames;

        OMX_TICKS		iCurrentTimestamp;
        OmxAmrEncoder*  ipAmrEnc;
};

#endif // OMX_AMRENC_COMPONENT_H_INCLUDED
