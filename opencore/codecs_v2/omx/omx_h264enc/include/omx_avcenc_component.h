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
	@file omx_avcenc_component.h
	OpenMax encoder_component component.

*/

#ifndef OMX_AVCENC_COMPONENT_H_INCLUDED
#define OMX_AVCENC_COMPONENT_H_INCLUDED

#ifndef PV_OMXCOMPONENT_H_INCLUDED
#include "pv_omxcomponent.h"
#endif

#ifndef AVC_ENC_H_INCLUDED
#include "avc_enc.h"
#endif


#define INPUT_BUFFER_SIZE_AVCENC 38016			//(176 * 144 * 1.5) for YUV 420 format.

#if (defined(TEST_FULL_AVC_FRAME_MODE) || defined(TEST_FULL_AVC_FRAME_MODE_SC))
#define OUTPUT_BUFFER_SIZE_AVCENC 38581 // (20 + 4 * MAX_NAL_PER_FRAME + 20 + 6) is size of extra data
#else
#define OUTPUT_BUFFER_SIZE_AVCENC 38135
#endif

#define NUMBER_INPUT_BUFFER_AVCENC  5
#define NUMBER_OUTPUT_BUFFER_AVCENC  2

class OmxComponentAvcEncAO : public OmxComponentVideo
{
    public:

        OmxComponentAvcEncAO();
        ~OmxComponentAvcEncAO();

        OMX_ERRORTYPE ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy);
        OMX_ERRORTYPE DestroyComponent();

        OMX_ERRORTYPE ComponentInit();
        OMX_ERRORTYPE ComponentDeInit();

        static void ComponentGetRolesOfComponent(OMX_STRING* aRoleString);
        void ProcessInBufferFlag();

        void ProcessData();

        OMX_ERRORTYPE SetConfig(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_IN  OMX_PTR pComponentConfigStructure);

    private:

        OMX_BOOL CopyDataToOutputBuffer();
        OMX_BOOL AppendExtraDataToBuffer(OMX_BUFFERHEADERTYPE* aOutputBuffer,
                                         OMX_EXTRADATATYPE aType,
                                         OMX_U8* aExtraData,
                                         OMX_U8 aDataLength);
        void ManageFrameBoundaries();

        AvcEncoder_OMX*   ipAvcEncoderObject;

        OMX_BOOL		  iBufferOverRun;
        OMX_U8*			  ipInternalOutBuffer;
        OMX_U32			  iInternalOutBufFilledLen;
        OMX_TICKS		  iOutputTimeStamp;
        OMX_BOOL		  iSyncFlag;
        OMX_BOOL          iEndOfOutputFrame;

        OMX_U32			  iNALSizeArray[MAX_NAL_PER_FRAME];
        OMX_U32			  iNALSizeSum;
        OMX_U32			  iNALCount;
};

#endif // OMX_AVCENC_COMPONENT_H_INCLUDED
