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

#include "oscl_mem_basic_functions.h"
#include "pv_mime_string_utils.h"
#include "pvmf_node_interface.h"


#ifndef PVMF_LOOPBACK_IOPORT_H_INCLUDED
#include "pvmf_loopback_ioport.h"
#endif

#ifndef PVMF_LOOPBACK_NODE_H_INCLUDED
#include "pvmf_loopback_node.h"
#endif

#include "pvlogger.h"
#include "oscl_tickcount.h"

////////////////////////////////////////////////////////////////////////////

PVMFLoopbackIOPort::PVMFLoopbackIOPort(int32 aTag, PVMFLoopbackNode* aNode)
        : PvmfPortBaseImpl(aTag, this)
        , OsclActiveObject(OsclActiveObject::EPriorityNominal, "PVMFLoopbackPort")
{
    Construct(aNode);
}

////////////////////////////////////////////////////////////////////////////
PVMFLoopbackIOPort::PVMFLoopbackIOPort(int32 aTag, PVMFLoopbackNode* aNode
                                       , uint32 aInCapacity
                                       , uint32 aInReserve
                                       , uint32 aInThreshold
                                       , uint32 aOutCapacity
                                       , uint32 aOutReserve
                                       , uint32 aOutThreshold)
        : PvmfPortBaseImpl(aTag, this, aInCapacity, aInReserve, aInThreshold, aOutCapacity, aOutReserve, aOutThreshold)
        , OsclActiveObject(OsclActiveObject::EPriorityNominal, "PVMFLoopbackPort")
{
    Construct(aNode);
}

////////////////////////////////////////////////////////////////////////////
void PVMFLoopbackIOPort::Construct(PVMFLoopbackNode*aNode)
{
    iNode = aNode;
    iWaiting = false;
    iLogger = PVLogger::GetLoggerObject("PVMFLoopbackPort");
    PvmiCapabilityAndConfigPortFormatImpl::Construct(
        PVMF_LOOPBACK_PORT_INPUT_FORMATS
        , PVMF_LOOPBACK_PORT_INPUT_FORMATS_VALTYPE);
    AddToScheduler();
}

////////////////////////////////////////////////////////////////////////////
PVMFLoopbackIOPort::~PVMFLoopbackIOPort()
{
    Disconnect();
    Reset();
    if (IsAdded())
        RemoveFromScheduler();
}

////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////

bool PVMFLoopbackIOPort::IsFormatSupported(PVMFFormatType aFmt)
{
    OSCL_UNUSED_ARG(aFmt);
    return true;
}

////////////////////////////////////////////////////////////////////////////
void PVMFLoopbackIOPort::FormatUpdated()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "PVMFLoopbackPort::FormatUpdated %s", iFormat.getMIMEStrPtr()));
}
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////

void PVMFLoopbackIOPort::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFLoopbackPort::PortActivity: port=0x%x, type=%d",
                     this, aActivity.iPort, aActivity.iType));

    //A port is reporting some activity or state change.  This code
    //figures out whether we need to queue a processing event
    //for the AO, and/or report a node event to the observer.

    switch (aActivity.iType)
    {
        case PVMF_PORT_ACTIVITY_CREATED:
            break;

        case PVMF_PORT_ACTIVITY_DELETED:
            break;

        case PVMF_PORT_ACTIVITY_CONNECT:
            //nothing needed.
            break;

        case PVMF_PORT_ACTIVITY_DISCONNECT:
            //nothing needed.
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            //An outgoing message was queued on this port.
            //Wakeup the AO on the first message only-- after
            //that it re-schedules itself as needed.
            if (aActivity.iPort->OutgoingMsgQueueSize() == 1)
                RunIfNotReady();
            break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
            //An incoming message was queued on this port.
            //Wakeup the AO on the first message only-- after
            //that it re-schedules itself as needed.
            if (aActivity.iPort->IncomingMsgQueueSize() == 1)
                RunIfNotReady();
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_BUSY:
            //Outgoing queue is now busy.
            //No action is needed here-- the node checks for
            //outgoing queue busy as needed during data processing.
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
            //the loopback port may be waiting on this event.

            if (iWaiting)
            {
                iWaiting = false;
                RunIfNotReady();
            }
            break;


        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY:
            // The connected port has become busy (its incoming queue is
            // busy).
            // No action is needed here-- the port processing code
            // checks for connected port busy during data processing.
            break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
            // The connected port has transitioned from Busy to Ready.
            if (OutgoingMsgQueueSize() > 0)
                RunIfNotReady();
            break;

        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
void PVMFLoopbackIOPort::Reset()
{
    Cancel();
    ClearMsgQueues();
    iWaiting = false;
}

////////////////////////////////////////////////////////////////////////////

void PVMFLoopbackIOPort::Run()
{
    //data should move through the ports only
    //when the node is active or flushing
    if (iNode->GetState() != EPVMFNodeStarted
            && !iNode->FlushPending())
        return;

    //Process incoming messages
    if (!iWaiting
            && IncomingMsgQueueSize() > 0)
    {
        //dispatch the incoming data.
        if (ProcessIncomingMsg() != PVMFSuccess)
            iNode->ReportErrorEvent(PVMF_NODE_ERROR_EVENT_LAST, (OsclAny*)this);

        //re-schedule if more data
        if (!iWaiting
                && IncomingMsgQueueSize() > 0)
        {
            RunIfNotReady();
        }
    }

    //Process outgoing messages
    if (OutgoingMsgQueueSize() > 0
            && !IsConnectedPortBusy())
    {
        //Send data to connected port
        PVMFStatus status = Send();
        if (status != PVMFSuccess)
            iNode->ReportErrorEvent(PVMF_NODE_ERROR_EVENT_LAST, (OsclAny*)this);

        //Reschedule if there's more data to process...
        if (OutgoingMsgQueueSize() > 0
                && !IsConnectedPortBusy())
        {
            RunIfNotReady();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

PVMFStatus PVMFLoopbackIOPort::ProcessIncomingMsg()
{
    if (IsOutgoingQueueBusy())
    {
        iWaiting = true;
        return PVMFSuccess;
    }

    //Move the incoming message from the input queue
    //to the output queue...
    PVMFSharedMediaMsgPtr msg;
    PVMFStatus status = DequeueIncomingMsg(msg);
    if (status != PVMFSuccess)
        return status;
    status = QueueOutgoingMsg(msg);
    if (status != PVMFSuccess)
        return status;

    return PVMFSuccess;

}
