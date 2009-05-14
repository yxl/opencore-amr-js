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
 *
 * @file pvmf_loopback_ioport.h
 * @brief Input/Output port for simple loopback node
 *
 */

#ifndef PVMF_LOOPBACK_IOPORT_H_INCLUDED
#define PVMF_LOOPBACK_IOPORT_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PVMF_PORT_INTERFACE_H_INCLUDED
#include "pvmf_port_interface.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
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

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

// Capability mime strings
#define PVMF_LOOPBACK_PORT_INPUT_FORMATS ".../input_formats"
#define PVMF_LOOPBACK_PORT_INPUT_FORMATS_VALTYPE ".../input_formats;valtype=uint32"

class PVLogger;
class PVMFLoopbackNode;

class PVMFLoopbackIOPort : public PvmfPortBaseImpl
            , public PvmiCapabilityAndConfigPortFormatImpl
            , public OsclActiveObject
            , public PVMFPortActivityHandler
{
    public:
        PVMFLoopbackIOPort(int32 aTag
                           , PVMFLoopbackNode* aNode);

        PVMFLoopbackIOPort(int32 aTag
                           , PVMFLoopbackNode* aNode
                           , uint32 aInCapacity
                           , uint32 aInReserve
                           , uint32 aInThreshold
                           , uint32 aOutCapacity
                           , uint32 aOutReserve
                           , uint32 aOutThreshold);

        ~PVMFLoopbackIOPort();

        // from PvmiCapabilityAndConfigPortFormatImpl
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

        void Reset();

        void SetDestNode(PVMFLoopbackNode* aNode)
        {
            iNode = aNode;
        }

    private:

        void Construct(PVMFLoopbackNode*);
        PVMFLoopbackNode* iNode;

        uint32 iTimedSendBytes;
        uint32 iWriteDataLength;
        uint32 iFirstSendStartTime;

        PVLogger *iLogger;

        //from OsclActiveObject
        void Run();

        //from PVMFPortActivityHandler
        void HandlePortActivity(const PVMFPortActivity &);

        //for port data processing.
        bool iWaiting;
        PVMFStatus ProcessIncomingMsg();

        friend class PVMFLoopbackNode;

        friend class Oscl_TAlloc<PVMFLoopbackIOPort, OsclMemAllocator>;

};

#endif // PVMF_LOOPBACK_IOPORT_H_INCLUDED
