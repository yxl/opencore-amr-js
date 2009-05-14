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
 * @file pvmf_port_interface.h
 */

#ifndef PVMF_PORT_INTERFACE_H_INCLUDED
#define PVMF_PORT_INTERFACE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_SHARED_PTR_H_INCLUDED
#include "oscl_shared_ptr.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif
#ifndef PVMF_MEDIA_MSG_H_INCLUDED
#include "pvmf_media_msg.h"
#endif

// Forward declaration
class PVMFNodeInterface;
class PVMFPortInterface;

/**
 * Enumerated list of port activity. This enumerated type is used to notify the owner
 * node of port activity and events.
 */
typedef enum
{
    PVMF_PORT_ACTIVITY_CREATED, /**< constructed*/
    PVMF_PORT_ACTIVITY_DELETED, /**< destroyed*/
    PVMF_PORT_ACTIVITY_CONNECT, /**< Connection with another port is established */
    PVMF_PORT_ACTIVITY_DISCONNECT, /**< Connection with another port is disconnected */
    PVMF_PORT_ACTIVITY_OUTGOING_MSG, /**< Outgoing message received. The node needs to call Send
	                              *  asynchronously to send this message to receiving port. */
    PVMF_PORT_ACTIVITY_INCOMING_MSG, /**< Incoming message received. The node needs to call DequeueIncomingMsg
	                              *  asynchronously to retrieve and handle the message. */
    PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_BUSY, /**< Outgoing queue becomes busy and cannot receive more outgoing messages.
	                                     *  The node should suspend calling QueueOutgoingMsg until receiving
	                                     *  PORT_ACTIVITY_OUTGOING_QUEUE_READY event. */
    PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY, /**< After the outgoing queue size reaches its capacity, the port
	                                      * would continue to process its outgoing queue, and when the size
	                                      * decreases to below the specified threshold, the owner node would
	                                      * be notified that it can resume queueing outgoing data to this port. */
    PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY, /**< Connected port is busy and cannot receive more incoming messages. The
	                                     *  node should suspend calling Send to process the outgoing messages until
                                         *  receiving PORT_ACTIVITY_CONNECTED_PORT_READY event. */
    PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY, /**< Connected port is ready to receive more incoming messages. The node
	                                     *  resume calling Send to process its outgoing queue. */
    PVMF_PORT_ACTIVITY_ERROR	/**< An error occurred in the port. */
} PVMFPortActivityType;

/** Structure containing the port activity and the port on which this activity occurred. */
class PVMFPortActivity
{
    public:
        PVMFPortActivity(PVMFPortInterface* aPort, PVMFPortActivityType aType) : iPort(aPort), iType(aType) {};
        PVMFPortActivity(const PVMFPortActivity& aActivity)
        {
            Copy(aActivity);
        }
        PVMFPortActivity& operator=(const PVMFPortActivity& aActivity)
        {
            Copy(aActivity);
            return (*this);
        }

        PVMFPortInterface* iPort; /**< Port on which activity occurred */
        PVMFPortActivityType iType; /**< Activity type. */

    private:
        void Copy(const PVMFPortActivity& aActivity)
        {
            iPort = aActivity.iPort;
            iType = aActivity.iType;
        }
};

class PVMFPortActivityHandler;
class PvmiCapabilityAndConfig;

/**
 * PVMF Port Interface is an input or output interface for communicating media, control,
 * and status data in or out of a graph node.  Through the port interface, media messages,
 * data, status and control may flow in both directions between two connected ports.
 */
class PVMFPortInterface
{
    public:
        virtual ~PVMFPortInterface() {}

        /**
         * Returns the tag of this port
         * @return Port tag
         */
        virtual int32 GetPortTag() const = 0;

        /**
         * Establish connection with another port.
         * @param aPort Port to connect with
         * @return Completion status
         */
        virtual PVMFStatus Connect(PVMFPortInterface* aPort) = 0;

        /**
         * Disconnect an established connection with another port
         * @return Completion status
         */
        virtual PVMFStatus Disconnect() = 0;

        /**
         * Queue an outgoing media message to the port.
         *
         * The outgoing message is added to the outgoing message queue, and the message
         * will be delivered to the receiving port asynchronously.
         *
         * @param aMsg Outgoing media message
         * @return Completion status. Returns PVMFFailure if the outgoing message queue is full
         */
        virtual PVMFStatus QueueOutgoingMsg(PVMFSharedMediaMsgPtr aMsg) = 0;

        /**
         * Send a queued outgoing message to the connected receiving port.
         *
         * @return Completion status. Returns PVMFFailure if the port is not connected
         * to another port, or the message is rejected by the receiving port.
         */
        virtual PVMFStatus Send() = 0;

        /**
         * Receives an incoming message.
         *
         * The incoming message is added to the incoming message queue, and the message
         * is processed by the node asynchronously.  If the incoming message queue is full,
         * this method would fail and the sending port should stop sending incoming messages
         * until it is notified by the ReadyToReceive call to resume sending messages.
         *
         * @param aMsg Incoming media message
         * @return Completion status.  Returns PVMFFailure if incoming mesasge queue is full.
         */
        virtual PVMFStatus Receive(PVMFSharedMediaMsgPtr aMsg) = 0;

        /**
         * Dequeue a message from the incoming queue and return the message.
         *
         * If the incoming message queue was full and the queue size has reduced to below
         * a reasonable level, the port should notify the sending port that it can resume
         * sending data by calling ReadyToReceive on the sending port.
         *
         * @param aMsg Outgoing parameter for the port to return the dequeued message
         * @return Completion status.
         */
        virtual PVMFStatus DequeueIncomingMsg(PVMFSharedMediaMsgPtr& aMsg) = 0;

        /**
         * Notification to sender port to resume sending outgoing message to receiving port.
         *
         * After the receiving port's incoming message queue size reached capacity, the port
         * would reject all incoming messages while continue to process queued messages.  When
         * the incoming queue size drops below a specified threshold, the receiving port will
         * call this method on the sender port to notify the sender that it can resume sending
         * incoming messages to the receiving port.
         *
         * @return Completion status.
         */
        virtual PVMFStatus ReadyToReceive() = 0;

        /**
         * Clear both incoming and outgoing queues.
         *
         * The queues messages in both queues will be discarded and not processed. The node will
         * need to clean its port activities queue such that it will not continue processing
         * queued port activities.
         *
         * @return Completion status.
         */
        virtual PVMFStatus ClearMsgQueues() = 0;

        /**
         * Query size of incoming message queue.
         * @return Size of incoming message queue.
         */
        virtual uint32 IncomingMsgQueueSize() = 0;

        /**
         * Query size of outgoing message queue.
         * @return Size of outgoing message queue.
         */
        virtual uint32 OutgoingMsgQueueSize() = 0;

        /**
         * Query whether the outgoing queue can accept more outgoing messages.
         * @return true if outgoing queue is busy, else false.
         */
        virtual bool IsOutgoingQueueBusy() = 0;

        /**
         * Query whether the connected port can receive more incoming messages.
         * @return true if connected port is busy, else false.
         */
        virtual bool IsConnectedPortBusy() = 0;

        /**
         * Methods to allow the node to suspend and resume
         * input queue operation.  When the input queue is
         * suspended, it will not accept data and will return
         * PVMFErrInvalidState.  Suspend & resume can be used
         * to implement the node flush operation.  Port input
         * should be suspended during a flush.
         */
        virtual void SuspendInput() = 0;
        virtual void ResumeInput() = 0;

        virtual bool IsConnected()
        {
            return iConnectedPort != NULL;
        }

        virtual void QueryInterface(const PVUuid &uuid, OsclAny* &ptr) = 0;

        /**
         * This method can be called during a Connect to set
         * the peer connection.
         */
        virtual PVMFStatus PeerConnect(PVMFPortInterface* aPort) = 0;

        /**
         * This method can be called during a Disconnect to disconnect
         * the peer connection.
         */
        virtual PVMFStatus PeerDisconnect() = 0;

    protected:
        PVMFPortInterface()
                : iConnectedPort(NULL)
                , iPortActivityHandler(NULL)
        {}
        PVMFPortInterface(PVMFPortActivityHandler* aNode)
                : iConnectedPort(NULL)
                , iPortActivityHandler(aNode)
        {}
        virtual void SetActivityHandler(PVMFPortActivityHandler* aNode)
        {
            iPortActivityHandler = aNode;
        }

        /**
        ** The port implementation uses this routine to report
        ** port activity to the activity handler (usually the
        ** node that contains the port).
        */
        virtual void PortActivity(PVMFPortActivityType aActivity) = 0;

        PVMFPortInterface* iConnectedPort;
        PVMFPortActivityHandler *iPortActivityHandler;
};

#endif
