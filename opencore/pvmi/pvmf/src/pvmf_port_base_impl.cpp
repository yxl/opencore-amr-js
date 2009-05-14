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
 * @file pvmf_port_base_impl.cpp
 * @brief Base implementation of pvmf port
 */

#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef OSCL_MEM_BASIC_FUNCTIONS_H
#include "oscl_mem_basic_functions.h"
#endif
#include "pvmf_media_msg_format_ids.h"
#include "pvmf_media_data.h"

#define LOGDATAPATH(x)	PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iDatapathLogger, PVLOGMSG_INFO, x);

void PvmfPortBaseImplQueue::Construct(uint32 aCap, uint32 aReserve, uint32 aThresh)
{
    iBusy = false;

    iCapacity = aCap;

    if (aReserve > 0)
        iQ.reserve(aReserve);

    if (aThresh > 100)
        aThresh = 100;
    iThresholdPercent = aThresh;
    iThreshold = iCapacity * iThresholdPercent / 100;
}

PVMFStatus PvmfPortBaseImplQueue::SetCapacity(uint32 aCap)
{
    iCapacity = aCap;
    iThreshold = iCapacity * iThresholdPercent / 100;

    return PVMFSuccess;
}

PVMFStatus PvmfPortBaseImplQueue::SetReserve(uint32 aReserve)
{
    if (aReserve > 0)
    {
        iQ.reserve(aReserve);
        return PVMFSuccess;
    }
    return PVMFFailure;
}

PVMFStatus PvmfPortBaseImplQueue::SetThreshold(uint32 aThresh)
{
    if (aThresh > 100)
        return PVMFFailure;

    iThresholdPercent = aThresh;
    iThreshold = iCapacity * iThresholdPercent / 100;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF int32 PvmfPortBaseImpl::GetPortTag() const
{
    return iTag;
}

OSCL_EXPORT_REF void PvmfPortBaseImpl::PortActivity(PVMFPortActivityType aActivity)
{
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
    if (iDatapathLogger)
    {
        switch (aActivity)
        {
            case PVMF_PORT_ACTIVITY_CREATED:
                LOGDATAPATH((0, "PORT %s Created", iPortName.get_cstr()));
                break;
            case PVMF_PORT_ACTIVITY_DELETED:
                LOGDATAPATH((0, "PORT %s Deleted", iPortName.get_cstr()));
                break;
            case PVMF_PORT_ACTIVITY_CONNECT:
                LOGDATAPATH((0, "PORT %s Connected", iPortName.get_cstr()));
                break;
            case PVMF_PORT_ACTIVITY_DISCONNECT:
                LOGDATAPATH((0, "PORT %s Disconnected", iPortName.get_cstr()));
                break;
            case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
                //msg was just added to end of in Q.
                LogMediaMsgInfo(iOutgoingQueue.iQ.back(), "Out Msg Q'd", iOutgoingQueue);
                break;
            case PVMF_PORT_ACTIVITY_INCOMING_MSG:
                //msg was just added to end of in Q.
                LogMediaMsgInfo(iIncomingQueue.iQ.back(), "In Msg Received ", iIncomingQueue);
                break;
            case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_BUSY:
                LOGDATAPATH(
                    (0, "PORT %s Outgoing Q Busy, Q-depth %d/%d", iPortName.get_cstr()
                     , iOutgoingQueue.iQ.size()
                     , iOutgoingQueue.iCapacity));
                break;
            case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
                LOGDATAPATH((0, "PORT %s Outgoing Q Ready, Q-depth %d/%d", iPortName.get_cstr()
                             , iOutgoingQueue.iQ.size()
                             , iOutgoingQueue.iCapacity));
                break;
            case PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY:
                LOGDATAPATH((0, "PORT %s Connected Port Busy, Q-depth %d/%d", iPortName.get_cstr()
                             , iOutgoingQueue.iQ.size()
                             , iOutgoingQueue.iCapacity));
                break;
            case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
                LOGDATAPATH((0, "PORT %s Connected Port Ready, Q-depth %d/%d", iPortName.get_cstr()
                             , iOutgoingQueue.iQ.size()
                             , iOutgoingQueue.iCapacity));
                break;
            case PVMF_PORT_ACTIVITY_ERROR:
            default:
                break;
        }
    }
#endif

    //report port activity to the activity handler
    if (iPortActivityHandler)
    {
        PVMFPortActivity activity(this, aActivity);
        iPortActivityHandler->HandlePortActivity(activity);
    }
}

OSCL_EXPORT_REF void PvmfPortBaseImpl::SetName(const char*name)
{
    if (name
            && name[0] != '\0')
    {
        iPortName = name;
        iDatapathLogger = PVLogger::GetLoggerObject("datapath");
    }
    else
    {
        iDatapathLogger = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PvmfPortBaseImpl::PvmfPortBaseImpl(int32 aTag, PVMFPortActivityHandler* aNode, const char*name)
        : PVMFPortInterface(aNode),
        iConnectedPortBusy(false),
        iInputSuspended(false),
        iTag(aTag)
{
    iLogger = PVLogger::GetLoggerObject("PvmfPortBaseImpl");
    iIncomingQueue.Construct(DEFAULT_DATA_QUEUE_CAPACITY, DEFAULT_DATA_QUEUE_CAPACITY, DEFAULT_READY_TO_RECEIVE_THRESHOLD_PERCENT);
    iOutgoingQueue.Construct(DEFAULT_DATA_QUEUE_CAPACITY, DEFAULT_DATA_QUEUE_CAPACITY, DEFAULT_READY_TO_RECEIVE_THRESHOLD_PERCENT);
    SetName(name);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PvmfPortBaseImpl::PvmfPortBaseImpl(int32 aTag
        , PVMFPortActivityHandler* aNode
        , uint32 aInCapacity
        , uint32 aInReserve
        , uint32 aInThreshold
        , uint32 aOutCapacity
        , uint32 aOutReserve
        , uint32 aOutThreshold
        , const char*name)
        : PVMFPortInterface(aNode),
        iConnectedPortBusy(false),
        iInputSuspended(false),
        iTag(aTag)
{
    iLogger = PVLogger::GetLoggerObject("PvmfPortBaseImpl");
    iIncomingQueue.Construct(aInCapacity, aInReserve, aInThreshold);
    iOutgoingQueue.Construct(aOutCapacity, aOutReserve, aOutThreshold);
    SetName(name);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmfPortBaseImpl::Construct()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::Construct: iTag=%d", this, iTag));

#if PVMF_PORT_BASE_IMPL_STATS
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
#endif
    PortActivity(PVMF_PORT_ACTIVITY_CREATED);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PvmfPortBaseImpl::~PvmfPortBaseImpl()
{
    if (iConnectedPort)
    {
        Disconnect();
    }
    while (!iIncomingQueue.iQ.empty())
    {
        PVMFSharedMediaMsgPtr msg = iIncomingQueue.iQ.front();
        iIncomingQueue.iQ.pop();
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
        if (iDatapathLogger)
            LogMediaMsgInfo(msg, "In Msg Cleared", iIncomingQueue);
#endif
    }
    while (!iOutgoingQueue.iQ.empty())
    {
        PVMFSharedMediaMsgPtr msg = iOutgoingQueue.iQ.front();
        iOutgoingQueue.iQ.pop();
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
        if (iDatapathLogger)
            LogMediaMsgInfo(msg, "Out Msg Cleared", iOutgoingQueue);
#endif
    }
    PortActivity(PVMF_PORT_ACTIVITY_DELETED);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::Connect(PVMFPortInterface* aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::Connect: aPort=0x%x", this, aPort));

    if (!aPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PvmfPortBaseImpl::Connect: Error - Connecting to invalid port", this));
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PvmfPortBaseImpl::Connect: Error - Already connected", this));
        return PVMFFailure;
    }

    //Automatically connect the peer.
    if (aPort->PeerConnect(this) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PvmfPortBaseImpl::Connect: Error - Peer Connect failed", this));
        return PVMFFailure;
    }

    iConnectedPort = aPort;

#if PVMF_PORT_BASE_IMPL_STATS
    // Reset statistics
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
#endif

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::PeerConnect(PVMFPortInterface* aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::PeerConnect: aPort=0x%x", this, aPort));

    if (!aPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PvmfPortBaseImpl::PeerConnect: Error - Connecting to invalid port", this));
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PvmfPortBaseImpl::PeerConnect: Error - Already connected", this));
        return PVMFFailure;
    }

    iConnectedPort = aPort;

#if PVMF_PORT_BASE_IMPL_STATS
    // Reset statistics
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
#endif

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::Disconnect()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::Disconnect", this));

    if (!iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "0x%x PvmfPortBaseImpl::Disconnect: Port already disconnected", this));
        return PVMFFailure;
    }

    //reset busy flags - this would prevent any queue / port ready
    //events from being generated as we clear the message queues
    iIncomingQueue.iBusy = false;
    iOutgoingQueue.iBusy = false;

    //Automatically disconnect the peer.
    iConnectedPort->PeerDisconnect();

    iConnectedPort = NULL;

    ClearMsgQueues();

#if PVMF_PORT_BASE_IMPL_STATS
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "0x%x PvmfPortBaseImpl::Disonnect: Stats: OutMsgQd=%d, OutMsgSent=%d, OutBusy=%d, ConnectedPortBusy=%d",
                     this, iStats.iOutgoingMsgQueued, iStats.iOutgoingMsgSent, iStats.iOutgoingQueueBusy,
                     iStats.iConnectedPortBusy));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "0x%x PvmfPortBaseImpl::Disonnect: Stats: InMsgRecv=%d, InMsgConsumed=%d, InBusy=%d",
                     this, iStats.iIncomingMsgRecv, iStats.iIncomingMsgConsumed, iStats.iIncomingQueueBusy));
#endif

    PortActivity(PVMF_PORT_ACTIVITY_DISCONNECT);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::PeerDisconnect()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::PeerDisconnect", this));

    if (!iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PvmfPortBaseImpl::PeerDisconnect: Error - Port not connected", this));
        return PVMFFailure;
    }

    //reset busy flags - this would prevent any queue / port ready
    //events from being generated as we clear the message queues
    iIncomingQueue.iBusy = false;
    iOutgoingQueue.iBusy = false;

    ClearMsgQueues();

    iConnectedPort = NULL;

#if PVMF_PORT_BASE_IMPL_STATS
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "0x%x PvmfPortBaseImpl::PeerDisconnect: Stats: OutMsgQd=%d, OutMsgSent=%d, OutBusy=%d, ConnectedPortBusy=%d",
                     this, iStats.iOutgoingMsgQueued, iStats.iOutgoingMsgSent, iStats.iOutgoingQueueBusy,
                     iStats.iConnectedPortBusy));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "0x%x PvmfPortBaseImpl::PeerDisconnect: Stats: InMsgRecv=%d, InMsgConsumed=%d, InBusy=%d",
                     this, iStats.iIncomingMsgRecv, iStats.iIncomingMsgConsumed, iStats.iIncomingQueueBusy));
#endif

    PortActivity(PVMF_PORT_ACTIVITY_DISCONNECT);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::QueueOutgoingMsg(PVMFSharedMediaMsgPtr aMsg)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::QueueOutgoingMsg", this));

    //If port is not connected, don't accept data on the
    //outgoing queue.
    if (!iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                        (0, "0x%x PvmfPortBaseImpl::QueueOutgoingMsg: Error - Port not connected", this));
        return PVMFFailure;
    }

    // Queue is in busy / flushing state.  Do not accept more outgoing messages until the queue
    // is not busy, i.e. queue size drops below specified threshold or FlushComplete is called.
    if (iOutgoingQueue.iBusy)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                        (0, "0x%x PvmfPortBaseImpl::QueueOutgoingMsg: Outgoing queue in busy / flushing state", this));
        return PVMFErrBusy;
    }

    // Add message to outgoing queue and notify the node of the activity
    // There is no need to trap the push_back, since it cannot leave in this usage
    // Reason being that we do a reserve in the constructor and we do not let the
    // port queues grow indefinitely (we either a connected port busy or outgoing Q busy
    // before we reach the reserved limit
    iOutgoingQueue.iQ.push(aMsg);

#if PVMF_PORT_BASE_IMPL_STATS
    ++iStats.iOutgoingMsgQueued;
#endif

    //Attempt to queue the message directly in connected port's incoming msg queue
    //first. If we cannot queue then we leave the msg in iOutgoingQueue
    //Doing the push in iOutgoingQueue first followed by Receive ensure that msgs
    //flow in FIFO order. If we did a Receive first then we would need
    PVMFStatus status = iConnectedPort->Receive(iOutgoingQueue.iQ.front());
    if (status == PVMFSuccess)
    {
        // Dequeue the message
        PVMFSharedMediaMsgPtr msg = iOutgoingQueue.iQ.front();
        iOutgoingQueue.iQ.pop();
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
        //log to datapath
        if (iDatapathLogger)
        {
            LogMediaMsgInfo(msg, "Msg Sent Directly", iOutgoingQueue);
        }
#endif
#if PVMF_PORT_BASE_IMPL_STATS
        // Count this message as either sent successfully,
        ++iStats.iOutgoingMsgSent;
#endif
        return status;
        //there is no need to queue port activity PVMF_PORT_ACTIVITY_OUTGOING_MSG
        //here since we have successfully q'd the msg on connected port's incoming
        //msg queue
    }
    else
    {
        PortActivity(PVMF_PORT_ACTIVITY_OUTGOING_MSG);
        // Outgoing queue size is at capacity and goes into busy state. The owner node is
        // notified of this transition into busy state.
        if (isOutgoingFull())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                            (0, "0x%x PvmfPortBaseImpl::QueueOutgoingMsg: Outgoing queue is full. Goes into busy state.", this));
            iOutgoingQueue.iBusy = true;
            PortActivity(PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_BUSY);
#if PVMF_PORT_BASE_IMPL_STATS
            ++iStats.iOutgoingQueueBusy;
#endif
        }
    }
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::Send()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::Send", this));

    if (!iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PvmfPortBaseImpl::Send: Error - Port not connected", this));
        return PVMFFailure;
    }

    if (iOutgoingQueue.iQ.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PvmfPortBaseImpl::Send: Error - No queued message on outgoing queue", this));
        return PVMFFailure;
    }

    if (iConnectedPortBusy)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                        (0, "0x%x PvmfPortBaseImpl::Send: Connected port is in busy state", this));
        return PVMFErrBusy;
    }

    PVMFStatus status = iConnectedPort->Receive(iOutgoingQueue.iQ.front());

    //Detect connected port busy...
    if (status == PVMFErrBusy)
    {
        //don't dequeue the message-- save it for
        //later.
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                        (0, "0x%x PvmfPortBaseImpl::Send: Receive failed. Connected port is in busy state", this));
        iConnectedPortBusy = true;
        PortActivity(PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY);
#if PVMF_PORT_BASE_IMPL_STATS
        ++iStats.iConnectedPortBusy;
#endif
        return status;
    }

    // Dequeue the message
    {
        PVMFSharedMediaMsgPtr msg = iOutgoingQueue.iQ.front();
        iOutgoingQueue.iQ.pop();
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
        //log to datapath
        if (iDatapathLogger)
        {
            switch (status)
            {
                case PVMFSuccess:
                    LogMediaMsgInfo(msg, "Msg Sent", iOutgoingQueue);
                    break;
                default:
                    LogMediaMsgInfo(msg, "Send Failed-- Msg Dropped!", iOutgoingQueue);
                    break;
            }
        }
#endif
    }

#if PVMF_PORT_BASE_IMPL_STATS
    // Count this message as either sent successfully,
    // or discarded.
    if (status == PVMFSuccess)
        ++iStats.iOutgoingMsgSent;
    else
        ++iStats.iOutgoingMsgDiscarded;
#endif

    // Notify the node if the queue was busy and the queue size has just dropped
    // below the threshold. The node can resume calling QueueOutgoingMsg.
    if (iOutgoingQueue.iBusy)
        EvaluateOutgoingBusy();

    return status;
}

OSCL_EXPORT_REF void PvmfPortBaseImpl::EvaluateOutgoingBusy()
{
    // Notify the node if the queue was busy and the queue size has just dropped
    // below the threshold. The node can resume calling QueueOutgoingMsg.
    if (iOutgoingQueue.iBusy && !isOutgoingFull())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                        (0, "0x%x PvmfPortBaseImpl::Send: Outgoing queue ready", this));
        iOutgoingQueue.iBusy = false;
        PortActivity(PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY);
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PvmfPortBaseImpl::isOutgoingFull()
//derived class can override this to redefine "queue full" condition.
{
    if (iOutgoingQueue.iThreshold == 0)
    {
        //should never happen
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PvmfPortBaseImpl::isOutgoingFull: Zero Threshold", this));
        OSCL_ASSERT(false);
        return true;
    }
    if (iOutgoingQueue.iQ.size() == iOutgoingQueue.iCapacity)
    {
        return true;
    }
    if (iOutgoingQueue.iBusy == true)
    {
        //this implies that we were at capacity previously
        //wait for occupancy to fall below threshold before
        //treating the queue as "not-full"
        if (iOutgoingQueue.iQ.size() >= iOutgoingQueue.iThreshold)
        {
            return true;
        }
    }
    //this means that we are below capacity and are approaching
    //capacity, or that we have fallen below threshold. in either
    //case treat queue as "not-full"
    return false;
}



void PvmfPortBaseImpl::LogMediaMsgInfo(PVMFSharedMediaMsgPtr aMediaMsg, const char* msg, PvmfPortBaseImplQueue&q)
//log media msg info, description, and associated q-depth.
{
    // to avoid compiler warnings when logger is not available
    OSCL_UNUSED_ARG(msg);
    OSCL_UNUSED_ARG(q);

    switch (aMediaMsg->getFormatID())
    {
        case PVMF_MEDIA_CMD_BOS_FORMAT_ID:
        {
            LOGDATAPATH(
                (0, "PORT %s %s MediaCmd FmtId %s, SeqNum %d, SId %d, TS %d, Q-depth %d/%d", iPortName.get_cstr()
                 , msg
                 , "BOS"
                 , aMediaMsg->getSeqNum()
                 , aMediaMsg->getStreamID()
                 , aMediaMsg->getTimestamp()
                 , q.iQ.size()
                 , q.iCapacity
                ));
        }
        break;
        case PVMF_MEDIA_CMD_EOS_FORMAT_ID:
        {
            LOGDATAPATH(
                (0, "PORT %s %s MediaCmd FmtId %s, SeqNum %d, SId %d, TS %d, Q-depth %d/%d", iPortName.get_cstr()
                 , msg
                 , "EOS"
                 , aMediaMsg->getSeqNum()
                 , aMediaMsg->getStreamID()
                 , aMediaMsg->getTimestamp()
                 , q.iQ.size()
                 , q.iCapacity
                ));
        }
        break;
        case PVMF_MEDIA_MSG_DATA_FORMAT_ID:
        {
            PVMFSharedMediaDataPtr mediaData;
            convertToPVMFMediaData(mediaData, aMediaMsg);
            LOGDATAPATH(
                (0, "PORT %s %s MediaData SeqNum %d, SId %d, TS %d, Q-depth %d/%d", iPortName.get_cstr()
                 , msg
                 , mediaData->getSeqNum()
                 , mediaData->getStreamID()
                 , mediaData->getTimestamp()
                 , q.iQ.size()
                 , q.iCapacity
                ));
        }
        break;
        default:
        {
            LOGDATAPATH(
                (0, "PORT %s %s MediaCmd FmtId %d, SeqNum %d, SId %d, TS %d, Q-depth %d/%d", iPortName.get_cstr()
                 , msg
                 , aMediaMsg->getFormatID()
                 , aMediaMsg->getSeqNum()
                 , aMediaMsg->getStreamID()
                 , aMediaMsg->getTimestamp()
                 , q.iQ.size()
                 , q.iCapacity
                ));
        }
        break;
    }
}

OSCL_EXPORT_REF void PvmfPortBaseImpl::LogMediaMsgInfo(PVMFSharedMediaMsgPtr aMediaMsg, const char* msg, int32 qsize)
//log media msg info, description, and associated q-depth.
{
    OSCL_UNUSED_ARG(msg);
    OSCL_UNUSED_ARG(qsize);

    if (!iDatapathLogger)
        return;

    switch (aMediaMsg->getFormatID())
    {
        case PVMF_MEDIA_CMD_EOS_FORMAT_ID:
        {
            LOGDATAPATH(
                (0, "PORT %s %s MediaCmd FmtId %s, SeqNum %d, SId %d, TS %d, Q-depth %d/%d", iPortName.get_cstr()
                 , msg
                 , "EOS"
                 , aMediaMsg->getSeqNum()
                 , aMediaMsg->getStreamID()
                 , aMediaMsg->getTimestamp()
                 , qsize
                ));
        }
        break;
        case PVMF_MEDIA_MSG_DATA_FORMAT_ID:
        {
            PVMFSharedMediaDataPtr mediaData;
            convertToPVMFMediaData(mediaData, aMediaMsg);
            LOGDATAPATH(
                (0, "PORT %s %s MediaData SeqNum %d, SId %d, TS %d, Q-depth %d/%d", iPortName.get_cstr()
                 , msg
                 , mediaData->getSeqNum()
                 , mediaData->getStreamID()
                 , mediaData->getTimestamp()
                 , qsize
                ));
        }
        break;
        default:
        {
            LOGDATAPATH(
                (0, "PORT %s %s MediaCmd FmtId %d, SeqNum %d, SId %d, TS %d, Q-depth %d/%d", iPortName.get_cstr()
                 , msg
                 , aMediaMsg->getFormatID()
                 , aMediaMsg->getSeqNum()
                 , aMediaMsg->getStreamID()
                 , aMediaMsg->getTimestamp()
                 , qsize
                ));
        }
        break;
    }
}

OSCL_EXPORT_REF void PvmfPortBaseImpl::LogMediaDataInfo(PVMFSharedMediaDataPtr aMediaData, const char* msg, int32 qsize)
//log media data info, description, and associated q-depth.
{
    OSCL_UNUSED_ARG(aMediaData);
    OSCL_UNUSED_ARG(msg);
    OSCL_UNUSED_ARG(qsize);

    if (!iDatapathLogger)
        return;

    LOGDATAPATH(
        (0, "PORT %s %s MediaData SeqNum %d, SId %d, TS %d, Q-depth %d/%d", iPortName.get_cstr()
         , msg
         , aMediaData->getSeqNum()
         , aMediaData->getStreamID()
         , aMediaData->getTimestamp()
         , qsize
        ));

}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::Receive(PVMFSharedMediaMsgPtr aMsg)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::Receive", this));

    // Incoming queue is in flush state. Do not receive more messages.
    if (iInputSuspended)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PvmfPortBaseImpl::Receive: Incoming queue is flushing", this));
        return PVMFErrInvalidState;
    }

    // Incoming queue is in busy state. Do not receive more messages until the queue
    // is not busy, i.e. queue size drops below specified threshold.
    if (iIncomingQueue.iBusy)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                        (0, "0x%x PvmfPortBaseImpl::Receive: Incoming queue is busy", this));
        return PVMFErrBusy;
    }

    // Add message to queue and notify the node of the activity
    // There is no need to trap the push_back, since it cannot leave in this usage
    // Reason being that we do a reserve in the constructor and we do not let the
    // port queues grow indefinitely (we either a connected port busy or outgoing Q busy
    // before we reach the reserved limit
    iIncomingQueue.iQ.push(aMsg);

    PortActivity(PVMF_PORT_ACTIVITY_INCOMING_MSG);
#if PVMF_PORT_BASE_IMPL_STATS
    ++iStats.iIncomingMsgRecv;
#endif

    // Incoming queue size is at capacity and goes into busy state.
    if (iIncomingQueue.iThreshold != 0 &&
            iIncomingQueue.iQ.size() >= iIncomingQueue.iCapacity)
    {
        iIncomingQueue.iBusy = true;
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
        //log transitions to Full state
        if (iDatapathLogger)
        {
            LOGDATAPATH(
                (0, "PORT %s Incoming Q Busy, Q-depth %d/%d", iPortName.get_cstr()
                 , iIncomingQueue.iQ.size()
                 , iIncomingQueue.iCapacity));
        }
#endif

#if PVMF_PORT_BASE_IMPL_STATS
        ++iStats.iIncomingQueueBusy;
#endif
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                        (0, "0x%x PvmfPortBaseImpl::Receive: Incoming queue is full. Goes into busy state.", this));
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::DequeueIncomingMsg(PVMFSharedMediaMsgPtr& aMsg)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::DequeueIncomingMsg", this));

    if (iIncomingQueue.iQ.empty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PvmfPortBaseImpl::DequeueIncomingMsg: Error - Incoming queue is empty", this));
        return PVMFFailure;
    }

    // Save message to output parameter and remove it from queue
    aMsg = iIncomingQueue.iQ.front();
    iIncomingQueue.iQ.pop();

#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
    if (iDatapathLogger)
        LogMediaMsgInfo(aMsg, "In Msg De-Q'd", iIncomingQueue);
#endif

#if PVMF_PORT_BASE_IMPL_STATS
    ++iStats.iIncomingMsgConsumed;
#endif

    // Notify the sender port if the queue was busy and the queue size has just dropped
    // below the threshold. Sender port can resume calling Receive.
    if (iIncomingQueue.iBusy)
        EvaluateIncomingBusy();

    return PVMFSuccess;
}

OSCL_EXPORT_REF void PvmfPortBaseImpl::EvaluateIncomingBusy()
{
    // Notify the sender port if the queue was busy and the queue size has just dropped
    // below the threshold. Sender port can resume calling Receive.
    if (iIncomingQueue.iBusy && !isIncomingFull())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                        (0, "0x%x PvmfPortBaseImpl::DequeueIncomingMsg: Ready to receive more messages", this));
        iIncomingQueue.iBusy = false;
        if (iConnectedPort)
            iConnectedPort->ReadyToReceive();
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PvmfPortBaseImpl::isIncomingFull()
//derived class can override this to redefine "queue full" condition.
{
    if (iIncomingQueue.iThreshold == 0)
    {
        //should never happen
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PvmfPortBaseImpl::isIncomingFull: Zero Threshold", this));
        OSCL_ASSERT(false);
        return true;
    }
    if (iIncomingQueue.iQ.size() == iIncomingQueue.iCapacity)
    {
        return true;
    }
    if (iIncomingQueue.iBusy == true)
    {
        //this implies that we were at capacity previously
        //wait for occupancy to fall below threshold before
        //treating the queue as "not-full"
        if (iIncomingQueue.iQ.size() >= iIncomingQueue.iThreshold)
        {
            return true;
        }
    }
    //this means that we are below capacity and are approaching
    //capacity, or that we have fallen below threshold. in either
    //case treat queue as "not-full"
    return false;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::ReadyToReceive()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::ReadyToReceive", this));

    // Notify the node to wake up and resume processing the outgoing queue of this port
    iConnectedPortBusy = false;
    PortActivity(PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::ClearMsgQueues()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::ClearMsgQueues", this));

    while (!iIncomingQueue.iQ.empty())
    {
        PVMFSharedMediaMsgPtr msg = iIncomingQueue.iQ.front();
        iIncomingQueue.iQ.pop();
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
        if (iDatapathLogger)
            LogMediaMsgInfo(msg, "In Msg Cleared", iIncomingQueue);
#endif
    }
    if (iIncomingQueue.iBusy)
        EvaluateIncomingBusy();

    while (!iOutgoingQueue.iQ.empty())
    {
        PVMFSharedMediaMsgPtr msg = iOutgoingQueue.iQ.front();
        iOutgoingQueue.iQ.pop();
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
        if (iDatapathLogger)
            LogMediaMsgInfo(msg, "Out Msg Cleared", iOutgoingQueue);
#endif
    }
    if (iOutgoingQueue.iBusy)
        EvaluateOutgoingBusy();
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 PvmfPortBaseImpl::IncomingMsgQueueSize()
{
    return iIncomingQueue.iQ.size();
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 PvmfPortBaseImpl::OutgoingMsgQueueSize()
{
    return iOutgoingQueue.iQ.size();
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PvmfPortBaseImpl::IsOutgoingQueueBusy()
{
    return iOutgoingQueue.iBusy;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PvmfPortBaseImpl::IsConnectedPortBusy()
{
    return iConnectedPortBusy;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmfPortBaseImpl::SuspendInput()
{
    iInputSuspended = true;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmfPortBaseImpl::ResumeInput()
{
    iInputSuspended = false;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 PvmfPortBaseImpl::GetCapacity(TPvmfPortBaseImplQueueType aType)
{
    if (aType == EPVIncomingDataQueue)
        return iIncomingQueue.iCapacity;
    return iOutgoingQueue.iCapacity;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 PvmfPortBaseImpl::GetThreshold(TPvmfPortBaseImplQueueType aType)
{
    if (aType == EPVIncomingDataQueue)
        return iIncomingQueue.iThreshold;
    return iOutgoingQueue.iThreshold;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::SetCapacity(TPvmfPortBaseImplQueueType aType, uint32 aCapacity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::SetCapacity: aCapacity=%d", this, aCapacity));

    if (aType == EPVIncomingDataQueue)
    {
        PVMFStatus status = iIncomingQueue.SetCapacity(aCapacity);
        if (status == PVMFSuccess)
            EvaluateIncomingBusy();
        return status;
    }
    else
    {
        PVMFStatus status = iOutgoingQueue.SetCapacity(aCapacity);
        if (status == PVMFSuccess)
            EvaluateOutgoingBusy();
        return status;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::SetReserve(TPvmfPortBaseImplQueueType aType, uint32 aCapacity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::SetCapacity: aCapacity=%d", this, aCapacity));

    if (aType == EPVIncomingDataQueue)
    {
        PVMFStatus status = iIncomingQueue.SetReserve(aCapacity);
        if (status == PVMFSuccess)
            EvaluateIncomingBusy();
        return status;
    }
    else
    {
        PVMFStatus status = iOutgoingQueue.SetReserve(aCapacity);
        if (status == PVMFSuccess)
            EvaluateOutgoingBusy();
        return status;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::SetThreshold(TPvmfPortBaseImplQueueType aType, uint32 aThreshold)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PvmfPortBaseImpl::SetThreshold: aThreshold=%d", this, aThreshold));
    if (aType == EPVIncomingDataQueue)
    {
        PVMFStatus status = iIncomingQueue.SetThreshold(aThreshold);
        if (status == PVMFSuccess)
            EvaluateIncomingBusy();
        return status;
    }
    else
    {
        PVMFStatus status = iOutgoingQueue.SetThreshold(aThreshold);
        if (status == PVMFSuccess)
            EvaluateOutgoingBusy();
        return status;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfPortBaseImpl::GetStats(PvmfPortBaseImplStats& aStats)
{
#if PVMF_PORT_BASE_IMPL_STATS
    oscl_memcpy(&aStats, &iStats, sizeof(PvmfPortBaseImplStats));
    return PVMFSuccess;
#else
    OSCL_UNUSED_ARG(aStats);
    return PVMFErrNotSupported;
#endif
}







