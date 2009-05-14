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
 * @file pvmf_protocol_engine_port.h
 */

#ifndef PVMF_PROTOCOLENGINE_PORT_H_INCLUDED
#define PVMF_PROTOCOLENGINE_PORT_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
#endif


//Default vector reserve size
#define PVMF_PROTOCOLENGINE_NODE_PORT_VECTOR_RESERVE 10

const PVMFStatus PVMFSuccessOutgoingMsgSent = 10;


class PVMFProtocolEnginePort : public PvmfPortBaseImpl,
            public PvmiCapabilityAndConfigPortFormatImpl
{
    public:
        PVMFProtocolEnginePort(int32 aTag,
                               PVMFNodeInterface* aNode, const char*);

        PVMFProtocolEnginePort(int32 aTag,
                               PVMFNodeInterface* aNode,
                               uint32 aInCapacity,
                               uint32 aInReserve,
                               uint32 aInThreshold,
                               uint32 aOutCapacity,
                               uint32 aOutReserve,
                               uint32 aOutThreshold, const char*);

        virtual ~PVMFProtocolEnginePort();

        // Implement pure virtuals from PvmiCapabilityAndConfigPortFormatImpl interface
        bool IsFormatSupported(PVMFFormatType);
        void FormatUpdated();

        // this port supports config interface
        void QueryInterface(const PVUuid &aUuid, OsclAny*&aPtr)
        {
            if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
            {
                aPtr = (PvmiCapabilityAndConfig*)this;
            }
            else
            {
                aPtr = NULL;
            }
        }

        // override the base implementation
        PVMFStatus QueueOutgoingMsg(PVMFSharedMediaMsgPtr aMsg);

        bool PeekOutgoingMsg(PVMFSharedMediaMsgPtr& aMsg);

    private:
        void Construct();

        PVLogger *iLogger;
        uint32 iNumFramesGenerated; //number of source frames generated.
        uint32 iNumFramesConsumed; //number of frames consumed & discarded.
        friend class PVMFProtocolEngineNode;
};

#endif // PVMF_PROTOCOLENGINE_PORT_H_INCLUDED



