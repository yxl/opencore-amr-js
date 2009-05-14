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
#ifndef PVMF_SOCKET_NODE_H_INCLUDED
#define PVMF_SOCKET_NODE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCLCONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef OSCL_SOCKET_TYPES_H_INCLUDED
#include "oscl_socket_types.h"
#endif
#ifndef OSCL_SOCKET_H_INCLUDED
#include "oscl_socket.h"
#endif
#ifndef OSCL_DNS_H_INCLUDED
#include "oscl_dns.h"
#endif
#include "oscl_tickcount.h"
#include "oscl_mem_mempool.h"

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif
#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMI_DATA_STREAM_INTERFACE_H_INCLUDED
#include "pvmi_data_stream_interface.h"
#endif

#include "pvmf_node_utils.h"
#include "pvmf_socket_port.h"

#ifndef PVMF_SOCKET_NODE_EXTENSION_INTERFACE_H_INCLUDED
#include "pvmf_socket_node_extension_interface.h"
#endif

#ifndef PVMF_SM_TUNABLES_H_INCLUDED
#include "pvmf_sm_tunables.h"
#endif
#ifndef PVMF_RESIZABLE_SIMPLE_MEDIAMSG_H_INCLUDED
#include "pvmf_resizable_simple_mediamsg.h"
#endif
#include "pvmf_socket_node_events.h"
#include "pvmf_media_frag_group.h"

//Enable socket node stats unless this is a release build.
#include "osclconfig.h"
#if(OSCL_RELEASE_BUILD)
#define ENABLE_SOCKET_NODE_STATS 0
#else
#define ENABLE_SOCKET_NODE_STATS 1
#endif

//Logger macros
#define PVMF_SOCKETNODE_LOGSTACKTRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_STACK_TRACE,m);
#define PVMF_SOCKETNODE_LOGINFO(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_INFO,m);
#define PVMF_SOCKETNODE_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define PVMF_SOCKETNODE_LOGWARNING(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_WARNING,m);
#define PVMF_SOCKETNODE_LOGDATATRAFFIC_I(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLogger,PVLOGMSG_INFO,m);
#define PVMF_SOCKETNODE_LOGDATATRAFFIC_E(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLogger,PVLOGMSG_ERR,m);
#define PVMF_SOCKETNODE_LOGDATATRAFFIC_RTP(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLoggerRTP,PVLOGMSG_INFO,m);
#define PVMF_SOCKETNODE_LOGDATATRAFFIC_RTCP(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLoggerRTCP,PVLOGMSG_INFO,m);

//memory allocator type for this node.
typedef OsclMemAllocator PVMFSocketNodeAllocator;

//Node command type.
typedef PVMFGenericNodeCommand<PVMFSocketNodeAllocator> PVMFSocketNodeCommandBase;
class PVMFSocketNodeCommand: public PVMFSocketNodeCommandBase
{
    public:

};

//Default vector reserve size
#define PVMF_SOCKET_NODE_COMMAND_VECTOR_RESERVE 10

//Starting value for command IDs
#define PVMF_SOCKET_NODE_COMMAND_ID_START 6000

class PVLogger;
class PVMFSocketPort;

/**
 * Mem pool class
 */
class PVMFSocketNodeMemPool
{
    public:
        PVMFSocketNodeMemPool(uint32 aMemPoolNumBufs = DEFAULT_NUM_MEDIA_MSGS_IN_JITTER_BUFFER);

        virtual ~PVMFSocketNodeMemPool()
        {
            if (iMediaDataMemPool != NULL)
            {
                iMediaDataMemPool->removeRef();
                iMediaDataMemPool = NULL;
            }
        };

        OsclSharedPtr<PVMFMediaDataImpl> getMediaDataImpl(uint32 size)
        {
            OsclSharedPtr<PVMFMediaDataImpl> mediaImpl = iMediaMsgAllocator->allocate(size);
            return mediaImpl;
        }

        void resizeSocketDataBuffer(OsclSharedPtr<PVMFMediaDataImpl>& aSharedBuffer)
        {
            if (iMediaMsgAllocator != NULL)
            {
                iMediaMsgAllocator->ResizeMemoryFragment(aSharedBuffer);
            }
        }

        void CreateAllocators(const OSCL_HeapString<OsclMemAllocator>& iMime, uint32 aSize, uint32 aExpectedNumberOfBlocksPerBuffer, uint32 aResizeSize, uint32 aMaxNumResizes);
        OsclMemPoolResizableAllocator* CreateResizableDataBufferAllocator(const char* allocatorName);
        void CreateDefaultDataBufferAllocator(const char* allocatorName);
        void CreateUDPMultipleRecvAllocator();

        uint32 GetMaxSizeMediaMsgLen();

        void DestroyAllocators();
        void DestroyUDPMultipleRecvAllocator();

        class SharedDataBufferInfo
        {
            public:
                SharedDataBufferInfo(): iSize(0), iResizeSize(0), iMaxNumResizes(0), iExpectedNumberOfBlocksPerBuffer(0) {}
                void Init(uint32 aSize, uint32 aExpectedNumberOfBlocksPerBuffer, uint32 aResizeSize, uint32 aMaxNumResizes)
                {
                    iSize = aSize;
                    iResizeSize = aResizeSize;
                    iMaxNumResizes = aMaxNumResizes;
                    iExpectedNumberOfBlocksPerBuffer = aExpectedNumberOfBlocksPerBuffer;
                }
                uint32 iSize;
                uint32 iResizeSize;
                uint32 iMaxNumResizes;
                uint32 iExpectedNumberOfBlocksPerBuffer;
        };
        SharedDataBufferInfo iSharedDataBufferInfo;
        // Memory pool for media data objects
        OsclMemPoolFixedChunkAllocator* iMediaDataMemPool;

        // Allocators for persisting the data received from the server (on all the ports) are created in respective SocketConfig.
        //For UDP [RTSP Based Streaming], allocators will be created before connection of port to its peer completes.
        //For TCP [MSHTTP Streaming], size of the memory pool for Data is determined only after receiving ASF header from the streaming server
        //therefore, to persist the response of the Server before completion of receiving the ASF header, an internal shared buffer allocator is created.
        PVMFResizableSimpleMediaMsgAlloc* iMediaMsgAllocator;
        OsclMemPoolResizableAllocator*	iSharedBufferAlloc;
        OsclMemPoolResizableAllocator*	iInternalAlloc;//[MSHTTP Streaming Specific specific]

        // Allocator for multiple receives[ for UDP only, will be created and used only when SNODE_ENABLE_UDP_MULTI_PACKET is defined].
        PVMFMediaFragGroupCombinedAlloc<PVMFSocketNodeAllocator>* iMediaFragGroupAlloc;
        OsclMemPoolFixedChunkAllocator* iMediaFragGroupAllocMempool;

        int32 iPortTag;
};

/*
** Socket address structure
*/

enum PROTOCOL
{
    INVALID_PROTOCOL,
    INET_TCP,
    INET_UDP
};

struct SOCKET_ADDR
{
    PROTOCOL iProtocol;

    OsclNetworkAddress iLocalAdd;
    OsclNetworkAddress iRemoteAdd;

    OSCL_HeapString<PVMFSocketNodeAllocator> iRemoteHost;	//could be either DNS or ip address

    //each socket maps to a port, hence a port tag is needed for unique identification
    int32 iTag;
};

class PVMFSocketNode;

/*
** Socket activity class is used to save Oscl socket or DNS results
*/
class PVMFSocketActivity
{
    public:
        PVMFSocketActivity(): iValid(false)
        {
        }
        PVMFSocketActivity(PVMFStatus aStatus, int32 aId, int32 aFxn, int32 aEvent, int32 aError)
        {
            Set(aStatus, aId, aFxn, aEvent, aError);
        }
        void Set(PVMFStatus aStatus, int32 aId, int32 aFxn, int32 aEvent, int32 aError)
        {
            iValid = true;
            iStatus = aStatus;
            iId = aId;
            iFxn = aFxn;
            iEvent = aEvent;
            iError = aError;
        }
        bool iValid;
        PVMFStatus iStatus;
        int32 iId;
        int32 iFxn;
        int32 iEvent;
        int32 iError;

    private:

};


/*
** The Socket port state keeps track of the Connect, Receive, and Send operations on
** a port, including wait states.  Note there may be simultaneous send & receive on
** a port, so it is necessary to keep separate status.  Connect operations cannot
** be concurrent with either send or receive operations, but for simplicity, the
** connect status is also maintained separately.
**
** The port state also contains information about sequences of operations.
*/

//Individual asynchronous operations and wait states that make up a connect or
//disconnect sequence.
enum TPVSocketPortConnectOperation
{
    EPVSocketPortConnectOperation_None
    , EPVSocketPortConnectOperation_GetHostByName
    , EPVSocketPortConnectOperation_Connect
    , EPVSocketPortConnectOperation_Shutdown
    , EPVSocketPortConnectOperation_WaitOnConnectedPort //for sending EOS during disconnect.
    , EPVSocketPortConnectOperation_WaitOnSendRecvIdle //for shutdown & cleanup.
    , EPVSocketPortConnectOperation_Last
};

//Individual asynchronous operations and wait states that make up a send sequence.
enum TPVSocketPortSendOperation
{
    EPVSocketPortSendOperation_None
    , EPVSocketPortSendOperation_Send
    , EPVSocketPortSendOperation_SendTo
    , EPVSocketPortSendOperation_Last
};

//Individual asynchronous operations and wait states that make up a receive sequence.
enum TPVSocketPortRecvOperation
{
    EPVSocketPortRecvOperation_None
    , EPVSocketPortRecvOperation_Recv
    , EPVSocketPortRecvOperation_RecvFrom
    , EPVSocketPortRecvOperation_WaitOnConnectedPort
    , EPVSocketPortRecvOperation_WaitOnMemory
    , EPVSocketPortRecvOperation_Last
};

//Sequences that may require multiple asynchronous operations
//and/or wait states.  There can only be one of these sequences active
//on a port at a time. For definitions of the operations that make
//up these sequences, please see the comments in the StartSequence
//implementation.
enum TPVSocketPortSequence
{
    EPVSocketPortSequence_None
    , EPVSocketPortSequence_RequestPort
    , EPVSocketPortSequence_InputConnectMsg
    , EPVSocketPortSequence_InputDataMsg
    , EPVSocketPortSequence_InputDisconnectMsg
    , EPVSocketPortSequence_SocketCleanup
    , EPVSocketPortSequence_Last
};

class SocketNodePortStats;

class SocketPortState
{
    public:
        SocketPortState(): iSequence(EPVSocketPortSequence_None)
                , iSequenceStatus(PVMFSuccess)
                , iConnectOperation(EPVSocketPortConnectOperation_None)
                , iConnectOperationStatus(PVMFSuccess)
                , iConnectOperationCanceled(false)
                , iSendOperation(EPVSocketPortSendOperation_None)
                , iSendOperationStatus(PVMFSuccess)
                , iSendOperationCanceled(false)
                , iRecvOperation(EPVSocketPortRecvOperation_None)
                , iRecvOperationStatus(PVMFSuccess)
                , iRecvOperationCanceled(false)
        {}

        TPVSocketPortSequence iSequence;
        PVMFStatus iSequenceStatus;

        TPVSocketPortConnectOperation iConnectOperation;
        PVMFStatus iConnectOperationStatus;
        bool iConnectOperationCanceled;

        TPVSocketPortSendOperation iSendOperation;
        PVMFStatus iSendOperationStatus;
        bool iSendOperationCanceled;

        TPVSocketPortRecvOperation iRecvOperation;
        PVMFStatus iRecvOperationStatus;
        bool iRecvOperationCanceled;
};

#if(ENABLE_SOCKET_NODE_STATS)
/*
** SocketNodeStats and SocketNodePortStats are used for tracking performance
** of socket node.
*/

//SocketNodePortStats contains all data that is maintained on a per-port basis.
class SocketNodePortStats
{
    public:
        SocketNodePortStats()
        {
            oscl_memset(this, 0, sizeof(SocketNodePortStats));
        }
        //port events.
        uint32 iNumPortEventConnectedPortReady;
        uint32 iNumPortEventConnect;
        uint32 iNumPortEventIncomingMsg;

        //incoming messages
        uint32 iNumDequeueIncomingConnectMsg;
        uint32 iNumDequeueIncomingDisconnectMsg;
        uint32 iNumDequeueIncomingDataMsg;

        //outgoing messages
        uint32 iNumQueueOutgoingUDPMsg;
        uint32 iNumQueueOutgoingTCPMsg;
        uint32 iNumQueueOutgoingEOSMsg;

        //socket receive results that had to be queued while
        //waiting on connected port
        uint32 iNumQueueSocketRecv;

        //number of times wait states were entered.
        uint32 iNumWaitOnMemory;
        uint32 iNumWaitOnConnectedPort;
        uint32 iNumWaitOnSendRecvIdle;

        //memory pool callbacks.
        uint32 iNumFreeChunkCallback;

        //Oscl socket & OScl DNS callbacks
        uint32 iNumSocketCallback;
        uint32 iNumDNSCallback;

        //number of Oscl socket and Oscl DNS calls.
        uint32 iNumSend;
        uint32 iNumSendTo;
        uint32 iNumRecv;
        uint32 iNumRecvFrom;
        uint32 iNumRecvFromPackets;
        uint32 iMaxRecvFromPackets;
        uint32 iNumGetHostByName;
        uint32 iNumConnect;
        uint32 iNumShutdown;

        //Latency times through this node.
        uint32 iMaxConnectOperationTime[EPVSocketPortConnectOperation_Last];
        uint32 iMaxSendOperationTime[EPVSocketPortSendOperation_Last];
        uint32 iMaxRecvOperationTime[EPVSocketPortRecvOperation_Last];

        uint32 iConnectOperationTime[EPVSocketPortConnectOperation_Last];
        uint32 iSendOperationTime[EPVSocketPortSendOperation_Last];
        uint32 iRecvOperationTime[EPVSocketPortRecvOperation_Last];

        void StartConnectTime(TPVSocketPortConnectOperation aOp)
        {
            iConnectOperationTime[aOp] = OsclTickCount::TicksToMsec(OsclTickCount::TickCount());
        }
        void EndConnectTime(TPVSocketPortConnectOperation aOp)
        {
            uint32 delta = OsclTickCount::TicksToMsec(OsclTickCount::TickCount()) - iConnectOperationTime[aOp];
            if (delta > iMaxConnectOperationTime[aOp])
                iMaxConnectOperationTime[aOp] = delta;
        }
        void StartRecvTime(TPVSocketPortRecvOperation aOp)
        {
            iRecvOperationTime[aOp] = OsclTickCount::TicksToMsec(OsclTickCount::TickCount());
        }
        void EndRecvTime(TPVSocketPortRecvOperation aOp)
        {
            uint32 delta = OsclTickCount::TicksToMsec(OsclTickCount::TickCount()) - iRecvOperationTime[aOp];
            if (delta > iMaxRecvOperationTime[aOp])
                iMaxRecvOperationTime[aOp] = delta;
        }
        void StartSendTime(TPVSocketPortSendOperation aOp)
        {
            iSendOperationTime[aOp] = OsclTickCount::TicksToMsec(OsclTickCount::TickCount());
        }
        void EndSendTime(TPVSocketPortSendOperation aOp)
        {
            uint32 delta = OsclTickCount::TicksToMsec(OsclTickCount::TickCount()) - iSendOperationTime[aOp];
            if (delta > iMaxSendOperationTime[aOp])
                iMaxSendOperationTime[aOp] = delta;
        }
        void Log(PVLogger* iLogger, OSCL_String& aMime)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats:PORT '%s'", aMime.get_cstr()));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num PortEventConnectedPortReady", iNumPortEventConnectedPortReady));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num PortEventConnect", iNumPortEventConnect));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num PortEventIncomingMsg", iNumPortEventIncomingMsg));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num DequeueIncomingConnectMsg", iNumDequeueIncomingConnectMsg));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num DequeueIncomingDisconnectMsg", iNumDequeueIncomingDisconnectMsg));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num DequeueIncomingDataMsg", iNumDequeueIncomingDataMsg));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num QueueOutgoingUDPMsg", iNumQueueOutgoingUDPMsg));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num QueueOutgoingTCPMsg", iNumQueueOutgoingTCPMsg));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num QueueOutgoingEOSMsg", iNumQueueOutgoingEOSMsg));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num QueueSocketRecv", iNumQueueSocketRecv));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num WaitOnMemory", iNumWaitOnMemory));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num WaitOnConnectedPort", iNumWaitOnConnectedPort));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num WaitOnSendRecvIdle", iNumWaitOnSendRecvIdle));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num FreeChunkCallback", iNumFreeChunkCallback));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num SocketCallback", iNumSocketCallback));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num DNSCallback", iNumDNSCallback));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num Send", iNumSend));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num SendTo", iNumSendTo));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num Recv", iNumRecv));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num RecvFrom", iNumRecvFrom));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num RecvFrom Packets", iNumRecvFromPackets));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Max RecvFrom Packets", iMaxRecvFromPackets));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num GetHostByName", iNumGetHostByName));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num Connect", iNumConnect));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Num Shutdown", iNumShutdown));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Max Connect GetHostByName Time (msec)", iMaxConnectOperationTime[EPVSocketPortConnectOperation_GetHostByName]));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Max Connect Time (msec)", iMaxConnectOperationTime[EPVSocketPortConnectOperation_Connect]));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Max Connect Shutdown Time (msec)", iMaxConnectOperationTime[EPVSocketPortConnectOperation_Shutdown]));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Max Send Time (msec)", iMaxSendOperationTime[EPVSocketPortSendOperation_Send]));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Max SendTo Time (msec)", iMaxSendOperationTime[EPVSocketPortSendOperation_SendTo]));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Max Recv Time (msec)", iMaxRecvOperationTime[EPVSocketPortRecvOperation_Recv]));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Max RecvFrom Time (msec)", iMaxRecvOperationTime[EPVSocketPortRecvOperation_RecvFrom]));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Max Recv Wait On Connected Port Time (msec)", iMaxRecvOperationTime[EPVSocketPortRecvOperation_WaitOnConnectedPort]));
            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats: %8d Max Recv Wait On Memory Time (msec)", iMaxRecvOperationTime[EPVSocketPortRecvOperation_WaitOnMemory]));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF, iLogger, PVLOGMSG_ERR,
                            (0, "SocketNodeStats:@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
        }
};

//SocketNodeStats contains all data that is maintained on a per-node basis.
class SocketNodeStats
{
    public:
        SocketNodeStats()
        {
            oscl_memset(this, 0, sizeof(SocketNodeStats));
        }

        PVLogger* iLogger;
        bool iLogged;
        void Init()
        {
            iLogged = false;
            iLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.socketnode");
        }
        void Logoff()
        {
            iLogger = NULL;
        }

        //Run calls
        uint32 iNumRun;

        //Node cmds.
        uint32 iNumQueueNodeCmd;

        //Number of UDP bind operations.
        uint32 iNumBind;

        void Log(PVMFPortVector<PVMFSocketPort, PVMFSocketNodeAllocator>& aPortVec);
};
#endif

/*
** SocketPortConfig contains all the information associated with a port including
** the Oscl socket, memory pool, and status.
*/
class SocketPortConfig : public OsclMemPoolFixedChunkAllocatorObserver, public OsclMemPoolResizableAllocatorObserver
{
    public:
        SocketPortConfig()
        {
            iPVMFPort = NULL;
            iSockId = 0;
            iUDPSocket = NULL;
            iTCPSocket = NULL;
            iDNS = NULL;
            iMemPool	= NULL;
            iContainer = NULL;
            iTag = PVMF_SOCKET_NODE_PORT_TYPE_UNKNOWN;
            iRTP = false;
            iRTCP = false;
        };
        void CleanupMemPools(Oscl_DefAlloc& aAlloc);
        void CreateAllocators(uint32 aSize, uint32 aExpectedNumberOfBlocksPerBuffer, uint32 aResizeSize, uint32 aMaxNumResizes);

        void freechunkavailable(OsclAny* aContextData);
        void freeblockavailable(OsclAny* aContextData);

        SocketPortState iState;

        PVMFSocketPort *iPVMFPort;
        SOCKET_ADDR	iAddr;
        uint32	iSockId;
        OsclUDPSocket* iUDPSocket;
        OsclTCPSocket* iTCPSocket;
        OsclDNS *iDNS;
        OsclNetworkAddress iSourceAddr;//arg for recv from calls.
        Oscl_Vector<uint32, OsclMemAllocator> iRecvFromPacketLen;//arg for recv from calls.

        //socket pending request and state
        PVMFSharedMediaDataPtr iPendingRecvMediaData, iPendingSendMediaData;

        //Fixed chunk memory pool for media messages
        PVMFSocketNodeMemPool *iMemPool;

        PVMFSocketNode* iContainer;
        int32 iTag;

        PVMFSocketActivity iSocketRecvActivity;

        OSCL_HeapString<OsclMemAllocator> iMime;
        bool iRTP;
        bool iRTCP;

#if(ENABLE_SOCKET_NODE_STATS)
        SocketNodePortStats iPortStats;
#endif
};

/*
** DnsCache is used to cache DNS lookup results to avoid unnecessary lookups of a previously
** used host.
*/
class PVMFDnsCache
{
    public:
        PVMFDnsCache(uint32 aNumOfRecords = 4): iNumOfRecords(aNumOfRecords), iAddrIP(NULL), iAddrDNS(NULL)
        {
        };
        ~PVMFDnsCache()
        {
            for (uint32 i = 0; i < iNumOfRecords; i++)
            {
                if (iAddrDNS)
                {
                    if (NULL != iAddrDNS[i])
                    {
                        OSCL_ARRAY_DELETE(iAddrDNS[i]);
                        iAddrDNS[i] = NULL;
                    }
                }
                if (iAddrIP)
                {
                    if (NULL != iAddrIP[i])
                    {
                        OSCL_ARRAY_DELETE(iAddrIP[i]);
                        iAddrIP[i] = NULL;
                    }
                }
            }
            OSCL_ARRAY_DELETE(iAddrIP);
            OSCL_ARRAY_DELETE(iAddrDNS);
        }
        void NewL(void)
        {
            iAddrIP = OSCL_ARRAY_NEW(char*, iNumOfRecords);
            iAddrDNS = OSCL_ARRAY_NEW(char*, iNumOfRecords);
            if ((NULL == iAddrIP) || (NULL == iAddrIP))
            {
                OSCL_ARRAY_DELETE(iAddrIP);
                OSCL_ARRAY_DELETE(iAddrDNS);
                OSCL_LEAVE(OsclErrNoMemory);
            }
            for (uint32 i = 0; i < iNumOfRecords; i++)
                iAddrDNS[i] = iAddrIP[i] = NULL;
        };

        //if name is not in record, add one; if it does, update it
        bool UpdateCacheRecord(char *name, char *ipAddr)
        {
            for (uint32 i = 0; i < iNumOfRecords; i++)
            {
                if (NULL == iAddrDNS[i])
                {//new record
                    int32 tmpLen = oscl_strlen(name);
                    iAddrDNS[i] = OSCL_ARRAY_NEW(char, tmpLen + 4);
                    oscl_strncpy(iAddrDNS[i], name, tmpLen + 2);
                    iAddrIP[i] = OSCL_ARRAY_NEW(char, 32);
                    oscl_strncpy(iAddrIP[i], ipAddr, 30);
                    return true;
                }
                if (0 == oscl_strcmp(iAddrDNS[i], name))
                {//update record
                    if (oscl_strlen(ipAddr) + 1 > 32)
                    {
                        return false;
                    }
                    oscl_strncpy(iAddrIP[i], ipAddr, 30);
                    return true;
                }
            }
            return false;
        }
        //find the ip address corresponds to name. len is the size of ipAddr bufferr
        bool QueryGetHostByName(const char *name, char *ipAddr, const uint32 len)
        {
            for (uint32 i = 0; i < iNumOfRecords; i++)
            {
                if (NULL == iAddrDNS[i])
                    return false;
                if (0 == oscl_strcmp(iAddrDNS[i], name))
                {
                    if (oscl_strlen(iAddrIP[i]) + 1 > len)
                    {
                        return false;
                    }
                    oscl_strncpy(ipAddr, iAddrIP[i], 30);
                    return true;
                }
            }
            return false;
        }
    private:
        uint32 iNumOfRecords;
        char** iAddrIP;
        char** iAddrDNS;
};

/*
** Memory pool sizes.
*/
#define SIMPLE_MEDIA_BUF_CLASS_SIZE 128 /*oscl_mem_aligned_size(sizeof(PVMFSimpleMediaBuffer) + oscl_mem_aligned_size(sizeof(OsclRefCounterSA<SimpleMediaBufferCombinedCleanupSA>)*/
#define MEDIA_DATA_CLASS_SIZE 128 /*oscl_mem_aligned_size(sizeof(PVMFMediaData)) + oscl_mem_aligned_size(sizeof(OsclRefCounterDA)) + oscl_mem_aligned_size(sizeof(MediaDataCleanupDA)) + sizeof(PVMFMediaMsgHeader))*/

/*
** The Socket Node
*/
class PVMFSocketNode
            : public PVInterface
            , public PVMFNodeInterface
            , public OsclActiveObject
            , public OsclSocketObserver
            , public OsclDNSObserver
{
    public:
        OSCL_IMPORT_REF PVMFSocketNode(int32 aPriority);
        OSCL_IMPORT_REF ~PVMFSocketNode();

        //************ begin OsclSocketObserver
        OSCL_IMPORT_REF void HandleSocketEvent(int32 aId, TPVSocketFxn aFxn, TPVSocketEvent aEvent, int32 aError);
        //************ end OsclSocketObserver

        //************ begin OsclDNSObserver
        OSCL_IMPORT_REF void HandleDNSEvent(int32 aId, TPVDNSFxn aFxn, TPVDNSEvent aEvent, int32 aError);
        //************ end OsclDNSObserver

        //from PVMFNodeInterface
        OSCL_IMPORT_REF PVMFStatus ThreadLogon();
        OSCL_IMPORT_REF PVMFStatus ThreadLogoff();
        OSCL_IMPORT_REF PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        OSCL_IMPORT_REF PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryUUID(PVMFSessionId, const PvmfMimeString& aMimeType,
                                                Oscl_Vector<PVUuid, PVMFSocketNodeAllocator>& aUuids,
                                                bool aExactUuidsOnly = false,
                                                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryInterface(PVMFSessionId, const PVUuid& aUuid,
                PVInterface*& aInterfacePtr,
                const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFCommandId RequestPort(PVMFSessionId aSession
                , int32 aPortTag
                , const PvmfMimeString* aPortConfig = NULL
                                                      , const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFCommandId ReleasePort(PVMFSessionId, PVMFPortInterface& aPort, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Init(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Prepare(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Start(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Stop(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Flush(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Pause(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Reset(PVMFSessionId, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelAllCommands(PVMFSessionId, const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelCommand(PVMFSessionId, PVMFCommandId aCmdId, const OsclAny* aContextData = NULL);

        //from PVMFPortActivityHandler
        void HandlePortActivity(const PVMFPortActivity& aActivity);

        //These are some extra APIs that are used by Streaming Manager and are unique to socket node.
        OSCL_IMPORT_REF bool GetPortConfig(PVMFPortInterface &aPort, OsclNetworkAddress &aLocalAdd, OsclNetworkAddress &aRemoteAdd);
        OSCL_IMPORT_REF bool SetPortConfig(PVMFPortInterface &aPort, OsclNetworkAddress aLocalAdd, OsclNetworkAddress aRemoteAdd);

        virtual void addRef()
        {
        }

        virtual void removeRef()
        {
        }

        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        //**********begin PVMFSocketNodeExtensionInterface
        PVMFStatus AllocateConsecutivePorts(PvmfMimeString* aPortConfig,
                                            uint32& aLowerPortNum,
                                            uint32& aHigherPortNum, uint32& aStartPortNum);
        OSCL_IMPORT_REF PVMFStatus SetMaxTCPRecvBufferSize(uint32 aBufferSize);
        OSCL_IMPORT_REF PVMFStatus GetMaxTCPRecvBufferSize(uint32& aSize);
        OSCL_IMPORT_REF PVMFStatus SetMaxTCPRecvBufferCount(uint32 aBufferSize);
        OSCL_IMPORT_REF PVMFStatus GetMaxTCPRecvBufferCount(uint32& aSize);
        OsclMemPoolResizableAllocator* CreateSharedBuffer(const PVMFPortInterface* aPort , uint32 aBufferSize, uint32 aExpectedNumberOfBlocksPerBuffer, uint32 aResizeSize, uint32 aMaxNumResizes);
        //**********end PVMFSocketNodeExtensionInterface

    private:
        friend class SocketPortConfig;

        //from OsclActiveObject
        void Run();

        /*********************************************
        * Command Processing and Event Notification
        **********************************************/
        //Command queue type
        typedef PVMFNodeCommandQueue<PVMFSocketNodeCommand, PVMFSocketNodeAllocator> PVMFSocketNodeCmdQ;

        PVMFSocketNodeCmdQ iPendingCmdQueue;
        PVMFSocketNodeCmdQ iCurrentCmdQueue;
        PVMFSocketNodeCmdQ iCancelCmdQueue;

        void MoveCmdToCancelQueue(PVMFSocketNodeCommand& aCmd);

        void CommandComplete(PVMFSocketNodeCmdQ&,
                             PVMFSocketNodeCommand&,
                             PVMFStatus,
                             OsclAny* aData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);

        void ReportErrorEvent(PVMFEventType aEventType,
                              OsclAny* aEventData = NULL,
                              PVUuid* aEventUUID = NULL,
                              int32* aEventCode = NULL);

        void ReportInfoEvent(PVMFEventType aEventType,
                             OsclAny* aEventData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);

        PVMFCommandId QueueCommandL(PVMFSocketNodeCommand& aCmd);

        bool CanProcessCommand();
        void ProcessCommand(PVMFSocketNodeCmdQ& aCmdQ, PVMFSocketNodeCommand&);

        //Command handlers.
        PVMFStatus DoRequestPort(PVMFSocketNodeCommand& aCmd, PVMFSocketPort* &port);
        PVMFStatus DoReset(PVMFSocketNodeCommand&);
        PVMFStatus DoQueryUuid(PVMFSocketNodeCommand&);
        PVMFStatus DoQueryInterface(PVMFSocketNodeCommand&);
        PVMFStatus DoReleasePort(PVMFSocketNodeCommand&);
        PVMFStatus DoInit(PVMFSocketNodeCommand&);
        PVMFStatus DoPrepare(PVMFSocketNodeCommand&);
        PVMFStatus DoStart(PVMFSocketNodeCommand&);
        PVMFStatus DoStop(PVMFSocketNodeCommand&);
        PVMFStatus DoFlush(PVMFSocketNodeCommand&);
        PVMFStatus DoPause(PVMFSocketNodeCommand&);
        PVMFStatus DoCancelAllCommands(PVMFSocketNodeCommand&);
        PVMFStatus DoCancelCommand(PVMFSocketNodeCommand&);

        PVMFStatus DoCancelCurrentCommand(PVMFSocketNodeCmdQ& aCmdQ, PVMFSocketNodeCommand& aCmd);
        PVMFStatus DoStopNodeActivity();
        int32 SocketPlacementNew(PVMFSocketNodeMemPool*&, OsclAny*, int32);
        int32 CreateMediaData(SocketPortConfig&, OsclSharedPtr<PVMFMediaDataImpl>&);
        int32 Allocate(SocketPortConfig&, OsclSharedPtr<PVMFMediaDataImpl>&);
        int32 GetMediaDataImpl(SocketPortConfig&, OsclSharedPtr<PVMFMediaDataImpl>&, int32);

        PVMFSocketPort* iRequestedPort;

        //node state
        void ChangeExternalState(TPVMFNodeInterfaceState aNewState)
        {
            iInterfaceState = aNewState;
        }

        //node capability
        PVMFNodeCapability iCapability;

        //for error messages.
        int32 iCommandErrorCode;
        int32 iErrorEventErrorCode;
        void ReportSocketNodeError(PVMFStatus aStatus, PVMFSocketNodeErrorEventType aEvent)
        {
            PVUuid eventuuid = PVMFSocketNodeEventTypeUUID;
            iErrorEventErrorCode = aEvent;
            ReportErrorEvent(aStatus, NULL, &eventuuid, &iErrorEventErrorCode);
        }

        //Used to stop all port activity, this var counts down from N to 0, or
        //has a value of (-1) to indicate it's inactive
        int32 iNumStopPortActivityPending;

        /*********************************************
        * Oscl Socket Handling
        **********************************************/

        void HandleRecvComplete(SocketPortConfig& tmpSockConfig, PVMFStatus, PVMFSocketActivity*, bool);
        void HandleRecvFromComplete(SocketPortConfig& tmpSockConfig, PVMFStatus, PVMFSocketActivity*, bool);

        OsclSocketServ	*iSockServ;

        const int TIMEOUT_CONNECT;
        const int TIMEOUT_SEND;
        const int TIMEOUT_SENDTO;
        const int TIMEOUT_RECV;
        const int TIMEOUT_RECVFROM;
        const int TIMEOUT_SHUTDOWN;
        const int UDP_PORT_RANGE;
        const int MAX_UDP_PACKET_SIZE;
        const int MIN_UDP_PACKET_SIZE;

        int32 iMaxTcpRecvBufferSize;
        int32 iMaxTcpRecvBufferCount;

        /*********************************************
        * Oscl DNS Handling
        **********************************************/

        PVMFDnsCache	iDnsCache;

        /*********************************************
        * Port Data Handling
        **********************************************/

        bool CanProcessIncomingMsg(SocketPortConfig& aSockConfig);

        void ProcessIncomingMsg(SocketPortConfig& aSockConfig);

        bool ParseTransportConfig(OSCL_String *aPortConfig,
                                  SOCKET_ADDR &aSockConfig,
                                  OSCL_String& aMime);
        bool ParseTransportConfig(char *aPortConfig,
                                  int32 aLen,
                                  SOCKET_ADDR &aSockConfig,
                                  OSCL_String& aMime);

        /*********************************************
        * Port & Socket Creation
        **********************************************/

        PVMFStatus AllocatePortMemPool(int32 tag, PVMFSocketNodeMemPool* & aMemPool);
        PVMFStatus AddPort(int32 tag, PVMFSocketPort* &port);
        void CleanupTCP(SocketPortConfig& tmpSockConfig);
        void CleanupUDP(SocketPortConfig& tmpSockConfig);
        void CleanupDNS(SocketPortConfig& tmpSockConfig);
        void CleanupPorts();

        OsclAny* CreateOsclSocketAndBind(SOCKET_ADDR &, uint32);

        uint32 iSocketID;
        bool iInSocketCallback;

        Oscl_Vector<OsclTCPSocket*, OsclMemAllocator> iClosedTCPSocketVector;
        void CleanupClosedTCPSockets();
        Oscl_Vector<OsclUDPSocket*, OsclMemAllocator> iClosedUDPSocketVector;
        void CleanupClosedUDPSockets();
        Oscl_Vector<OsclDNS*, OsclMemAllocator> iClosedDNSVector;
        void CleanupClosedDNS();

        /*********************************************
        * Port Data
        **********************************************/

        PVMFPortVector<PVMFSocketPort, PVMFSocketNodeAllocator> iPortVector;
        Oscl_Vector<SocketPortConfig*, PVMFSocketNodeAllocator> iAllocatedPortVector;

        SocketPortConfig* FindSocketPortConfig(SOCKET_ADDR& aSockConfig);
        bool MatchSocketAddr(SOCKET_ADDR& aSockAddr, SocketPortConfig& aSockConfig);
        SocketPortConfig* FindSocketPortConfig(uint32 aId);

        /*********************************************
        * Operation and Sequence Handlers
        **********************************************/

        PVMFStatus StartSequence(SocketPortConfig&, TPVSocketPortSequence, OsclAny* param = NULL);
        void SequenceComplete(SocketPortConfig& tmpSockConfig, PVMFStatus);

        bool CanReceive(SocketPortConfig& aConfig);

        PVMFStatus StartConnectOperation(SocketPortConfig& aSockConfig, TPVSocketPortConnectOperation aOperation);
        PVMFStatus StartRecvOperation(SocketPortConfig& aSockConfig);
        PVMFStatus StartSendOperation(SocketPortConfig& aSockConfig, PVMFSharedMediaMsgPtr& aMsg);
        void StartRecvWaitOnMemory(SocketPortConfig& aSockConfig, int32 aSize = 0);
        void StartRecvWaitOnConnectedPort(SocketPortConfig& aSockConfig, PVMFSocketActivity& aSocketActivity);

        PVMFStatus ConnectOperationComplete(SocketPortConfig& tmpSockConfig, PVMFStatus, PVMFSocketActivity*);
        PVMFStatus SendOperationComplete(SocketPortConfig& tmpSockConfig, PVMFStatus, PVMFSocketActivity*);
        PVMFStatus RecvOperationComplete(SocketPortConfig& tmpSockConfig, PVMFStatus, PVMFSocketActivity*);

        PVMFStatus CancelSendOperation(SocketPortConfig& tmpSockConfig);
        PVMFStatus CancelRecvOperation(SocketPortConfig& tmpSockConfig);
        PVMFStatus CancelConnectOperation(SocketPortConfig& tmpSockConfig);

        /*********************************************
        * Loggers, Error Trap, Allocator
        **********************************************/

        PVLogger *iLogger;
        PVLogger *iDataPathLogger;
        PVLogger *iDataPathLoggerRTP;
        PVLogger *iDataPathLoggerRTCP;

        void LogRTPHeaderFields(SocketPortConfig& aSockConfig,
                                OsclRefCounterMemFrag& memFragIn);

        void LogRTCPHeaderFields(SocketPortConfig& aSockConfig,
                                 OsclRefCounterMemFrag& memFragIn);

        OsclErrorTrapImp* iOsclErrorTrapImp;

        PVMFSocketNodeAllocator iAlloc;
        PVMFSocketNodeExtensionInterface* iExtensionInterface;
#if(ENABLE_SOCKET_NODE_STATS)
        SocketNodeStats iSocketNodeStats;
#endif
};

class PVMFSocketNodeExtensionInterfaceImpl: public PVInterfaceImpl<PVMFSocketNodeAllocator>, public  PVMFSocketNodeExtensionInterface
{
    public:
        PVMFSocketNodeExtensionInterfaceImpl(PVMFSocketNode* iContainer);
        ~PVMFSocketNodeExtensionInterfaceImpl();
        virtual void addRef();
        virtual void removeRef();
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface); //From PVInterface
        OSCL_IMPORT_REF virtual PVMFStatus AllocateConsecutivePorts(PvmfMimeString* aPortConfig,
                uint32& aLowerPortNum,
                uint32& aHigherPortNum, uint32& aStartPortNum);

        OSCL_IMPORT_REF virtual PVMFStatus SetMaxTCPRecvBufferSize(uint32 aBufferSize);
        OSCL_IMPORT_REF virtual PVMFStatus GetMaxTCPRecvBufferSize(uint32& aSize);
        OSCL_IMPORT_REF virtual PVMFStatus SetMaxTCPRecvBufferCount(uint32 aCount);
        OSCL_IMPORT_REF virtual PVMFStatus GetMaxTCPRecvBufferCount(uint32& aCount);
        OSCL_IMPORT_REF virtual OsclMemPoolResizableAllocator* CreateSharedBuffer(const PVMFPortInterface* aPort , uint32 aBufferSize, uint32 aExpectedNumberOfBlocksPerBuffer, uint32 aResizeSize, uint32 aMaxNumResizes);
    private:
        PVMFSocketNode *iContainer;
};

#endif //PVMF_SOCKET_NODE_H_INCLUDED

