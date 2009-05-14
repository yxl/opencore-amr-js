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
	@file omx_aac_component.h
	OpenMax decoder_component component.

*/

#ifndef OMX_AAC_COMPONENT_H_INCLUDED
#define OMX_AAC_COMPONENT_H_INCLUDED

#ifndef PV_OMXCOMPONENT_H_INCLUDED
#include "pv_omxcomponent.h"
#endif

#ifndef AAC_DEC_H_INCLUDED
#include "aac_dec.h"
#endif

#ifndef AAC_TIMESTAMP_H_INCLUDED
#include "aac_timestamp.h"
#endif


#define INPUT_BUFFER_SIZE_AAC 1536
#define OUTPUT_BUFFER_SIZE_AAC 8192

#define NUMBER_INPUT_BUFFER_AAC  10
#define NUMBER_OUTPUT_BUFFER_AAC  9


class OpenmaxAacAO : public OmxComponentAudio
{
    public:

        OpenmaxAacAO();
        ~OpenmaxAacAO();

        OMX_ERRORTYPE ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy);
        OMX_ERRORTYPE DestroyComponent();

        OMX_ERRORTYPE ComponentInit();
        OMX_ERRORTYPE ComponentDeInit();

        static void ComponentGetRolesOfComponent(OMX_STRING* aRoleString);
        void ProcessData();
        void SyncWithInputTimestamp();
        void ProcessInBufferFlag();

        void UpdateAACPlusFlag(OMX_BOOL aAacPlusFlag);
        void ResetComponent();
        OMX_ERRORTYPE GetConfig(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_INOUT OMX_PTR pComponentConfigStructure);


    private:

        void CheckForSilenceInsertion();
        void DoSilenceInsertion();


        AacTimeStampCalc iCurrentFrameTS;
        OmxAacDecoder* ipAacDec;
};



#endif // OMX_AAC_COMPONENT_H_INCLUDED
