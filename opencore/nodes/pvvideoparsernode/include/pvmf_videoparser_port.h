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
#ifndef PVMF_VIDEODEC_PORT_H_INCLUDED
#define PVMF_VIDEODEC_PORT_H_INCLUDED


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

class PVMFVideoParserNode;

//Default vector reserve size
#define PVMF_VIDEOPARSER_NODE_PORT_VECTOR_RESERVE 10

typedef enum
{
    PVMF_VIDEOPARSER_NODE_PORT_TYPE_SINK = 0,
    PVMF_VIDEOPARSER_NODE_PORT_TYPE_SOURCE,
} PVMFVideoParserPortType;

class PVMFVideoParserPort : public PvmfPortBaseImpl
            , public PvmiCapabilityAndConfigPortFormatImpl
{
    public:
        PVMFVideoParserPort(int32 aTag,
                            PVMFFormatType format,
                            PVMFNodeInterface* aNode,
                            uint8* aFormatSpecificInfo, uint32 aFormatSpecificInfoLen,
                            char*);
        ~PVMFVideoParserPort();

        // Implement pure virtuals from PvmiCapabilityAndConfigPortFormatImpl interface
        bool IsFormatSupported(PVMFFormatType);
        void FormatUpdated();

        // this port supports config interface
        void QueryInterface(const PVUuid &aUuid, OsclAny*&aPtr)
        {
            if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
                aPtr = (PvmiCapabilityAndConfig*)this;
            else
                aPtr = NULL;
        }

        /* Over ride Connect() */
        PVMFStatus Connect(PVMFPortInterface* aPort);
        PVMFStatus PeerConnect(PVMFPortInterface* aPort);

        /* Implement pure virtuals from PvmiCapabilityAndConfig interface */
        PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                                     PvmiKvp*& aParameters, int& num_parameter_elements,	PvmiCapabilityContext aContext);
        PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                               int num_elements, PvmiKvp * & aRet_kvp);
        PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);

    private:
        void Construct(int32 aTag, PVMFFormatType format, uint8* aFormatSpecificInfo, uint32 aFormatSpecificInfoLen);

        bool pvmiSetPortFormatSpecificInfoSync(PvmiCapabilityAndConfig *aPort,
                                               const char* aFormatValType);

        bool pvmiGetPortFormatSpecificInfoSync(const char* aFormatValType,
                                               PvmiKvp*& aKvp);

        PVLogger *iLogger;

        // Format specific info associated with the codec in this datapath
        uint8* iFormatSpecificInfo;
        uint32 iFormatSpecificInfoLen;
};

#endif // PVMF_VIDEODEC_PORT_H_INCLUDED
