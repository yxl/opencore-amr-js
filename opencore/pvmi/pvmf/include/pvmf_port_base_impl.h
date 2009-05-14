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
 * @file pvmf_port_base_impl.h
 */
#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#define PVMF_PORT_BASE_IMPL_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_QUEUE_H_INCLUDED
#include "oscl_queue.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_PORT_INTERFACE_H_INCLUDED
#include "pvmf_port_interface.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

/**
 * When this macro is defined to 1, statistics of this port will be tracked and can be
 * retrieved for debugging and performance analysis.
 */
#define PVMF_PORT_BASE_IMPL_STATS (PVLOGGER_INST_LEVEL >= PVLOGMSG_INST_MLDBG)

/** Structure containing statistics of a port */
struct PvmfPortBaseImplStats
{
    uint32 iOutgoingMsgQueued;		/**< Number of outgoing messages queued */
    uint32 iOutgoingMsgSent;		/**< Number of outgoing messages sent */
    uint32 iOutgoingMsgDiscarded;	/**< Number of outgoing messages discarded */
    uint32 iOutgoingQueueBusy;		/**< Number of times outgoing message queue became busy */
    uint32 iConnectedPortBusy;		/**< Number of times connected port became busy */
    uint32 iIncomingMsgRecv;		/**< Number of incoming messages received */
    uint32 iIncomingMsgConsumed;	/**< Number of incoming messages dequeued by the node */
    uint32 iIncomingQueueBusy;		/**< Number of times incoming message queue became busy */
};

/** Queue structure */
class PvmfPortBaseImplQueue
{
    public:
        void Construct(uint32 aCap, uint32 aReserve, uint32 aThresh);
        PVMFStatus SetCapacity(uint32);
        PVMFStatus SetReserve(uint32);
        PVMFStatus SetThreshold(uint32);

        Oscl_Queue<PVMFSharedMediaMsgPtr, OsclMemAllocator> iQ;

        // Flow control
        uint32 iCapacity;
        uint32 iThresholdPercent; /**< Threshold for allowing new messages, in terms of percentage of capacity. */
        uint32 iThreshold; /**< Threshold size for allowing new messages */
        bool iBusy;
};
/** Queue type */
enum TPvmfPortBaseImplQueueType
{
    EPVIncomingDataQueue
    , EPVOutgoingDataQueue
};

/** Default data queue capacity */
const uint32 DEFAULT_DATA_QUEUE_CAPACITY = 10;

/**
 * Threshold in terms of percentage of the total queue size at which a data queue
 * will be ready to resume receiving data. After the reaching the maximum queue size
 * the port would wait until enough queued messages are processed such that the queue
 * size drops below this threshold before notifying the sender port / node to resume
 * sending data.
 */
const uint32 DEFAULT_READY_TO_RECEIVE_THRESHOLD_PERCENT = 60;

/**
 * Base implementation of PVMFPortInterface. This implementation provides an incoming
 * and an outgoing queue for media messages, and depends on the PVMF Node that owns
 * this port on processing these messages asynchronously.  The node will be notified
 * of the events occurred to this port, and the node implementation will need to
 * handle these message as necessary in order for this base port implementation to
 * function.  This implementation does not perform capability exchange when two ports
 * are establishing a connection.  The final implementation of a port should derive
 * from this class and the PvmiConfigAndCapability interface for full functionality
 * with capability and data exchange.
 */
class PvmfPortBaseImpl : public PVMFPortInterface
{
    public:
        /**
         * Default constructor. Data queue size and ready to receive threshold will be set to
         * default values.
         *
         * @param aTag Port tag associated to this port
         * @param aNode The node that owns this port. Port events will be reported to this node.
         * @param name Optional port name.  If a name is provided, then datapath logging will
         *    be activated on the port.
         */
        OSCL_IMPORT_REF PvmfPortBaseImpl(int32 aPortTag, PVMFPortActivityHandler* aNode, const char*name = NULL);

        /**
         * Constructor specifying the data queue size and ready to receive threshold.
         *
         * @param aTag Port tag associated to this port
         * @param aNode The node that owns this port. Port events will be reported to this node.
         * @param aSize Data queue capacity. The data queue size will not grow beyond this capacity.
         * @param aReserve Size of data queue for which memory is reserved. This must be
         * less than or equal to the capacity. If this is less than capacity, memory will be
         * allocated when the queue grows beyond the reserve size, but will stop growing at
         * capacity.
         * @param aThreshold Ready-to-receive threshold, in terms of percentage of the data queue capacity.
         * This value should be between 0 - 100.
         * @param name Optional port name.  If a name is provided, then datapath logging will
         *    be activated on the port.
         */
        OSCL_IMPORT_REF PvmfPortBaseImpl(int32 aTag
                                         , PVMFPortActivityHandler* aNode
                                         , uint32 aInputCapacity
                                         , uint32 aInputReserve
                                         , uint32 aInputThreshold
                                         , uint32 aOutputCapacity
                                         , uint32 aOutputReserve
                                         , uint32 aOutputThreshold
                                         , const char*name = NULL);

        /** Destructor */
        OSCL_IMPORT_REF virtual ~PvmfPortBaseImpl();
        OSCL_IMPORT_REF virtual int32 GetPortTag() const;/*{ return iTag; }*/
        OSCL_IMPORT_REF virtual PVMFStatus Connect(PVMFPortInterface* aPort);
        OSCL_IMPORT_REF virtual PVMFStatus Disconnect();
        OSCL_IMPORT_REF virtual PVMFStatus PeerConnect(PVMFPortInterface* aPort);
        OSCL_IMPORT_REF virtual PVMFStatus PeerDisconnect() ;
        OSCL_IMPORT_REF virtual PVMFStatus QueueOutgoingMsg(PVMFSharedMediaMsgPtr aMsg);
        OSCL_IMPORT_REF virtual PVMFStatus Send();
        OSCL_IMPORT_REF virtual PVMFStatus Receive(PVMFSharedMediaMsgPtr aMsg);
        OSCL_IMPORT_REF virtual PVMFStatus DequeueIncomingMsg(PVMFSharedMediaMsgPtr& aMsg);
        OSCL_IMPORT_REF virtual PVMFStatus ReadyToReceive();
        OSCL_IMPORT_REF virtual PVMFStatus ClearMsgQueues();
        OSCL_IMPORT_REF virtual uint32 IncomingMsgQueueSize();
        OSCL_IMPORT_REF virtual uint32 OutgoingMsgQueueSize();
        OSCL_IMPORT_REF virtual bool IsOutgoingQueueBusy();
        OSCL_IMPORT_REF virtual bool IsConnectedPortBusy();
        OSCL_IMPORT_REF virtual void SuspendInput();
        OSCL_IMPORT_REF virtual void ResumeInput();

        /**
         * Return capacity of data queues
         * @return Capacity of data queues
         */
        OSCL_IMPORT_REF uint32 GetCapacity(TPvmfPortBaseImplQueueType);

        /**
         * Returns ready-to-receive threshold
         * @return Ready-to-receive threshold, in terms of percentage of the data queue capacity.
         */
        OSCL_IMPORT_REF uint32 GetThreshold(TPvmfPortBaseImplQueueType);

        /**
         * Update capacity of the data queues.
         *
         * @param aCapacity New capacity of the data queues.
         * @return Completion status.
         */
        OSCL_IMPORT_REF PVMFStatus SetCapacity(TPvmfPortBaseImplQueueType, uint32 aCapacity);
        /**
         * Update reserve of the data queues.
         *
         * @param aCapacity New capacity of the data queues.
         * @return Completion status.
         */
        OSCL_IMPORT_REF PVMFStatus SetReserve(TPvmfPortBaseImplQueueType, uint32 aCapacity);

        /**
         * Update ready-to-receive threshold, in terms of percentage of the data queue capacity.
         *
         * @param aThreshold Ready to receive threshold. This value should be between 0 and 100.
         * @return Completion status
         */
        OSCL_IMPORT_REF PVMFStatus SetThreshold(TPvmfPortBaseImplQueueType, uint32 aThreshold);

        /**
         * Return statistics of this port.
         * @param aStats Output parameter where port statistics will be repoted to.
         * @return Completion status.
         */
        OSCL_IMPORT_REF PVMFStatus GetStats(PvmfPortBaseImplStats& aStats);

        /**
         * Reports port activity to the activity handler (usually
         * the node).
         */
        OSCL_IMPORT_REF void PortActivity(PVMFPortActivityType aActivity);

        /**
         * Derived classes must override this if they
         * implement the capability and config interface
         */
        virtual void QueryInterface(const PVUuid&, OsclAny*&aPtr)
        {
            aPtr = NULL;
        }

        OSCL_IMPORT_REF void Construct();
        OSCL_IMPORT_REF virtual void EvaluateIncomingBusy();
        OSCL_IMPORT_REF virtual void EvaluateOutgoingBusy();
        OSCL_IMPORT_REF virtual bool isIncomingFull();
        OSCL_IMPORT_REF virtual bool isOutgoingFull();

        // Queues
        PvmfPortBaseImplQueue iIncomingQueue;
        PvmfPortBaseImplQueue iOutgoingQueue;

        // Flow control
        bool iConnectedPortBusy;
        bool iInputSuspended;

    protected:
        // Port tag
        int32 iTag;

        // Logging and statistics
        PVLogger* iLogger;
        PvmfPortBaseImplStats iStats;

        //For datapath logging.  If a port name is provided, either in
        //the constructor or by calling SetName, then datapath logging will
        //be activated on the port.
        OSCL_IMPORT_REF void SetName(const char*name);
        OSCL_StackString<20> iPortName;
        PVLogger* iDatapathLogger;
        char* PortName()
        {
            return iPortName.get_str();
        }
        void LogMediaMsgInfo(PVMFSharedMediaMsgPtr aMediaMsg, const char*, PvmfPortBaseImplQueue&);
    public:
        //a couple of functions for use by the nodes to log info about other internal
        //data queues using the port datapath logger.
        OSCL_IMPORT_REF void LogMediaMsgInfo(PVMFSharedMediaMsgPtr aMediaMsg, const char*, int32);
        OSCL_IMPORT_REF void LogMediaDataInfo(PVMFSharedMediaDataPtr aMediaData, const char*, int32);
};

#endif // PVMF_PORT_BASE_IMPL_H_INCLUDED
