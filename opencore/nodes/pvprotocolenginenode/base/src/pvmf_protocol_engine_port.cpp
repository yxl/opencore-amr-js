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

#include "pvmf_protocol_engine_port.h"

#include "oscl_mem_basic_functions.h"

#include "pv_mime_string_utils.h"

#include "pvmf_node_interface.h"

#include "pvmf_protocol_engine_defs.h"



////////////////////////////////////////////////////////////////////////////
PVMFProtocolEnginePort::PVMFProtocolEnginePort(int32 aTag, PVMFNodeInterface* aNode, const char*name)
        : PvmfPortBaseImpl(aTag, aNode, name)
{
    Construct();
}

////////////////////////////////////////////////////////////////////////////
PVMFProtocolEnginePort::PVMFProtocolEnginePort(int32 aTag, PVMFNodeInterface* aNode,
        uint32 aInCapacity,
        uint32 aInReserve,
        uint32 aInThreshold,
        uint32 aOutCapacity,
        uint32 aOutReserve,
        uint32 aOutThreshold, const char*name) :
        PvmfPortBaseImpl(aTag, aNode, aInCapacity, aInReserve, aInThreshold, aOutCapacity, aOutReserve, aOutThreshold, name)
{
    Construct();
}

////////////////////////////////////////////////////////////////////////////
void PVMFProtocolEnginePort::Construct()
{
    iLogger = PVLogger::GetLoggerObject("PVMFProtocolEnginePort");
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
    iNumFramesGenerated = 0;
    iNumFramesConsumed = 0;
    PvmiCapabilityAndConfigPortFormatImpl::Construct(
        PVMF_PROTOCOLENGINE_PORT_INPUT_FORMATS,
        PVMF_PROTOCOLENGINE_PORT_INPUT_FORMATS_VALTYPE);
}

////////////////////////////////////////////////////////////////////////////
PVMFProtocolEnginePort::~PVMFProtocolEnginePort()
{
    Disconnect();
    ClearMsgQueues();
}

////////////////////////////////////////////////////////////////////////////
bool PVMFProtocolEnginePort::IsFormatSupported(PVMFFormatType aFmt)
{
    bool formatSupported = false;
    if (aFmt == PVMF_MIME_INET_TCP)
    {
        formatSupported = true;
    }
    return formatSupported;
}

////////////////////////////////////////////////////////////////////////////
void PVMFProtocolEnginePort::FormatUpdated()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "PVMFProtocolEnginePort::FormatUpdated() %s", iFormat.getMIMEStrPtr()));
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFProtocolEnginePort::QueueOutgoingMsg(PVMFSharedMediaMsgPtr aMsg)
{
    uint32 aOutgoingQueueSizeBefore = OutgoingMsgQueueSize();
    PVMFStatus status = PvmfPortBaseImpl::QueueOutgoingMsg(aMsg);
    if (status != PVMFSuccess) return status;
    uint32 aOutgoingQueueSizeAfter = OutgoingMsgQueueSize();
    if (aOutgoingQueueSizeBefore == aOutgoingQueueSizeAfter) return PVMFSuccessOutgoingMsgSent;
    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
bool PVMFProtocolEnginePort::PeekOutgoingMsg(PVMFSharedMediaMsgPtr& aMsg)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFProtocolEnginePort::PeekOutgoingMsg", this));

    if (iOutgoingQueue.iQ.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFProtocolEnginePort::PeekOutgoingMsg: Error - Incoming queue is empty", this));
        return false;
    }

    // Save message to output parameter and erase it from queue
    aMsg = iOutgoingQueue.iQ.front();
    return true;
}

