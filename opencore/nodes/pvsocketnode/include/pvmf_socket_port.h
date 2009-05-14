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
#ifndef PVMF_SOCKET_PORT_H_INCLUDED
#define PVMF_SOCKET_PORT_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef OSCL_SOCKET_TYPES_H_INCLUDED
#include "oscl_socket_types.h"
#endif
#ifndef OSCL_SOCKET_H_INCLUDED
#include "oscl_socket.h"
#endif
#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#include "pvmf_node_interface.h"

#ifndef PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
#endif
#ifndef PVMI_PORT_CONFIG_KVP_H_INCLUDED
#include "pvmi_port_config_kvp.h"
#endif
#ifndef PVMI_PORT_CONFIG_KVP_H_INCLUDED
#include "pvmi_port_config_kvp.h"
#endif

//Default vector reserve size
#define PVMF_SOCKET_NODE_PORT_VECTOR_RESERVE 10

// Capability mime strings
// Capability mime strings
#define PVMF_SOCKET_PORT_SPECIFIC_ALLOCATOR "x-pvmf/pvmfstreaming/socketmemallocator"
#define PVMF_SOCKET_PORT_SPECIFIC_ALLOCATOR_VALTYPE "x-pvmf/pvmfstreaming/socketmemallocator;valtype=ksv"

/** Enumerated list of port tags supported by this port */
typedef enum
{
    PVMF_SOCKET_NODE_PORT_TYPE_UNKNOWN,
    PVMF_SOCKET_NODE_PORT_TYPE_SOURCE,
    PVMF_SOCKET_NODE_PORT_TYPE_SINK,
    PVMF_SOCKET_NODE_PORT_TYPE_PASSTHRU
} PVMFSocketNodePortTag;

class PVMFSocketNode;
class SocketPortConfig;

class PVMFSocketPort : public PvmfPortBaseImpl,
            public PvmiCapabilityAndConfigPortFormatImpl
{
    public:
        /**
         * Default constructor. Default settings will be used for the data queues.
         * @param aId ID assigned to this port
         * @param aTag Port tag
         * @param aNode Container node
         */
        PVMFSocketPort(int32 aTag, PVMFNodeInterface* aNode);

        /**
         * Constructor that allows the node to configure the data queues of this port.
         * @param aTag Port tag
         * @param aNode Container node
         * @param aSize Data queue capacity. The data queue size will not grow beyond this capacity.
         * @param aReserve Size of data queue for which memory is reserved. This must be
         * less than or equal to the capacity. If this is less than capacity, memory will be
         * allocated when the queue grows beyond the reserve size, but will stop growing at
         * capacity.
         * @param aThreshold Ready-to-receive threshold, in terms of percentage of the data queue capacity.
         * This value should be between 0 - 100.
         */
        PVMFSocketPort(int32 aTag
                       , PVMFNodeInterface* aNode
                       , uint32 aInCapacity
                       , uint32 aInReserve
                       , uint32 aInThreshold
                       , uint32 aOutCapacity
                       , uint32 aOutReserve
                       , uint32 aOutThreshold);

        /** Destructor */
        ~PVMFSocketPort();

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

        PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                                     PvmiKvp*& aParameters, int& num_parameter_elements,	PvmiCapabilityContext aContext);
        void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                               int num_elements, PvmiKvp * & aRet_kvp);
        PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);

        PVMFStatus PeekIncomingMsg(PVMFSharedMediaMsgPtr& aMsg);

        SocketPortConfig* iConfig;
        PVMFSocketNodePortTag iPortTag;

        //overrides from PVMFPortInterface so we can skip the outgoing port queue.
        PVMFStatus QueueOutgoingMsg(PVMFSharedMediaMsgPtr aMsg);
        bool IsOutgoingQueueBusy();

    private:
        void Construct();
        bool pvmiGetPortInPlaceDataProcessingInfoSync(const char* aFormatValType,
                PvmiKvp*& aKvp);
        PVLogger *iLogger;

        uint32 iNumFramesConsumed; //number of frames consumed & discarded.

        friend class PVMFSocketNode;
        friend class Oscl_TAlloc<PVMFSocketPort, OsclMemAllocator>;
        friend class PVMFSocketNodeCustomInterface1Impl;
};

#endif

