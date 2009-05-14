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
* "pvmf_clientserver_socket_port.cpp"
*/
#ifndef PVMF_CLIENTSERVER_SOCKET_PORT_H_INCLUDED
#include "pvmf_clientserver_socket_port.h"
#endif
#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif

////////////////////////////////////////////////////////////////////////////
PVMFClientServerSocketPort::PVMFClientServerSocketPort(int32 aTag,
        PVMFPortActivityHandler* aNode)
        : PvmfPortBaseImpl(aTag, aNode)
{
    Construct();
}


////////////////////////////////////////////////////////////////////////////
PVMFClientServerSocketPort::PVMFClientServerSocketPort(int32 aTag
        , PVMFPortActivityHandler* aNode
        , uint32 aInCapacity
        , uint32 aInReserve
        , uint32 aInThreshold
        , uint32 aOutCapacity
        , uint32 aOutReserve
        , uint32 aOutThreshold)
        : PvmfPortBaseImpl(aTag, aNode, aInCapacity, aInReserve,
                           aInThreshold, aOutCapacity, aOutReserve, aOutThreshold)
{
    iPortTag = (PVMFSocketNodePortTag)aTag;
    Construct();
}


////////////////////////////////////////////////////////////////////////////
void PVMFClientServerSocketPort::Construct()
{
    iConfig = NULL;
    iLogger = PVLogger::GetLoggerObject("PVMFClientServerSocketPort");
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
    iNumFramesConsumed = 0;
    PvmiCapabilityAndConfigPortFormatImpl::Construct(PVMF_CLIENTSERVER_SOCKET_PORT_SPECIFIC_ALLOCATOR
            , PVMF_CLIENTSERVER_SOCKET_PORT_SPECIFIC_ALLOCATOR_VALTYPE);
}


////////////////////////////////////////////////////////////////////////////
PVMFClientServerSocketPort::~PVMFClientServerSocketPort()
{
    Disconnect();
}


////////////////////////////////////////////////////////////////////////////
bool PVMFClientServerSocketPort::IsFormatSupported(PVMFFormatType aFmt)
{
    if (aFmt == PVMF_MIME_INET_TCP)
        return true;
    else
        return false;
}


////////////////////////////////////////////////////////////////////////////
void PVMFClientServerSocketPort::FormatUpdated()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "PVMFClientServerSocketPort::FormatUpdated %s", iFormat.getMIMEStrPtr()));
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFClientServerSocketPort::PeekIncomingMsg(PVMFSharedMediaMsgPtr& aMsg)
{
    if (iIncomingQueue.iQ.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFClientServerSocketPort::PeekIncomingMsg: Error - Incoming queue is empty", this));
        return PVMFFailure;
    }

    // Save message to output parameter and erase it from queue
    aMsg = iIncomingQueue.iQ.front();
    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFClientServerSocketPort::QueueOutgoingMsg(PVMFSharedMediaMsgPtr aMsg)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFClientServerSocketPort::QueueOutgoingMsg"));

    //If port is not connected, don't accept data on the
    //outgoing queue.
    if (!iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFClientServerSocketPort::QueueOutgoingMsg: Error - Port not connected"));
        return PVMFFailure;
    }

    PvmfPortBaseImpl* cpPort = OSCL_STATIC_CAST(PvmfPortBaseImpl*, iConnectedPort);

    // Connected Port incoming Queue is in busy / flushing state.  Do not accept more outgoing messages
    // until the queue is not busy, i.e. queue size drops below specified threshold or FlushComplete
    // is called.
    if (cpPort->iIncomingQueue.iBusy)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFClientServerSocketPort::QueueOutgoingMsg: Connected Port Incoming queue in busy / flushing state - Attempting to Q in output port's outgoing msg q"));
        return (PvmfPortBaseImpl::QueueOutgoingMsg(aMsg));
    }

    // Add message to outgoing queue and notify the node of the activity
    // There is no need to trap the push_back, since it cannot leave in this usage
    // Reason being that we do a reserve in the constructor and we do not let the
    // port queues grow indefinitely (we either a connected port busy or outgoing Q busy
    // before we reach the reserved limit
    PVMFStatus status = cpPort->Receive(aMsg);

    if (status != PVMFSuccess)
    {
        return PVMFFailure;
    }

    // Outgoing queue size is at capacity and goes into busy state. The owner node is
    // notified of this transition into busy state.
    if (cpPort->isIncomingFull())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFClientServerSocketPort::QueueOutgoingMsg: Connected Port incoming queue is full. Goes into busy state"));
        cpPort->iIncomingQueue.iBusy = true;
        PvmfPortBaseImpl::PortActivity(PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY);
    }

    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
bool PVMFClientServerSocketPort::IsOutgoingQueueBusy()
{
    if (iConnectedPort != NULL)
    {
        PvmfPortBaseImpl* cpPort = OSCL_STATIC_CAST(PvmfPortBaseImpl*, iConnectedPort);
        return (cpPort->iIncomingQueue.iBusy);
    }
    return (PvmfPortBaseImpl::IsOutgoingQueueBusy());;
}


////////////////////////////////////////////////////////////////////////////
void PVMFClientServerSocketPort::setParametersSync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements,
        PvmiKvp * & aRet_kvp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFClientServerSocketPort::setParametersSync: aSession=0x%x, aParameters=0x%x, num_elements=%d, aRet_kvp=0x%x",
                    aSession, aParameters, num_elements, aRet_kvp));
    OSCL_UNUSED_ARG(aSession);
    if (!aParameters || (num_elements != 1) ||
            (pv_mime_strcmp(aParameters->key, PVMF_CLIENTSERVER_SOCKET_PORT_SPECIFIC_ALLOCATOR_VALTYPE) != 0))
    {
        aRet_kvp = aParameters;
        OSCL_LEAVE(OsclErrArgument);
    }
    if (aParameters->value.key_specific_value == NULL)
    {
        aRet_kvp = aParameters;
        OSCL_LEAVE(OsclErrArgument);
    }
    else
    {
        aRet_kvp = NULL;
        iAllocSharedPtr =
            *((OsclSharedPtr<PVMFSharedSocketDataBufferAlloc>*)(aParameters->value.key_specific_value));
    }
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFClientServerSocketPort::getParametersSync(PvmiMIOSession aSession,
        PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters,
        int& num_parameter_elements,
        PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aSession);
    if (pv_mime_strcmp(aIdentifier, PVMI_PORT_CONFIG_INPLACE_DATA_PROCESSING_KEY) != 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFClientServerSocketPort::getParametersSync: Error - Unsupported PvmiKeyType"));
        return PVMFErrNotSupported;
    }

    num_parameter_elements = 0;
    if (!pvmiGetPortInPlaceDataProcessingInfoSync(PVMI_PORT_CONFIG_INPLACE_DATA_PROCESSING_VALUE, aParameters))
    {
        return PVMFFailure;
    }
    num_parameter_elements = 1;
    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFClientServerSocketPort::releaseParameters(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    if ((num_elements != 1) ||
            (pv_mime_strcmp(aParameters->key, PVMI_PORT_CONFIG_INPLACE_DATA_PROCESSING_VALUE) != 0))
    {
        return PVMFFailure;
    }
    OsclMemAllocator alloc;
    alloc.deallocate((OsclAny*)(aParameters));
    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
bool PVMFClientServerSocketPort::pvmiGetPortInPlaceDataProcessingInfoSync(const char*
        aFormatValType,
        PvmiKvp*& aKvp)
{
    //Create PvmiKvp for capability settings
    aKvp = NULL;
    OsclMemAllocator alloc;
    uint32 strLen = oscl_strlen(aFormatValType) + 1;
    uint8* ptr = (uint8*)alloc.allocate(sizeof(PvmiKvp) + strLen);

    if (!ptr)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFClientServerSocketPort::pvmiGetPortInPlaceDataProcessingInfoSync: Error - No memory. Cannot allocate PvmiKvp"));
        return false;
    }

    aKvp = new(ptr) PvmiKvp;
    ptr += sizeof(PvmiKvp);
    aKvp->key = (PvmiKeyType)ptr;
    oscl_strncpy(aKvp->key, aFormatValType, strLen);
    aKvp->length = aKvp->capacity = strLen;
    aKvp->value.bool_value = true;
    return true;
}
