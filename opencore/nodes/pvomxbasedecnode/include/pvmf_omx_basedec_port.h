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
#ifndef PVMF_OMX_DEC_PORT_H_INCLUDED
#define PVMF_OMX_DEC_PORT_H_INCLUDED


#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PVMF_PORT_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif

#ifndef PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
#endif

#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif

class PVMFOMXBaseDecNode;
class PVMFOMXAudioDecNode;
class PVMFOMXVideoDecNode;


//Default vector reserve size
#define PVMF_OMX_DEC_NODE_PORT_VECTOR_RESERVE 10

#define PVMF_OMX_AUDIO_DEC_PORT_INPUT_FORMATS "x-pvmf/audio/decode/input_formats"
#define PVMF_OMX_AUDIO_DEC_PORT_INPUT_FORMATS_VALTYPE "x-pvmf/port/formattype;valtype=int32"
#define PVMF_OMX_VIDEO_DEC_PORT_INPUT_FORMATS "x-pvmf/video/decode/input_formats"
#define PVMF_OMX_VIDEO_DEC_PORT_INPUT_FORMATS_VALTYPE "x-pvmf/port/formattype;valtype=int32"

#define PVMF_OMX_VIDEO_DEC_INPUT_PORT_NAME "OMXVideoDecIn"
#define PVMF_OMX_VIDEO_DEC_OUTPUT_PORT_NAME "OMXVideoDecOut"
#define PVMF_OMX_AUDIO_DEC_INPUT_PORT_NAME "OMXAudioDecIn"
#define PVMF_OMX_AUDIO_DEC_OUTPUT_PORT_NAME "OMXAudioDecOut"

typedef enum
{
    PVMF_OMX_DEC_NODE_PORT_TYPE_INPUT,
    PVMF_OMX_DEC_NODE_PORT_TYPE_OUTPUT,
} PVMFOMXDecPortType;


class PVMFOMXDecPort : public PvmfPortBaseImpl
            , public PvmiCapabilityAndConfigPortFormatImpl
{
    public:
        OSCL_IMPORT_REF PVMFOMXDecPort(int32 aTag, PVMFNodeInterface* aNode, const char*);
        OSCL_IMPORT_REF ~PVMFOMXDecPort();


        // Implement pure virtuals from PvmiCapabilityAndConfigPortFormatImpl interface
        OSCL_IMPORT_REF bool IsFormatSupported(PVMFFormatType);
        OSCL_IMPORT_REF void FormatUpdated();

        // this port supports config interface
        void QueryInterface(const PVUuid &aUuid, OsclAny*&aPtr)
        {
            if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
                aPtr = (PvmiCapabilityAndConfig*)this;
            else
                aPtr = NULL;
        }

        OSCL_IMPORT_REF bool pvmiSetPortFormatSpecificInfoSync(OsclRefCounterMemFrag& aMemFrag, PvmiKeyType KvpKey);

        OSCL_IMPORT_REF bool pvmiGetBufferAllocatorSpecificInfoSync(PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements);
        OSCL_EXPORT_REF bool releaseParametersSync(PvmiKvp*& aParameters, int& aNumParamElements);

        OSCL_IMPORT_REF PVMFStatus Connect(PVMFPortInterface* aPort);
        OSCL_IMPORT_REF void setParametersSync(PvmiMIOSession aSession,
                                               PvmiKvp* aParameters,
                                               int num_elements,
                                               PvmiKvp * & aRet_kvp);

        OSCL_IMPORT_REF PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        OSCL_IMPORT_REF PVMFStatus verifyConnectedPortParametersSync(const char* aFormatValType, OsclAny* aConfig);
        uint32 getTrackConfigSize()
        {
            return iTrackConfigSize;
        }
        uint8* getTrackConfig()
        {
            return iTrackConfig;
        }



    private:


        OSCL_IMPORT_REF void Construct();
        PVLogger *iLogger;
        uint32 iNumFramesGenerated; //number of source frames generated.
        uint32 iNumFramesConsumed; //number of frames consumed & discarded.
        uint32 iTrackConfigSize;
        uint8* iTrackConfig;
        friend class PVMFOMXBaseDecNode;
        friend class PVMFOMXVideoDecNode;
        friend class PVMFOMXAudioDecNode;
        PVMFOMXBaseDecNode* iOMXNode;
};

#endif // PVMF_OMX_DEC_PORT_H_INCLUDED
