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
#ifndef PVAE_NODE_UTILITY_H_INCLUDED
#define PVAE_NODE_UTILTIY_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#define LOG_STACK_TRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, m)
#define LOG_DEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, m)
#define LOG_ERR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m)

/** Structure to contain a node and all ports and extensions associated to it */
struct PVAENodeContainer
{
    PVMFNodeInterface* iNode;
    PVMFSessionId iSessionId;
    PVUuid iUuid;
    Oscl_Vector<PVMFPortInterface*, OsclMemAllocator> iInputPorts;
    Oscl_Vector<PVMFPortInterface*, OsclMemAllocator> iOutputPorts;
    Oscl_Vector<PVInterface*, OsclMemAllocator> iExtensions;
    Oscl_Vector<PVUuid, OsclMemAllocator> iExtensionUuids;
    PVInterface* iNodeCapConfigIF;
};

/** A vector of node container structures */
typedef Oscl_Vector<PVAENodeContainer*, OsclMemAllocator> PVAENodeContainerVector;

/** Enumerated list of node commands */
typedef enum
{
    PVAENU_CMD_NONE,
    PVAENU_CMD_CONNECT,
    PVAENU_CMD_DISCONNECT,
    PVAENU_CMD_QUERY_UUID,
    PVAENU_CMD_QUERY_INTERFACE,
    PVAENU_CMD_INIT,
    PVAENU_CMD_PREPARE,
    PVAENU_CMD_START,
    PVAENU_CMD_STOP,
    PVAENU_CMD_FLUSH,
    PVAENU_CMD_PAUSE,
    PVAENU_CMD_RESET,
} PVAENodeUtilCmdType;

/** Node utility command class */
class PVAENodeUtilCmd
{
    public:

        PVAENodeUtilCmd() : iType(PVAENU_CMD_NONE), iParam1(NULL), iParam2(NULL), iContext(NULL) {}
        PVAENodeUtilCmd(const PVAENodeUtilCmd& aCmd)
        {
            Copy(aCmd);
        }

        PVAENodeUtilCmd& operator=(const PVAENodeUtilCmd& aCmd)
        {
            Copy(aCmd);
            return (*this);
        }

        // For most commands
        PVMFStatus Construct(PVAENodeUtilCmdType aType, const PVAENodeContainerVector& aNodes, OsclAny* aContext)
        {
            iType = aType;
            iNodes = aNodes;
            iContext = aContext;
            return PVMFSuccess;
        }

        // For Connect
        PVMFStatus ConstructConnect(PVAENodeContainer* aMasterNode, int32 aTag1,
                                    PVAENodeContainer* aSlaveNode, int32 aTag2,
                                    const PvmfMimeString& aMimeType, OsclAny* aContext)
        {
            iType = PVAENU_CMD_CONNECT;
            iContext = aContext;
            iParam1 = (OsclAny*)aTag1;
            iParam2 = (OsclAny*)aTag2;

            int32 err = 0;
            OSCL_TRY(err,
                     iNodes.push_back(aMasterNode);
                     iNodes.push_back(aSlaveNode);
                     iMimeType = aMimeType;
                    );
            OSCL_FIRST_CATCH_ANY(err, return PVMFErrNoMemory;);

            return PVMFSuccess;
        }

        PVMFStatus ParseConnect(int32& aTag1, int32& aTag2, PvmfMimeString& aMimeType)
        {
            if (iType != PVAENU_CMD_CONNECT)
                return PVMFFailure;

            aTag1 = (int32)iParam1;
            aTag2 = (int32)iParam2;
            aMimeType = iMimeType;
            return PVMFSuccess;
        }

        PVMFStatus ConstructDisconnect(PVAENodeContainer* aNodeContainer, OsclAny* aContext)
        {
            iType = PVAENU_CMD_DISCONNECT;
            iContext = aContext;

            int32 err = 0;
            OSCL_TRY(err,
                     iNodes.push_back(aNodeContainer);
                    );
            OSCL_FIRST_CATCH_ANY(err, return PVMFErrNoMemory;);

            return PVMFSuccess;
        }

        PVMFStatus ConstructQueryUUID(PVAENodeContainer* aNodeContainer, const PvmfMimeString& aMimeType,
                                      Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                      bool aExactUuidsOnly, OsclAny* aContext)
        {
            iType = PVAENU_CMD_QUERY_UUID;
            iContext = aContext;
            iParam1 = (OsclAny*) & aUuids;
            iParam2 = (OsclAny*)aExactUuidsOnly;

            int32 err = 0;
            OSCL_TRY(err,
                     iNodes.push_back(aNodeContainer);
                     iMimeType = aMimeType;
                    );
            OSCL_FIRST_CATCH_ANY(err, return PVMFErrNoMemory;);

            return PVMFSuccess;
        }

        PVMFStatus ParseQueryUUID(Oscl_Vector<PVUuid, OsclMemAllocator>* aUuids, bool& aExactUuidsOnly)
        {
            if (iType != PVAENU_CMD_QUERY_UUID)
                return PVMFFailure;

            aUuids = (Oscl_Vector<PVUuid, OsclMemAllocator>*)iParam1;
            aExactUuidsOnly = (iParam2 != NULL); // iParam2 is a bool
            return PVMFSuccess;
        }

        PVMFStatus ConstructQueryInterface(PVAENodeContainer* aNodeContainer,
                                           const PVUuid& aUuid,
                                           PVInterface*& aInterfacePtr,
                                           OsclAny* aContext)
        {
            iType = PVAENU_CMD_QUERY_INTERFACE;
            iContext = aContext;
            iParam1 = (OsclAny*) & aInterfacePtr;
            iUuid = aUuid;

            int32 err = 0;
            OSCL_TRY(err,
                     iNodes.push_back(aNodeContainer);
                    );
            OSCL_FIRST_CATCH_ANY(err, return PVMFErrNoMemory;);

            return PVMFSuccess;
        }

        PVMFStatus ParseQueryInterface(PVInterface**& aInterfacePtr)
        {
            if (iType != PVAENU_CMD_QUERY_INTERFACE)
                return PVMFFailure;

            aInterfacePtr = (PVInterface**)iParam1;
            return PVMFSuccess;
        }

        PVMFStatus ConstructInit(PVAENodeContainer* aNodeContainer, OsclAny* aContext)
        {
            iType = PVAENU_CMD_INIT;
            iContext = aContext;

            int32 err = 0;
            OSCL_TRY(err,
                     iNodes.push_back(aNodeContainer);
                    );
            OSCL_FIRST_CATCH_ANY(err, return PVMFErrNoMemory;);

            return PVMFSuccess;
        }

        PVAENodeUtilCmdType iType;
        PVAENodeContainerVector iNodes;
        OSCL_HeapString<OsclMemAllocator> iMimeType;
        PVUuid iUuid; // For QueryInterface
        OsclAny* iParam1;
        OsclAny* iParam2;
        OsclAny* iContext;

    private:
        void Copy(const PVAENodeUtilCmd& aCmd)
        {
            iType = aCmd.iType;
            iNodes = aCmd.iNodes;
            iMimeType = aCmd.iMimeType;
            iUuid = aCmd.iUuid;
            iParam1 = aCmd.iParam1;
            iParam2 = aCmd.iParam2;
            iContext = aCmd.iContext;
        }
};

/** Observer class to listen for utility command completion events */
class PVAENodeUtilObserver
{
    public:
        virtual void NodeUtilCommandCompleted(const PVMFCmdResp& aResponse) = 0;
        virtual void NodeUtilErrorEvent(const PVMFAsyncEvent& aEvent) = 0;
        virtual ~PVAENodeUtilObserver() {}
};

/**
 * Utility class to connect two nodes and handle node commands to multiple
 * nodes. Users of this utility must provide PVMFNodeCmdStatusObserver,
 * PVMFNodeErrorEventObserver and PVMFNodeInfoEventObserver in order to
 * receive callbacks for results of command issued to this utility.
 *
 * While this utility is processing a command, it will change the nodes'
 * observers to this utility. Therefore there must be no pending commands
 * in the node before this method is called. Other users of the node must not
 * change the observers until this command is completed.  Also, no additional commands
 * should be made to these nodes until the command is completed. When the
 * command is complete, the observers of the node will be set to the observers
 * of this utility.
 */
class PVAuthorEngineNodeUtility : public OsclTimerObject,
            public PVMFNodeCmdStatusObserver
{
    public:
        PVAuthorEngineNodeUtility();
        virtual ~PVAuthorEngineNodeUtility();

        /**
         * Sets command status observer. After a utility
         * command is completed, the command status observer for the nodes
         * will be sent to the observer set here.
         *
         * @param aObserver Command status observer
         */
        void SetObserver(const PVAENodeUtilObserver& aObserver)
        {
            iObserver = (PVAENodeUtilObserver*) & aObserver;
        }

        /**
         * Connect two nodes. An output port will be requested from the specified
         * input node, and an input port will be requested from the specified output
         * node.  The two ports will then be connected to establish a connection
         * between the two nodes. This utility will callback to report status of
         * the command after the connection is complete and will return the newly
         * requested and connected ports will be added to the iInPorts or iOutPorts
         * vector in the PVAENodeContainer objects passed in.
         *
         * @param aInputNode Container of input node of the connection. The newly requested
         * port will be added to the iOutPorts vector of the node container.
         * @param aOutputNode Container of output node of the connection.  The newly requested
         * port will be added to the iInPorts vector of the node container.
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns Completion status
         */
        PVMFStatus Connect(PVAENodeContainer* aMasterNode, int32 aTag1,
                           PVAENodeContainer* aSlaveNode, int32 aTag2,
                           const PvmfMimeString& aMimeType,
                           OsclAny* aContext = NULL);

        /**
         * Disconnect and release all ports of a node.
         *
         * @param aNodeContainer Container for node which ports will be disonnected and released
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns Completion status
         */
        PVMFStatus Disconnect(PVAENodeContainer* aNodeContainer, OsclAny* aContext = NULL);

        /**
         * Query UUIDs supported by the specified node.
         *
         * @param aNodeContainer Container for node to which this query is made
         * @param aMimeType The MIME type of the desired interfaces
         * @param aUuids A vector to hold the discovered UUIDs
         * @param aExactUuidsOnly Turns on/off the retrival of UUIDs with aMimeType as a base type
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns Completion status
         */
        PVMFStatus QueryUUID(PVAENodeContainer* aNodeContainer,
                             const PvmfMimeString& aMimeType,
                             Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                             bool aExactUuidsOnly = false,
                             OsclAny* aContext = NULL);

        /**
         * Query for interface of specified Uuid from the specified node. The queried interface
         * object will be added to the iExtensions vector and stored in the interface pointer
         * provided by user.  Also, Uuid of the queried interface will be added to the
         * iExtensionsUuid vector.
         *
         * @param aNodeContainer Container for node to which this query is made
         * @param aUuid The UUID of the desired interface
         * @param aInterfacePtr Optional interface pointer to store the requested interface
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns Completion status
         */
        PVMFStatus QueryInterface(PVAENodeContainer* aNodeContainer,
                                  const PVUuid& aUuid,
                                  PVInterface*& aInterfacePtr,
                                  OsclAny* aContext = NULL);

        /**
         * Initialize a node
         *
         * @param aNode Node container containing the node to be initialized.
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns Completion status
         */
        PVMFStatus Init(const PVAENodeContainer* aNode, OsclAny* aContext = NULL);

        /**
         * Initialize all nodes in the vector.
         *
         * @param aNodes Vector of nodes to be initialized
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns Completion status
         */
        PVMFStatus Init(const PVAENodeContainerVector& aNodes, OsclAny* aContext = NULL);

        /**
         * Prepare all nodes in the vector.
         *
         * @param aNodes Vector of nodes to be prepared
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns Completion status
         */
        PVMFStatus Prepare(const PVAENodeContainerVector& aNodes, OsclAny* aContext = NULL);

        /**
         * Start all ports of all nodes in the vector.
         *
         * @param aNodes Vector of nodes to be started
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns Completion status
         */
        PVMFStatus Start(const PVAENodeContainerVector& aNodes, OsclAny* aContext = NULL);

        /**
         * Pause all ports of all nodes in the vector.
         *
         * @param aNodes Vector of nodes to be paused
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns Completion status
         */
        PVMFStatus Pause(const PVAENodeContainerVector& aNodes, OsclAny* aContext = NULL);

        /**
         * Stop all ports of all nodes in the vector.
         *
         * @param aNodes Vector of nodes to be stopped
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns Completion status
         */
        PVMFStatus Stop(const PVAENodeContainerVector& aNodes, OsclAny* aContext = NULL);

        /**
         * Flush all ports of all nodes in the vector.
         *
         * @param aNodes Vector of nodes to be flushed
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns Completion status
         */
        PVMFStatus Flush(const PVAENodeContainerVector& aNodes, OsclAny* aContext = NULL);

        /**
         * Start all ports of all nodes in the vector.
         *
         * @param aNodes Vector of nodes to be reset
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns Completion status
         */
        PVMFStatus Reset(const PVAENodeContainerVector& aNodes, OsclAny* aContext = NULL);

        /**
         * Provides the size of command queue
         *
         * @returns Number of pending commands in command queue
         */
        uint32 GetCommandQueueSize();

        // Implement pure virtual method of PVMFNodeCmdStatusObserver
        void NodeCommandCompleted(const PVMFCmdResp& aResponse);

    private:
        void Run();

        // Command queue routines
        PVMFStatus AddCmdToQueue(PVAENodeUtilCmd& aCmd);
        void CompleteUtilityCmd(const PVAENodeUtilCmd& aCmd, PVMFStatus aStatus);

        // Command processing routines
        PVMFStatus DoConnect(const PVAENodeUtilCmd& aCmd);
        PVMFStatus CompleteConnect(const PVAENodeUtilCmd& aCmd, const PVMFCmdResp& aResponse);
        PVMFStatus DoDisconnect(const PVAENodeUtilCmd& aCmd);
        PVMFStatus DoQueryUuid(const PVAENodeUtilCmd& aCmd);
        PVMFStatus DoQueryInterface(const PVAENodeUtilCmd& aCmd);
        PVMFStatus CompleteQueryInterface(const PVAENodeUtilCmd& aCmd);
        PVMFStatus DoInit(const PVAENodeUtilCmd& aCmd);
        PVMFStatus DoPrepare(const PVAENodeUtilCmd& aCmd);
        PVMFStatus DoStart(const PVAENodeUtilCmd& aCmd);
        PVMFStatus DoPause(const PVAENodeUtilCmd& aCmd);
        PVMFStatus DoStop(const PVAENodeUtilCmd& aCmd);
        PVMFStatus DoFlush(const PVAENodeUtilCmd& aCmd);
        PVMFStatus DoReset(const PVAENodeUtilCmd& aCmd);
        PVMFStatus CompleteStateTransition(const PVAENodeUtilCmd& aCmd, TPVMFNodeInterfaceState aState);
        PVMFStatus ReleasePort(PVAENodeContainer*&, PVMFPortInterface*&);
        // Observer
        PVAENodeUtilObserver* iObserver;

        // Command queue
        Oscl_Vector<PVAENodeUtilCmd, OsclMemAllocator> iCmdQueue;

        PVLogger* iLogger;
};

#endif // PVAE_NODE_UTILTIY_H_INCLUDED
