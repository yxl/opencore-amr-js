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
#ifndef PVMF_SM_FSP_BASE_IMPL_H
#define PVMF_SM_FSP_BASE_IMPL_H

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

#ifndef PVMF_DATA_SOURCE_INIT_EXTENSION_H_INCLUDED
#include "pvmf_data_source_init_extension.h"
#endif

#ifndef PVMF_TRACK_SELECTION_EXTENSION_H_INCLUDED
#include "pvmf_track_selection_extension.h"
#endif

#ifndef PVMF_DATA_SOURCE_PLAYBACK_CONTROL_H_INCLUDED
#include "pvmf_data_source_playback_control.h"
#endif

#ifndef PVMF_META_DATA_EXTENSION_H_INCLUDED
#include "pvmf_meta_data_extension.h"
#endif

#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif

#ifndef PVMF_CPMPLUGIN_LICENSE_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_license_interface.h"
#endif

#ifndef CPM_H_INCLUDED
#include "cpm.h"
#endif

#ifndef PVMF_STREAMING_DATA_SOURCE_H_INCLUDED
#include "pvmf_streaming_data_source.h"
#endif

#ifndef PVMF_SOURCE_CONTEXT_DATA_H_INCLUDED
#include "pvmf_source_context_data.h"
#endif

#ifndef PVMF_SM_FSP_BASE_CMDS_H_INCLUDED
#include "pvmf_sm_fsp_base_cmds.h"
#endif

#ifndef PVMF_SM_FSP_BASE_TYPES_H_INCLUDED
#include "pvmf_sm_fsp_base_types.h"
#endif

class JitterBufferFactory;


#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVMF_SM_FSP_BASE_METADATA_H_INCLUDED
#include "pvmf_sm_fsp_base_metadata.h"
#endif

#define PVMF_SM_FSP_BASE_LOGSTACKTRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iSMBaseLogger,PVLOGMSG_STACK_TRACE,m);
#define PVMF_SM_FSP_BASE_LOGDEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iSMBaseLogger,PVLOGMSG_DEBUG,m);
#define PVMF_SM_FSP_BASE_LOGERR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iSMBaseLogger,PVLOGMSG_ERR,m);
#define PVMF_SM_FSP_BASE_LOGCMDSEQ(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iCommandSeqLogger,PVLOGMSG_STACK_TRACE,m);

#define PVMF_SM_ERRHANDLER_LOGSTACKTRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_STACK_TRACE,m);
#define PVMF_SM_ERRHANDLER_LOGDEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_DEBUG,m);
#define PVMF_SM_ERRHANDLER_LOGERR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_ERR,m);
#define PVMF_SM_ERRHANDLER_LOGCMDSEQ(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_STACK_TRACE,m);

class PVMFSMSessionMetaDataInfo;
class PVMFSMFSPChildNodeErrorHandler;
#define PVMF_STREAMING_MANAGER_INTERNAL_CMDQ_SIZE 40

class PVMFSMFSPBaseNode : public PVMFNodeInterface,
            public OsclActiveObject,
            public PvmiCapabilityAndConfig,
            public PVMFDataSourceInitializationExtensionInterface,
            public PVMFTrackSelectionExtensionInterface,
            public PvmfDataSourcePlaybackControlInterface,
            public PVMFMetadataExtensionInterface,
            public PVMFCPMPluginLicenseInterface,
            public PVMFNodeErrorEventObserver,
            public PVMFNodeInfoEventObserver,
            public PVMFNodeCmdStatusObserver,
            public PVMFCPMStatusObserver
{
        friend class PVMFSMFSPChildNodeErrorHandler;
    public:
        OSCL_IMPORT_REF virtual ~PVMFSMFSPBaseNode();

        /* From PVMFNodeInterface */
        OSCL_IMPORT_REF PVMFStatus ThreadLogon();
        OSCL_IMPORT_REF PVMFStatus ThreadLogoff();

        //Added overload of Connect
        //Connects PVMFSMFSPBaseNode to the StreamingManagerNode
        OSCL_IMPORT_REF PVMFSessionId Connect(const PVMFNodeSession &iUpstreamSession);


        OSCL_IMPORT_REF PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        OSCL_IMPORT_REF PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);

        //Deprecated.Will cause assertion failure
        OSCL_IMPORT_REF PVMFCommandId QueryUUID(PVMFSessionId,
                                                const PvmfMimeString& aMimeType,
                                                Oscl_Vector< PVUuid, OsclMemAllocator >& aUuids,
                                                bool aExactUuidsOnly = false,
                                                const OsclAny* aContext = NULL);

        //Synchronous add-ons only for quering PVMFDataSourceInitializationExtensionInterface
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface);
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

        /* From PvmiCapabilityAndConfig */
        virtual PVMFCommandId setParametersAsync(PvmiMIOSession aSession,
                PvmiKvp* aParameters,
                int num_elements,
                PvmiKvp*& aRet_kvp,
                OsclAny* context = NULL);

        //Pure virtual(s) from following classes should be implemented in feature specific derived classes
        //(i)	 PVMFDataSourceInitializationExtensionInterface
        //(ii)   PVMFTrackSelectionExtensionInterface

        /* From PvmfDataSourcePlaybackControlInterface */
        virtual PVMFCommandId SetDataSourcePosition(PVMFSessionId aSessionId,
                PVMFTimestamp aTargetNPT,
                PVMFTimestamp& aActualNPT,
                PVMFTimestamp& aActualMediaDataTS,
                bool aSeekToSyncPoint = true,
                uint32 aStreamID = 0,
                OsclAny* aContext = NULL);

        virtual PVMFCommandId SetDataSourcePosition(PVMFSessionId aSessionId,
                PVMFDataSourcePositionParams& aPVMFDataSourcePositionParams,
                OsclAny* aContext = NULL);

        virtual PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId,
                PVMFTimestamp aTargetNPT,
                PVMFTimestamp& aActualNPT,
                bool aSeekToSyncPoint = true,
                OsclAny* aContext = NULL);
        virtual PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId,
                PVMFTimestamp aTargetNPT,
                PVMFTimestamp& aSyncBeforeTargetNPT,
                PVMFTimestamp& aSyncAfterTargetNPT,
                OsclAny* aContext = NULL,
                bool aSeekToSyncPoint = true);
        virtual PVMFCommandId SetDataSourceRate(PVMFSessionId aSessionId,
                                                int32 aRate,
                                                PVMFTimebase* aTimebase = NULL,
                                                OsclAny* aContext = NULL);
        //PvmfDataSourcePlaybackControlInterface::ComputeSkipTimeStamp is not supported
        //in all streaming formats. So not overriding
        //the base implementataion of returning PVMFErrNotSupported. FSP plugins can override
        //if they so choose.

        /* From PVMFMetadataExtensionInterface */
        virtual uint32 GetNumMetadataKeysBase(char* aQueryKeyString = NULL);
        virtual uint32 GetNumMetadataValuesBase(PVMFMetadataList& aKeyList);
        virtual PVMFCommandId GetNodeMetadataKeys(PVMFSessionId aSessionId,
                PVMFMetadataList& aKeyList,
                uint32 aStartingKeyIndex,
                int32 aMaxKeyEntries = -1,
                char* aQueryKeyString = NULL,
                const OsclAny* aContextData = NULL);
        virtual PVMFCommandId GetNodeMetadataValues(PVMFSessionId aSessionId,
                PVMFMetadataList& aKeyList,
                Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                uint32 aStartingValueIndex,
                int32 aMaxValueEntries = -1,
                const OsclAny* aContextData = NULL);
        PVMFStatus ReleaseNodeMetadataKeysBase(PVMFMetadataList& aKeyList,
                                               uint32 aStartingKeyIndex,
                                               uint32 aEndKeyIndex);
        PVMFStatus ReleaseNodeMetadataValuesBase(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                uint32 aStartingValueIndex,
                uint32 aEndValueIndex);

        /* From PVMFCPMPluginLicenseInterface */
        virtual PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                         OSCL_wString& aContentName,
                                         OSCL_wString& aLicenseURL);
        virtual PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                         OSCL_String&  aContentName,
                                         OSCL_String&  aLicenseURL);
        virtual PVMFCommandId GetLicense(PVMFSessionId aSessionId
                                         , OSCL_wString& aContentName
                                         , OsclAny* aLicenseData = NULL
                                                                   , uint32 aDataSize = 0
                                                                                        , int32 aTimeoutMsec = (-1)
                                                                                                               , OsclAny* aContext = NULL);
        virtual PVMFCommandId GetLicense(PVMFSessionId aSessionId
                                         , OSCL_String&  aContentName
                                         , OsclAny* aLicenseData = NULL
                                                                   , uint32 aDataSize = 0
                                                                                        , int32 aTimeoutMsec = (-1)
                                                                                                               , OsclAny* aContext = NULL);
        virtual PVMFCommandId CancelGetLicense(PVMFSessionId aSessionId
                                               , PVMFCommandId aCmdId
                                               , OsclAny* aContext = NULL);
        virtual PVMFStatus GetLicenseStatus(PVMFCPMLicenseStatus& aStatus);

        /* From PVMFNodeErrorEventObserver */
        virtual void HandleNodeErrorEvent(const PVMFAsyncEvent& aEvent);

        //Pure virtual(s) from following classes should be implemented in feature specific derived classes
        //(i)	 PVMFNodeInfoEventObserver
        //(ii)   PVMFNodeCmdStatusObserver

        //For streaming of protected content PVMFCPM plugin is required.
        //Response of async commands executed on PVMFCPM will be notified by callback to func CPMCommandCompleted
        virtual void CPMCommandCompleted(const PVMFCmdResp& aResponse);
    protected:
        PVMFSMFSPBaseNode(int32 aPriority);
        //Second Phase ctor for allocating mem on heap
        void Construct();

        //Pure virtuals to be implemented in the derived classes
        virtual bool ProcessCommand(PVMFSMFSPBaseNodeCommand&) = 0;	//FSP concrete implementation need to implement it.
        virtual bool IsFSPInternalCmd(PVMFCommandId aId) = 0;
        virtual void PopulateDRMInfo() = 0;
        virtual bool RequestUsageComplete() = 0;

        //For processing command Queues
        void MoveCmdToCurrentQueue(PVMFSMFSPBaseNodeCommand& aCmd);
        void MoveCmdToCancelQueue(PVMFSMFSPBaseNodeCommand& aCmd);
        void MoveErrHandlingCmdToCurErrHandlingQ(PVMFSMFSPBaseNodeCommand& aCmd);

        virtual PVMFCommandId QueueCommandL(PVMFSMFSPBaseNodeCommand& aCmd);
        PVMFCommandId QueueErrHandlingCommandL(PVMFSMFSPBaseNodeCommand& aCmd);

        //Functions for reporting error event, info event and command completion
        virtual void CommandComplete(PVMFFSPNodeCmdQ&,
                                     PVMFSMFSPBaseNodeCommand&,
                                     PVMFStatus,
                                     OsclAny* aData = NULL,
                                     PVUuid* aEventUUID = NULL,
                                     int32* aEventCode = NULL,
                                     PVInterface* aExtMsg = NULL,
                                     uint32 aEventDataLen = 0);

        void ReportErrorEvent(PVMFEventType aEventType,
                              OsclAny* aEventData = NULL,
                              PVUuid* aEventUUID = NULL,
                              int32* aEventCode = NULL);
        void ReportInfoEvent(PVMFEventType aEventType,
                             OsclAny* aEventData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);

        //Utiliy funcions for internal commands handling
        PVMFSMFSPCommandContext* RequestNewInternalCmd();
        virtual void InternalCommandComplete(PVMFFSPNodeCmdQ&,
                                             PVMFSMFSPBaseNodeCommand&,
                                             PVMFStatus,
                                             OsclAny* aData = NULL,
                                             PVUuid* aEventUUID = NULL,
                                             int32* aEventCode = NULL,
                                             PVInterface* aExtMsg = NULL);
        void ResetNodeContainerCmdState();

        //Utility funct that can be called by derived class to populate available keys based on iMetaDataInfo
        void PopulateAvailableMetadataKeys();

        //PVMFNodeInterface
        //To process Cancel command & CancelAll command
        /**
        * Assumption: When this function is called, cancel all command is present in the input Q.
        *             Cancellion of the API's will be attempted till the point CancelAllCommand is issued.
        *			  No attempt will be made to cancel any of the async command that are queued after
        *			  making call to CancelAllCommand.
        * Command completion status values:
        *			  PVMFErrNoMemory, PVMFSuccess, PVMFFailure
        */
        virtual void DoCancelAllCommands(PVMFSMFSPBaseNodeCommand&);
        void DoCancelAllPendingCommands(PVMFSMFSPBaseNodeCommand& aCmd);
        void DoResetDueToErr(PVMFSMFSPBaseNodeCommand& aCmd);

        /**
        * Assumption: When this function is called, cancel all command is present in the input Q.
        *             Cancellion of the API's will be attempted till the point CancelAllCommand is issued.
        *			  No attempt will be made to cancel any of the async command that are queued after
        *			  making call to CancelAllCommand.
        * Command completion status values:
        *			  PVMFErrNoMemory, PVMFSuccess, PVMFFailure
        */
        virtual void DoCancelCommand(PVMFSMFSPBaseNodeCommand&);

        //Functions used to check if cancelcommand/cancelallcommand is complete
        virtual void CompleteChildNodesCmdCancellation();
        virtual bool CheckChildrenNodesCancelAll();

        //To process Reset command
        virtual void DoReset(PVMFSMFSPBaseNodeCommand&);
        virtual void CompleteReset();
        virtual bool CheckChildrenNodesReset();
        void CompleteResetDueToErr();

        //To process Flush command
        virtual void DoFlush(PVMFSMFSPBaseNodeCommand&);
        virtual void CompleteFlush();
        virtual bool CheckChildrenNodesFlush();
        virtual bool FlushPending();

        //PVMFMetadataExtensionInterface
        PVMFStatus DoGetMetadataKeysBase(PVMFSMFSPBaseNodeCommand& aCmd);
        PVMFStatus CompleteGetMetadataKeys(PVMFSMFSPBaseNodeCommand& aCmd);
        PVMFStatus DoGetMetadataValuesBase(PVMFSMFSPBaseNodeCommand& aCmd);


        PVMFStatus DoGetLicense(PVMFSMFSPBaseNodeCommand& aCmd,
                                bool aWideCharVersion = false);
        void DoCancelGetLicense(PVMFSMFSPBaseNodeCommand& aCmd);

        void CompleteGetLicense();

        //CPM related functions
        void InitCPM();
        void OpenCPMSession();
        void CPMRegisterContent();
        bool GetCPMContentAccessFactory();
        bool GetCPMMetaDataExtensionInterface();
        void GetCPMLicenseInterface();
        void GetCPMCapConfigInterface();
        bool SetCPMKvps();
        void RequestUsage();
        void SendUsageComplete();
        void CloseCPMSession();
        void ResetCPM();
        void GetCPMMetaDataKeys();
        void GetCPMMetaDataValues();
        void CompleteGetMetaDataValues();
        void CompleteDRMInit();

        PVMFStatus CheckCPMCommandCompleteStatus(PVMFCommandId aID, PVMFStatus aStatus);

        void GetActualMediaTSAfterSeek();
        virtual bool IsFatalErrorEvent(const PVMFEventType& event);
        PVMFSMFSPChildNodeContainer* getChildNodeContainer(int32 tag);
        virtual void ResetNodeParams(bool aReleaseMemmory = true);

        uint32	iNoOfValuesIteratedForValueVect;
        uint32	iNoOfValuesPushedInValueVect;

        PVMFNodeCapability iCapability;

        bool iRepositioning;
        PVMFTimestamp iRepositionRequestedStartNPTInMS;
        PVMFTimestamp iActualRepositionStartNPTInMS;
        PVMFTimestamp* iActualRepositionStartNPTInMSPtr;
        PVMFTimestamp iActualMediaDataTS;
        PVMFTimestamp* iActualMediaDataTSPtr;
        bool iJumpToIFrame;


        /* Session start & stop times */
        uint32 iSessionStartTime;
        uint32 iSessionStopTime;
        bool   iSessionStopTimeAvailable;
        bool   iSessionSeekAvailable;

        bool iPlaylistPlayInProgress;

        PVMFDataSourcePositionParams* iPVMFDataSourcePositionParamsPtr;
        uint32 iStreamID;
        bool iPlayListRepositioning;
        bool iPlayListRepositioningSupported;
        bool iGraphConstructComplete;
        bool iGraphConnectComplete;
        uint32 iNumRequestPortsPending;
        uint32 iTotalNumRequestPortsComplete;
        //Filled on completion of init
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iAvailableMetadataKeys;
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iCPMMetadataKeys;
        PVMFSMSessionMetaDataInfo* iMetaDataInfo;

        PVMFStatus GetIndexParamValues(char* aString, uint32& aStartIndex, uint32& aEndIndex);
        PVMFStatus GetMaxSizeValue(char* aString, uint32& aMaxSize);
        PVMFStatus GetTruncateFlagValue(char* aString, uint32& aTruncateFlag);

        JitterBufferFactory* iJBFactory;

        //CPM related
        bool iPreviewMode;
        bool iUseCPMPluginRegistry;
        bool iDRMResetPending;
        bool iCPMInitPending;
        uint32 maxPacketSize;
        uint32 iPVMFStreamingManagerNodeMetadataValueCount;
        PVMFStreamingDataSource iCPMSourceData;
        PVMFSourceContextData iSourceContextData;
        bool iSourceContextDataValid;
        PVMFCPM* iCPM;
        PVMFSessionId iCPMSessionID;
        PVMFCPMContentType iCPMContentType;
        PVMFCPMPluginAccessInterfaceFactory* iCPMContentAccessFactory;
        PVMFCPMPluginAccessUnitDecryptionInterface* iDecryptionInterface;
        PVMFCPMPluginLicenseInterface* iCPMLicenseInterface;
        PVInterface* iCPMLicenseInterfacePVI;
        PvmiCapabilityAndConfig* iCPMCapConfigInterface;
        PVInterface* iCPMCapConfigInterfacePVI;
        PVMFSMNodeKVPStore iCPMKvpStore;
        PvmiKvp iRequestedUsage;
        PvmiKvp iApprovedUsage;
        PvmiKvp iAuthorizationDataKvp;
        PVMFCPMUsageID iUsageID;
        PVMFCommandId iCPMInitCmdId;
        PVMFCommandId iCPMOpenSessionCmdId;
        PVMFCommandId iCPMRegisterContentCmdId;
        PVMFCommandId iCPMRequestUsageId;
        PVMFCommandId iCPMUsageCompleteCmdId;
        PVMFCommandId iCPMCloseSessionCmdId;
        PVMFCommandId iCPMResetCmdId;
        PVMFCommandId iCPMGetMetaDataKeysCmdId;
        PVMFCommandId iCPMGetMetaDataValuesCmdId;
        PVMFCommandId iCPMGetLicenseInterfaceCmdId;
        PVMFCommandId iCPMGetLicenseCmdId;
        PVMFCommandId iCPMGetCapConfigCmdId;
        PVMFCommandId iCPMCancelGetLicenseCmdId;
        PVMFStatus iCPMRequestUsageCommandStatus;

        PVMFFSPNodeCmdQ iInputCommands;
        PVMFFSPNodeCmdQ iCurrentCommand;
        PVMFFSPNodeCmdQ iCancelCommand;
        PVMFFSPNodeCmdQ iErrHandlingCommandQ;
        PVMFFSPNodeCmdQ iCurrErrHandlingCommand;

        PVMFSMFSPCommandContext iInternalCmdPool[PVMF_STREAMING_MANAGER_INTERNAL_CMDQ_SIZE];

        PVMFSMFSPChildNodeContainerVector  iFSPChildNodeContainerVec;
        PVMFSMFSPSessionSourceInfo* iSessionSourceInfo;

        PVLogger *iCommandSeqLogger;
        PVLogger *iLogger;
        PVLogger * iSMBaseLogger;

        //For Error handling
        void HandleError(const PVMFCmdResp& aResponse);
        void ErrHandlingComplete(const PVMFSMFSPBaseNodeCommand* aErroneousCmd = NULL);

        //For pushing data to vect
        PVMFStatus PushKeyToMetadataList(PVMFMetadataList* aMetaDataListPtr, const OSCL_HeapString<OsclMemAllocator> & aKey)const;
        PVMFStatus PushKVPToMetadataValueList(Oscl_Vector<PvmiKvp, OsclMemAllocator>* aValueList, const PvmiKvp& aKVP)const;
        PVMFStatus SetCPMKvp(PvmiKvp& aKVP);
        void CleanUp();
        uint8* GetMemoryChunk(OsclMemAllocDestructDealloc<uint8>& aAllocator, const uint32 aChunkSize)
        {
            int32 leaveCode = 0;
            uint8* memChunk = NULL;
            OSCL_TRY(leaveCode,
                     memChunk = OSCL_STATIC_CAST(uint8*, aAllocator.ALLOCATE(aChunkSize)));
            if (leaveCode != OsclErrNone)
            {
                PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode::GetMemoryChunk - Error Memory Allocation failed"));
            }
            return memChunk;
        }

        bool SupressInfoEvent();


    private:
        PVMFSMFSPChildNodeErrorHandler* iChildNodeErrHandler;
        OSCL_IMPORT_REF PVMFSessionId Connect(const PVMFNodeSessionInfo &aSessionInfo);
        /* From OsclActiveObject */
        void Run();
        void DoCancel();

        void CleanupCPMdata();
        void CreateCommandQueues();
        virtual bool IsInternalCmd(PVMFCommandId aId);

        void ResetCPMParams(bool aReleaseMem = true);
        bool ErrorHandlingRequired(PVMFStatus aStatus);

        //CPM related
        PVMFMetadataExtensionInterface* iCPMMetaDataExtensionInterface;
};

//This class does error handling only for the errors issued by child nodes of SM node.
//For handling errors generated by CPM plugin we rely on engine to do err handling.
class PVMFSMFSPChildNodeErrorHandler
{
    public:
        PVMFSMFSPChildNodeErrorHandler(PVMFSMFSPBaseNode * aFSPBaseNode): iCmdResponse(NULL)
                , iAsyncEvent(NULL)
                , iErroneousCmdResponse(NULL)
                , iErrCmd(NULL)
                , iErrSource(SMFSP_ERR_SOURCE_INDETERMINATE)
                , iState(SMFSP_ERRHANDLER_IDLE)
                , iSMFSPNode(aFSPBaseNode)
                , iLogger(NULL)
        {

        }

        void Construct()
        {
            iLogger = PVLogger::GetLoggerObject("PVMFSMFSPChildNodeErrorHandler");
        }
        static PVMFSMFSPChildNodeErrorHandler* CreateErrHandler(PVMFSMFSPBaseNode* aFSPBaseNode);
        static void DeleteErrHandler(PVMFSMFSPChildNodeErrorHandler*& aErrHandler);
        void InitiateErrorHandling(const PVMFCmdResp& aCmdResponse);
        void InitiateErrorHandling(const PVMFAsyncEvent& aAsyncEvent);
        void CompleteErrorHandling(const PVMFCmdResp& aResponse);
        bool IsErrorHandlingComplete() const
        {
            return (SMFSP_ERRHANDLER_IDLE == iState);
        }
        void Reset();
        const PVMFCmdResp* GetErroneousCmdResponse();
        const PVMFAsyncEvent* GetAsyncErrEvent();
        void ErrHandlingCommandComplete(PVMFFSPNodeCmdQ&,
                                        PVMFSMFSPBaseNodeCommand&,
                                        PVMFStatus,
                                        OsclAny* aData = NULL,
                                        PVUuid* aEventUUID = NULL,
                                        int32* aEventCode = NULL,
                                        PVInterface* aExtMsg = NULL);
    private:
        void SaveErrorInfo(const PVMFCmdResp& aCmdResponse);
        void SaveErrorInfo(const PVMFAsyncEvent& aAsyncEvent);
        void PerformErrorHandling();
        void ContinueChildNodesCmdCancellation();
        void CompleteChildNodesCmdCancellationDueToErr();
        void CompleteChildNodesResetDueToError();

        enum SMFSPErrorSource
        {
            SMFSP_ERR_SOURCE_INDETERMINATE,
            SMFSP_ERR_SOURCE_EVENT,
            SMFSP_ERR_SOURCE_NODE_CMD_COMPLETION
        };

        enum SMFSPChildNodeErrorHandlerState
        {
            SMFSP_ERRHANDLER_IDLE,
            SMFSP_ERRHANDLER_WAITING_FOR_CANCEL_COMPLETION,	//when er occurs for cancel command we do not queue cancel due to err
            SMFSP_ERRHANDLER_WAITING_FOR_CANCEL_DUE_TO_ERR_COMPLETION,
            SMFSP_ERRHANDLER_WAITING_FOR_RESET_DUE_TO_ERR_COMPLETION
        };

        PVMFCmdResp *iCmdResponse;
        PVMFAsyncEvent *iAsyncEvent;
        PVMFCmdResp *iErroneousCmdResponse;
        PVMFSMFSPBaseNodeCommand* iErrCmd;
        SMFSPErrorSource iErrSource;
        SMFSPChildNodeErrorHandlerState iState;
        PVMFSMFSPBaseNode * iSMFSPNode;
        PVLogger *iLogger;
};
#endif
