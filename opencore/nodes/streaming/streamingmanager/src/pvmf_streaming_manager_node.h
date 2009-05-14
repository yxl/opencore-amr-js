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
#ifndef PVMF_STREAMING_MANAGER_NODE_H_INCLUDED
#define PVMF_STREAMING_MANAGER_NODE_H_INCLUDED

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

#ifndef PVMF_DATA_SOURCE_INIT_EXTENSION_H_INCLUDED
#include "pvmf_data_source_init_extension.h"
#endif

#ifndef PVMF_SM_FSP_BASE_IMPL_H
#include "pvmf_sm_fsp_base_impl.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

class PVMFSMFSPRegistry;

//Logging macros
#define PVMF_SM_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define PVMF_SM_LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_NOTICE,m);
#define PVMF_SM_LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_SM_LOGSTACKTRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_STACK_TRACE,m);

typedef PVMFGenericNodeCommand <OsclMemAllocator> PVMFStreamingManagerNodeCommand;
typedef PVMFNodeCommandQueue<PVMFStreamingManagerNodeCommand, OsclMemAllocator> PVMFStreamingManagerNodeCmdQ;

/* Default vector reserve size */
#define PVMF_STREAMING_MANAGER_NODE_COMMAND_ID_START	9000
#define PVMF_STREAMING_MANAGER_VECTOR_RESERVE 10

///////////////////////////////////////////////
//IDs for all of the asynchronous node commands.
///////////////////////////////////////////////
enum TPVMFStreamingManagerNodeCommand
{
    PVMF_STREAMING_MANAGER_NODE_QUERYINTERFACE = PVMF_GENERIC_NODE_QUERYINTERFACE,
    PVMF_STREAMING_MANAGER_NODE_RESET = PVMF_GENERIC_NODE_RESET,
    PVMF_STREAMING_MANAGER_NODE_CANCELALLCOMMANDS = PVMF_GENERIC_NODE_CANCELALLCOMMANDS,
    PVMF_STREAMING_MANAGER_NODE_CANCELCOMMAND = PVMF_GENERIC_NODE_CANCELCOMMAND,
    PVMF_STREAMING_MANAGER_NODE_COMMAND_LAST
};

class PVMFStreamingManagerNode : public PVMFNodeInterface,
            public OsclActiveObject,
            public PVMFDataSourceInitializationExtensionInterface
{
    public:
        OSCL_IMPORT_REF static PVMFStreamingManagerNode* New(int32 aPriority);
        OSCL_IMPORT_REF virtual ~PVMFStreamingManagerNode();

        /* From PVMFNodeInterface */
        OSCL_IMPORT_REF PVMFStatus ThreadLogon();
        OSCL_IMPORT_REF PVMFStatus ThreadLogoff();
        virtual PVMFSessionId Connect(const PVMFNodeSessionInfo &aSession);
        virtual PVMFStatus Disconnect(PVMFSessionId aSessionId);
        virtual TPVMFNodeInterfaceState GetState()
        {
            if (iSMFSPlugin)
                return iSMFSPlugin->GetState();
            return PVMFNodeInterface::GetState();
        }
        OSCL_IMPORT_REF PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        OSCL_IMPORT_REF PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryUUID(PVMFSessionId,
                                                const PvmfMimeString& aMimeType,
                                                Oscl_Vector< PVUuid, OsclMemAllocator >& aUuids,
                                                bool aExactUuidsOnly = false,
                                                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryInterface(PVMFSessionId, const PVUuid& aUuid,
                PVInterface*& aInterfacePtr,
                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId RequestPort(PVMFSessionId,
                int32 aPortTag,
                const PvmfMimeString* aPortConfig = NULL,
                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId ReleasePort(PVMFSessionId,
                PVMFPortInterface& aPort,
                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Init(PVMFSessionId,
                                           const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Prepare(PVMFSessionId,
                                              const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Start(PVMFSessionId,
                                            const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Stop(PVMFSessionId,
                                           const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Flush(PVMFSessionId,
                                            const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Pause(PVMFSessionId,
                                            const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Reset(PVMFSessionId,
                                            const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelAllCommands(PVMFSessionId,
                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelCommand(PVMFSessionId,
                PVMFCommandId aCmdId,
                const OsclAny* aContextData = NULL);

        /* From PVMFPortActivityHandler */
        void HandlePortActivity(const PVMFPortActivity& aActivity);

        /* From PVMFDataSourceInitializationExtensionInterface */
        virtual PVMFStatus SetSourceInitializationData(OSCL_wString& aSourceURL,
                PVMFFormatType& aSourceFormat,
                OsclAny* aSourceData);
        virtual PVMFStatus SetClientPlayBackClock(PVMFMediaClock* aClientClock);
        virtual PVMFStatus SetEstimatedServerClock(PVMFMediaClock* aClientClock);
    private:
        /* From OsclActiveObject */
        void Run();
        void DoCancel();
        void Construct();
        void addRef();
        void removeRef();

        OSCL_IMPORT_REF PVMFStreamingManagerNode(int32 aPriority);
        bool ProcessCommand(PVMFStreamingManagerNodeCommand&);
        PVMFCommandId QueueCommandL(PVMFStreamingManagerNodeCommand&);

        bool queryInterface(const PVUuid& uuid, PVInterface*& iface);
        void DoQueryInterface(PVMFStreamingManagerNodeCommand&);
        bool DoCancelAllCommands(PVMFStreamingManagerNodeCommand& aCmdQ);
        void CommandComplete(PVMFStreamingManagerNodeCmdQ& aCmdQ,
                             PVMFStreamingManagerNodeCommand& aCmd,
                             PVMFStatus aStatus,
                             OsclAny* aEventData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);

        PVMFStreamingManagerNodeCmdQ iInputCommands;

        PVMFDataSourceInitializationExtensionInterface* iFSPDataSourceInitializationIntf;
        PVMFSMFSPRegistry* iSMFSPRegistry;
        PVMFSMFSPBaseNode* iSMFSPlugin;
        PVUuid			iFSPUuid;
        PVMFNodeSession iUpstreamSession;
        PVLogger * iLogger;
};

#endif


