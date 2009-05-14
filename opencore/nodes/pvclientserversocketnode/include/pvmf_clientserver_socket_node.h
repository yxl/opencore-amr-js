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
#ifndef PVMF_CLIENTSERVER_SOCKET_NODE_H_INCLUDED
#define PVMF_CLIENTSERVER_SOCKET_NODE_H_INCLUDED

#ifndef OSCL_SOCKET_TYPES_H_INCLUDED
#include "oscl_socket_types.h"
#endif
#ifndef PVMF_CLIENTSERVER_SOCKET_TUNEABLES_H_INCLUDED
#include "pvmf_clientserver_socket_tuneables.h"
#endif

#ifndef PVMF_CLIENTSERVER_SOCKET_PORT_H_INCLUDED
#include "pvmf_clientserver_socket_port.h"
#endif

//Logger macros
#define PVMF_SOCKETNODE_LOGSTACKTRACE(m)	PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_STACK_TRACE,m);
#define PVMF_SOCKETNODE_LOGINFO(m)			PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_INFO,m);
#define PVMF_SOCKETNODE_LOGERROR(m)			PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);

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
class PVMFClientServerSocketPort;

/**
 * Mem pool class
 */
class PVMFSocketNodeMemPool
{
    public:
        PVMFSocketNodeMemPool(uint32 aMemPoolNumBufs = DEFAULT_SOCKET_NODE_MEDIA_MSGS_NUM);

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
            return (iSocketAllocSharedPtr->createSharedBuffer(size));
        }

        void resizeSocketDataBuffer(OsclSharedPtr<PVMFMediaDataImpl>& aSharedBuffer)
        {
            if (iSocketAllocSharedPtr.GetRep() != NULL)
            {
                iSocketAllocSharedPtr->ResizeMemoryFragment(aSharedBuffer);
            }
        }

        void notifyfreechunkavailable(OsclMemPoolFixedChunkAllocatorObserver& aObserver,
                                      uint32 aSize,
                                      OsclAny* aContextData = NULL)
        {
            if (iSocketAllocSharedPtr.GetRep() != NULL)
            {
                iSocketAllocSharedPtr->notifyfreechunkavailable(aObserver, aSize, aContextData);
            }
        }

        void CancelFreeChunkAvailableCallback()
        {
            if (iSocketAllocSharedPtr.GetRep() != NULL)
            {
                iSocketAllocSharedPtr->CancelFreeChunkAvailableCallback();
            }
        }

        OsclSharedPtr<PVMFSharedSocketDataBufferAlloc> iSocketAllocSharedPtr;

        // Memory pool for media data objects
        OsclMemPoolFixedChunkAllocator* iMediaDataMemPool;

        int32 iPortTag;

        // Allocator created in this node, for TCP only.
        PVMFSMSharedBufferAllocWithReSize* iInternalAlloc;
};

class PVMFClientServerSocketNode;

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

//Individual asynchronous operations and wait states that make up a send sequence.
enum TPVSocketPortSendOperation
{
    EPVSocketPortSendOperation_None
    , EPVSocketPortSendOperation_Send
    , EPVSocketPortSendOperation_Last
};

//Individual asynchronous operations and wait states that make up a receive sequence.
enum TPVSocketPortRecvOperation
{
    EPVSocketPortRecvOperation_None
    , EPVSocketPortRecvOperation_Recv
    , EPVSocketPortRecvOperation_WaitOnConnectedPort
    , EPVSocketPortRecvOperation_WaitOnMemory
    , EPVSocketPortRecvOperation_Last
};

enum TPVSocketPortSequence
{
    EPVSocketPortSequence_None
    , EPVSocketPortSequence_RequestPort
    , EPVSocketPortSequence_InputDataMsg
    , EPVSocketPortSequence_InputDisconnectMsg
    , EPVSocketPortSequence_SocketCleanup
    , EPVSocketPortSequence_Last
};

class SocketPortState
{
    public:
        SocketPortState(): iSequence(EPVSocketPortSequence_None)
                , iSequenceStatus(PVMFSuccess)
                , iSendOperation(EPVSocketPortSendOperation_None)
                , iSendOperationStatus(PVMFSuccess)
                , iSendOperationCanceled(false)
                , iRecvOperation(EPVSocketPortRecvOperation_None)
                , iRecvOperationStatus(PVMFSuccess)
                , iRecvOperationCanceled(false)
        {}

        TPVSocketPortSequence iSequence;
        PVMFStatus iSequenceStatus;

        TPVSocketPortSendOperation iSendOperation;
        PVMFStatus iSendOperationStatus;
        bool iSendOperationCanceled;

        TPVSocketPortRecvOperation iRecvOperation;
        PVMFStatus iRecvOperationStatus;
        bool iRecvOperationCanceled;
};

/*
** SocketPortConfig contains all the information associated with a port including
** the Oscl socket, memory pool, and status.
*/
class SocketPortConfig : public OsclMemPoolFixedChunkAllocatorObserver
{
    public:
        SocketPortConfig()
        {
            iPVMFPort = NULL;
            iSockId = 0;
            iTCPSocket = NULL;
            iMemPool	= NULL;
            iContainer = NULL;
            iTag = PVMF_CLIENTSERVER_SOCKET_NODE_PORT_TYPE_UNKNOWN;
        };

        void DoSetSocketPortMemAllocator(PVLogger*, OsclSharedPtr<PVMFSharedSocketDataBufferAlloc> aAlloc);
        void CleanupMemPools();

        void freechunkavailable(OsclAny* aContextData);

        SocketPortState iState;

        PVMFClientServerSocketPort *iPVMFPort;
        uint32	iSockId;
        OsclTCPSocket* iTCPSocket;

        //socket pending request and state
        PVMFSharedMediaDataPtr iPendingRecvMediaData, iPendingSendMediaData;

        //Fixed chunk memory pool for media messages
        PVMFSocketNodeMemPool *iMemPool;

        PVMFClientServerSocketNode* iContainer;
        int32 iTag;

        PVMFSocketActivity iSocketRecvActivity;
};

/*
** Memory pool sizes.
*/
#define SIMPLE_MEDIA_BUF_CLASS_SIZE 128 /*oscl_mem_aligned_size(sizeof(PVMFSimpleMediaBuffer) + oscl_mem_aligned_size(sizeof(OsclRefCounterSA<SimpleMediaBufferCombinedCleanupSA>)*/
#define MEDIA_DATA_CLASS_SIZE 128 /*oscl_mem_aligned_size(sizeof(PVMFMediaData)) + oscl_mem_aligned_size(sizeof(OsclRefCounterDA)) + oscl_mem_aligned_size(sizeof(MediaDataCleanupDA)) + sizeof(PVMFMediaMsgHeader))*/

/*
** The Socket Node
*/
class PVMFClientServerSocketNode  :	public PVMFNodeInterface
            , public OsclActiveObject
            , public OsclSocketObserver
{
    public:

        OSCL_IMPORT_REF PVMFClientServerSocketNode(OsclTCPSocket* aSocketHandle, int32 aPriority);
        OSCL_IMPORT_REF ~PVMFClientServerSocketNode();


        //**********begin PVMFSocketNodeExtensionInterface
        OSCL_IMPORT_REF void addRef();
        OSCL_IMPORT_REF void removeRef();
        OSCL_IMPORT_REF bool queryInterface(const PVUuid& uuid, PVInterface*& iface);
        OSCL_IMPORT_REF PVMFStatus AllocateConsecutivePorts(PvmfMimeString* aPortConfig,
                uint32& aLowerPortNum,
                uint32& aHigherPortNum, uint32& aStartPortNum);
        OSCL_IMPORT_REF PVMFStatus SetMaxTCPRecvBufferSize(uint32 aBufferSize);
        OSCL_IMPORT_REF PVMFStatus GetMaxTCPRecvBufferSize(uint32& aSize);
        OSCL_IMPORT_REF PVMFStatus SetMaxTCPRecvBufferCount(uint32 aBufferSize);
        OSCL_IMPORT_REF PVMFStatus GetMaxTCPRecvBufferCount(uint32& aSize);

        //**********end PVMFSocketNodeExtensionInterface

        //************ begin OsclSocketObserver
        OSCL_IMPORT_REF void HandleSocketEvent(int32 aId, TPVSocketFxn aFxn, TPVSocketEvent aEvent, int32 aError);
        //************ end OsclSocketObserver

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

        PVMFStatus CloseSocketConnection();
        OSCL_IMPORT_REF bool setSocketPortMemAllocator(PVMFPortInterface* aPort,
                OsclSharedPtr<PVMFSharedSocketDataBufferAlloc> aAlloc);


    private:
        friend class SocketPortConfig;

        //from OsclActiveObject
        void Run();

        /*********************************************
        * Socket extension interface
        **********************************************/

        // Reference counter for extension
        uint32 iExtensionRefCount;

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
        PVMFStatus DoRequestPort(PVMFSocketNodeCommand& aCmd);
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
        //PVMFClientServerSocketPort* iRequestedPort;

        //node state
        void ChangeExternalState(TPVMFNodeInterfaceState aNewState)
        {
            iInterfaceState = aNewState;
        }

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

        void HandleRecvComplete(PVMFStatus, PVMFSocketActivity*, bool);

        OsclSocketServ	*iSockServ;

        const int TIMEOUT_CONNECT;
        const int TIMEOUT_SEND;
        const int TIMEOUT_RECV;
        const int TIMEOUT_SHUTDOWN;

        int32 iMaxTcpRecvBufferSize;
        int32 iMaxTcpRecvBufferCount;

        /*********************************************
        * Port Data Handling
        **********************************************/

        bool CanProcessIncomingMsg();

        void ProcessIncomingMsg();


        /*********************************************
        * Port & Socket Creation
        **********************************************/

        PVMFStatus AllocatePortMemPool(int32 tag, PVMFSocketNodeMemPool* & aMemPool);
        PVMFStatus AddPort(int32 tag);
        void CleanupTCP();
        void CleanupPorts();

        uint32 iSocketID;

        /*********************************************
        * Port Data
        **********************************************/

        PVMFClientServerSocketPort* iPVMFPort;

        /*********************************************
        * Operation and Sequence Handlers
        **********************************************/

        PVMFStatus StartSequence(TPVSocketPortSequence, OsclAny* param = NULL);
        void SequenceComplete(PVMFStatus);

        bool CanReceive();

        PVMFStatus StartRecvOperation();
        PVMFStatus StartSendOperation(PVMFSharedMediaMsgPtr& aMsg);
        void StartRecvWaitOnMemory(int32 aSize = 0);
        void StartRecvWaitOnConnectedPort(PVMFSocketActivity& aSocketActivity);

        PVMFStatus SendOperationComplete(PVMFStatus, PVMFSocketActivity*);
        PVMFStatus RecvOperationComplete(PVMFStatus, PVMFSocketActivity*);

        PVMFStatus CancelSendOperation();
        PVMFStatus CancelRecvOperation();

        /*********************************************
        * Loggers, Error Trap, Allocator
        **********************************************/

        PVLogger *iLogger;
        PVLogger *iDataPathLogger;
        OsclErrorTrapImp* iOsclErrorTrapImp;
        OsclTCPSocket* iSocketHandle;

        PVMFSocketNodeAllocator iAlloc;
        SocketPortConfig* iSockConfig;
};

#endif //PVMF_CLIENTSERVER_SOCKET_NODE_H_INCLUDED

