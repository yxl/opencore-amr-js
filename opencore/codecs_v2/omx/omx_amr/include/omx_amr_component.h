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
	@file src/base/amr_component.h

	OpenMax base_component component. This component does not perform any multimedia
	processing.	It is used as a base_component for new components development.

*/

#ifndef OMX_AMR_COMPONENT_H_INCLUDED
#define OMX_AMR_COMPONENT_H_INCLUDED

#ifndef PV_OMXCOMPONENT_H_INCLUDED
#include "pv_omxcomponent.h"
#endif

#ifndef AMR_DEC_H_INCLUDED
#include "amr_dec.h"
#endif

#define INPUT_BUFFER_SIZE_AMR 2000
#define OUTPUT_BUFFER_SIZE_AMR 640

#define NUMBER_INPUT_BUFFER_AMR  10
#define NUMBER_OUTPUT_BUFFER_AMR  9


class OpenmaxAmrAO : public OmxComponentAudio
{
    public:

        OpenmaxAmrAO();
        ~OpenmaxAmrAO();


        OMX_ERRORTYPE ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy);
        OMX_ERRORTYPE DestroyComponent();

        OMX_ERRORTYPE ComponentInit();
        OMX_ERRORTYPE ComponentDeInit();

        static void ComponentGetRolesOfComponent(OMX_STRING* aRoleString);
        void ProcessData();

        void DecodeWithoutMarker();
        void DecodeWithMarker();

        void ComponentBufferMgmtWithoutMarker();
        void SyncWithInputTimestamp();
        void ProcessInBufferFlag();

        void ResetComponent();

        OMX_ERRORTYPE GetConfig(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_INOUT OMX_PTR pComponentConfigStructure);

    private:

        void CheckForSilenceInsertion();
        void DoSilenceInsertion();

        OMX_TICKS				iCurrentTimestamp;
        OMX_S32 				iPreviousFrameLength;
        OMX_U32					iZeroFramesNeeded;

        OmxAmrDecoder* ipAmrDec;
};

#endif // OMX_AMR_COMPONENT_H_INCLUDED
