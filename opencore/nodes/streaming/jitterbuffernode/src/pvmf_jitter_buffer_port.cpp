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
 * @file pvmf_jitter_buffer_port.cpp
 * @brief PVMF Jitter Buffer Port implementation
 */

#ifndef PVMF_JITTER_BUFFER_PORT_H_INCLUDED
#include "pvmf_jitter_buffer_port.h"
#endif
#ifndef OSCL_MEM_BASIC_FUNCTIONS_H
#include "oscl_mem_basic_functions.h"
#endif
#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMF_JITTER_BUFFER_H_INCLUDED
#include "pvmf_jitter_buffer.h"
#endif
#ifndef PVMF_RTCP_TIMER_H_INCLUDED
#include "pvmf_rtcp_timer.h"
#endif
#ifndef PVMF_STREAMING_MEM_CONFIG_H_INCLUDED
#include "pvmf_streaming_mem_config.h"
#endif
#ifndef PVMF_JITTER_BUFFER_NODE_H_INCLUDED
#include "pvmf_jitter_buffer_node.h"
#endif

#define PVMF_JB_PORT_OVERRIDE 1

////////////////////////////////////////////////////////////////////////////
PVMFJitterBufferPort::PVMFJitterBufferPort(int32 aTag, PVMFJitterBufferNode& aNode, const char*name)
        : PvmfPortBaseImpl(aTag, &aNode, name)
        , iFormat(PVMF_MIME_FORMAT_UNKNOWN)
        , irJitterBufferNode(aNode)
{
    Construct();
}

////////////////////////////////////////////////////////////////////////////
PVMFJitterBufferPort::PVMFJitterBufferPort(int32 aTag, PVMFJitterBufferNode& aNode
        , uint32 aInCapacity
        , uint32 aInReserve
        , uint32 aInThreshold
        , uint32 aOutCapacity
        , uint32 aOutReserve
        , uint32 aOutThreshold, const char*name)
        : PvmfPortBaseImpl(aTag, &aNode, aInCapacity, aInReserve, aInThreshold, aOutCapacity, aOutReserve, aOutThreshold, name)
        , iFormat(PVMF_MIME_FORMAT_UNKNOWN)
        , irJitterBufferNode(aNode)
{
    Construct();
}

////////////////////////////////////////////////////////////////////////////
void PVMFJitterBufferPort::Construct()
{
    iPortParams = NULL;
    iCounterpartPortParams = NULL;
    iPortCounterpart = NULL;
    ipLogger = PVLogger::GetLoggerObject("PVMFJitterBufferPort");
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
    /*
     * Input ports have tags: 0, 3, 6, ...
     * Output ports have tags: 1, 4, 7, ...
     * Feedback ports have tags: 2, 5, 8, ...
     */
    if (iTag % 3)
    {
        if (iTag % 3 == 1)
        {
            iPortType = PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT;
        }
        else if (iTag % 3 == 2)
        {
            iPortType = PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK;
        }
    }
    else
    {
        iPortType = PVMF_JITTER_BUFFER_PORT_TYPE_INPUT;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFJitterBufferPort::~PVMFJitterBufferPort()
{
    Disconnect();
    ClearMsgQueues();
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFJitterBufferPort::Connect(PVMFPortInterface* aPort)
{
    PVMF_JBNODE_LOGINFO((0, "0x%x PVMFJitterBufferPort::Connect: aPort=0x%x", this, aPort));

    if (!aPort)
    {
        PVMF_JBNODE_LOGERROR((0, "0x%x PVMFJitterBufferPort::Connect: Error - Connecting to invalid port", this));
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        PVMF_JBNODE_LOGERROR((0, "0x%x PVMFJitterBufferPort::Connect: Error - Already connected", this));
        return PVMFFailure;
    }

    if ((iPortType == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT) ||
            (iPortType == PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK))
    {
        OsclAny* temp = NULL;
        aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
        PvmiCapabilityAndConfig *config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);

        if (config != NULL)
        {
            int numKvp = 0;
            PvmiKvp* kvpPtr = NULL;
            PVMFStatus status =
                config->getParametersSync(NULL, (char*)PVMI_PORT_CONFIG_INPLACE_DATA_PROCESSING_KEY, kvpPtr, numKvp, NULL);
            if (status == PVMFSuccess)
            {
                bool inPlaceDataProcessing = kvpPtr[0].value.bool_value;
                irJitterBufferNode.SetInPlaceProcessingMode(OSCL_STATIC_CAST(PVMFPortInterface*, this),
                        inPlaceDataProcessing);
            }
            config->releaseParameters(NULL, kvpPtr, numKvp);
        }
    }

    //Automatically connect the peer.
    if (aPort->PeerConnect(this) != PVMFSuccess)
    {
        PVMF_JBNODE_LOGERROR((0, "0x%x PVMFJitterBufferPort::Connect: Error - Peer Connect failed", this));
        return PVMFFailure;
    }

    iConnectedPort = aPort;

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;
}
////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFJitterBufferPort::getParametersSync(PvmiMIOSession aSession,
        PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters,
        int& num_parameter_elements,
        PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aIdentifier);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(num_parameter_elements);
    OSCL_UNUSED_ARG(aContext);

    return PVMFErrNotSupported;
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFJitterBufferPort::releaseParameters(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(aSession);

    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferPort::releaseParameters: aSession=0x%x, aParameters=0x%x, num_elements=%d",
                         aSession, aParameters, num_elements));

    if ((num_elements != 1) ||
            (pv_mime_strcmp(aParameters->key, PVMF_JITTER_BUFFER_PORT_SPECIFIC_ALLOCATOR_VALTYPE) != 0))
    {
        PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferPort::releaseParameters: Error - Not a PvmiKvp created by this port"));
        return PVMFFailure;
    }

    OsclMemAllocator alloc;
    alloc.deallocate((OsclAny*)(aParameters));
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
void PVMFJitterBufferPort::setParametersSync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements,
        PvmiKvp * & aRet_kvp)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);

    aRet_kvp = NULL;
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFJitterBufferPort::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);

    return PVMFErrNotSupported;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFJitterBufferPort::QueueOutgoingMsg(PVMFSharedMediaMsgPtr aMsg)
{
#if PVMF_JB_PORT_OVERRIDE
    if (iPortType == PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT)
    {
        PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferPort::QueueOutgoingMsg"));
        //If port is not connected, don't accept data on the
        //outgoing queue.
        if (!iConnectedPort)
        {
            PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferPort::QueueOutgoingMsg: Error - Port not connected"));
            return PVMFFailure;
        }

        PvmfPortBaseImpl* cpPort = OSCL_STATIC_CAST(PvmfPortBaseImpl*, iConnectedPort);

        // Connected Port incoming Queue is in busy / flushing state.  Do not accept more outgoing messages
        // until the queue is not busy, i.e. queue size drops below specified threshold or FlushComplete
        // is called.
        if (cpPort->iIncomingQueue.iBusy)
        {
            PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferPort::QueueOutgoingMsg: Connected Port Incoming queue in busy / flushing state"));
            return PVMFErrBusy;
        }

        // Add message to outgoing queue and notify the node of the activity
        // There is no need to trap the push_back, since it cannot leave in this usage
        // Reason being that we do a reserve in the constructor and we do not let the
        // port queues grow indefinitely (we either a connected port busy or outgoing Q busy
        // before we reach the reserved limit
        PVMFStatus status = cpPort->Receive(aMsg);

        if (status != PVMFSuccess)
        {
            OSCL_ASSERT(false);
        }

        // Outgoing queue size is at capacity and goes into busy state. The owner node is
        // notified of this transition into busy state.
        if (cpPort->isIncomingFull())
        {
            PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferPort::QueueOutgoingMsg: Connected Port incoming queue is full. Goes into busy state"));
            cpPort->iIncomingQueue.iBusy = true;
            PvmfPortBaseImpl::PortActivity(PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY);
        }
        return PVMFSuccess;
    }
    else
    {
        return (PvmfPortBaseImpl::QueueOutgoingMsg(aMsg));
    }
#else
    return (PvmfPortBaseImpl::QueueOutgoingMsg(aMsg));
#endif
}

bool PVMFJitterBufferPort::IsOutgoingQueueBusy()
{
#if PVMF_JB_PORT_OVERRIDE
    if (iPortType == PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT)
    {
        if (iConnectedPort != NULL)
        {
            PvmfPortBaseImpl* cpPort = OSCL_STATIC_CAST(PvmfPortBaseImpl*, iConnectedPort);
            return (cpPort->iIncomingQueue.iBusy);
        }
    }
    return (PvmfPortBaseImpl::IsOutgoingQueueBusy());;
#else
    return (PvmfPortBaseImpl::IsOutgoingQueueBusy());
#endif
}







