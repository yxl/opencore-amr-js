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
#ifndef PVMF_DOWNLOADMANAGER_NODE_H_INCLUDED
#define PVMF_DOWNLOADMANAGER_NODE_H_INCLUDED


#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif
#ifndef PVMF_DOWNLOADMANAGER_DEFS_H_INCLUDED
#include "pvmf_downloadmanager_defs.h"
#endif
#ifndef PVMF_DATA_SOURCE_PLAYBACK_CONTROL_H_INCLUDED
#include "pvmf_data_source_playback_control.h"
#endif
#ifndef PVMF_META_DATA_EXTENSION_H_INCLUDED
#include "pvmf_meta_data_extension.h"
#endif
#ifndef PVMF_DATA_SOURCE_INIT_EXTENSION_H_INCLUDED
#include "pvmf_data_source_init_extension.h"
#endif
#ifndef PVMF_TRACK_SELECTION_EXTENSION_H_INCLUDED
#include "pvmf_track_selection_extension.h"
#endif
#ifndef PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED
#include "pvmf_media_presentation_info.h"
#endif
#ifndef PVMF_DOWNLOAD_PROGRESS_INTERFACE_H_INCLUDED
#include "pvmf_download_progress_interface.h"
#endif
#ifndef PVMF_FF_PROGDOWNLOAD_SUPPORT_EXTENSION_H_INCLUDED
#include "pvmf_format_progdownload_support_extension.h"
#endif
#ifndef PVMFPROTOCOLENGINENODE_EXTENSION_H_INCLUDED
#include "pvmf_protocol_engine_node_extension.h"
#endif
#ifndef  PVMF_FILEBUFFERDATASTREAM_FACTORY_H_INCLUDED
#include "pvmf_filebufferdatastream_factory.h"
#endif
#ifndef PVMF_SOURCE_CONTEXT_DATA_H_INCLUDED
#include "pvmf_source_context_data.h"
#endif
#ifndef PVMF_RECOGNIZER_REGISTRY_H_INCLUDED
#include "pvmf_recognizer_registry.h"
#endif
#ifndef PV_PLAYER_NODE_REGISTRY_INTERFACE_H_INCLUDED
#include "pv_player_node_registry_interface.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif

// only include pvmf_downloadmanager_config.h if CML2 is NOT being used
#ifndef USE_CML2_CONFIG
#ifndef PVMF_DOWNLOADMANAGER_CONFIG_H_INCLUDED
#include "pvmf_downloadmanager_config.h"
#endif
#endif

#if(PVMF_DOWNLOADMANAGER_SUPPORT_PPB)
#ifndef  PVMF_MEMORYBUFFERDATASTREAM_FACTORY_H_INCLUDED
#include "pvmf_memorybufferdatastream_factory.h"
#endif
#endif //PVMF_DOWNLOADMANAGER_SUPPORT_PPB

#ifndef PVMF_CPMPLUGIN_LICENSE_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_license_interface.h"
#endif

/**
* Node command handling
*/

typedef PVMFGenericNodeCommand<OsclMemAllocator> PVMFDownloadManagerNodeCommandBase;

enum PVMFDownloadManagerNodeCommandType
{
    PVDLM_NODE_CMD_SETDATASOURCEPOSITION = PVMF_GENERIC_NODE_COMMAND_LAST
    , PVDLM_NODE_CMD_QUERYDATASOURCEPOSITION
    , PVDLM_NODE_CMD_SETDATASOURCERATE
    , PVDLM_NODE_CMD_GETNODEMETADATAKEY
    , PVDLM_NODE_CMD_GETNODEMETADATAVALUE
    , PVDLM_NODE_CMD_GET_LICENSE
    , PVDLM_NODE_CMD_GET_LICENSE_W
    , PVDLM_NODE_CMD_CANCEL_GET_LICENSE
};

class PVMFDownloadManagerNodeCommand : public PVMFDownloadManagerNodeCommandBase
{
    public:
        //override the default implementation of "hipri" and add the "cancel get license"
        //command to the list of hi-priority commands.
        bool hipri()
        {
            return PVMFDownloadManagerNodeCommandBase::hipri()
                   || iCmd == PVDLM_NODE_CMD_CANCEL_GET_LICENSE;
        }

        // Constructor and parser for GetNodeMetadataKey
        void Construct(PVMFSessionId s, int32 cmd
                       , PVMFMetadataList& aKeyList
                       , uint32 aStartingIndex
                       , int32 aMaxEntries
                       , char* aQueryKey
                       , const OsclAny* aContext)
        {
            PVMFDownloadManagerNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aKeyList;
            iParam2 = (OsclAny*)aStartingIndex;
            iParam3 = (OsclAny*)aMaxEntries;
            if (aQueryKey)
            {
                //allocate a copy of the query key string.
                Oscl_TAlloc<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> str;
                iParam4 = str.ALLOC_AND_CONSTRUCT(aQueryKey);
            }
        }
        void Parse(PVMFMetadataList*& MetaDataListPtr, uint32 &aStartingIndex, int32 &aMaxEntries, char*& aQueryKey)
        {
            MetaDataListPtr = (PVMFMetadataList*)iParam1;
            aStartingIndex = (uint32)iParam2;
            aMaxEntries = (int32)iParam3;
            aQueryKey = NULL;
            if (iParam4)
            {
                OSCL_HeapString<OsclMemAllocator>* keystring = (OSCL_HeapString<OsclMemAllocator>*)iParam4;
                aQueryKey = keystring->get_str();
            }
        }

        // Constructor and parser for GetNodeMetadataValue
        void Construct(PVMFSessionId s, int32 cmd, PVMFMetadataList& aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 aStartIndex, int32 aMaxEntries, const OsclAny* aContext)
        {
            PVMFDownloadManagerNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aKeyList;
            iParam2 = (OsclAny*) & aValueList;
            iParam3 = (OsclAny*)aStartIndex;
            iParam4 = (OsclAny*)aMaxEntries;

        }
        void Parse(PVMFMetadataList* &aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>* &aValueList, uint32 &aStartingIndex, int32 &aMaxEntries)
        {
            aKeyList = (PVMFMetadataList*)iParam1;
            aValueList = (Oscl_Vector<PvmiKvp, OsclMemAllocator>*)iParam2;
            aStartingIndex = (uint32)iParam3;
            aMaxEntries = (int32)iParam4;
        }

        // Constructor and parser for SetDataSourcePosition
        void Construct(PVMFSessionId s, int32 cmd, PVMFTimestamp aTargetNPT, PVMFTimestamp& aActualNPT, PVMFTimestamp& aActualMediaDataTS,
                       bool aSeekToSyncPoint, uint32 aStreamID, const OsclAny*aContext)
        {
            PVMFDownloadManagerNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aTargetNPT;
            iParam2 = (OsclAny*) & aActualNPT;
            iParam3 = (OsclAny*) & aActualMediaDataTS;
            iParam4 = (OsclAny*)aSeekToSyncPoint;
            iParam5 = (OsclAny*)aStreamID;
        }
        void Parse(PVMFTimestamp& aTargetNPT, PVMFTimestamp*& aActualNPT, PVMFTimestamp*& aActualMediaDataTS, bool& aSeekToSyncPoint, uint32& aStreamID)
        {
            aTargetNPT = (PVMFTimestamp)iParam1;
            aActualNPT = (PVMFTimestamp*)iParam2;
            aActualMediaDataTS = (PVMFTimestamp*)iParam3;
            aSeekToSyncPoint = (iParam4) ? true : false;
            aStreamID = (uint32)iParam5;
        }

        // Constructor and parser for QueryDataSourcePosition
        void Construct(PVMFSessionId s, int32 cmd, PVMFTimestamp aTargetNPT, PVMFTimestamp& aActualNPT,
                       bool aSeekToSyncPoint, const OsclAny*aContext)
        {
            PVMFDownloadManagerNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aTargetNPT;
            iParam2 = (OsclAny*) & aActualNPT;
            iParam3 = (OsclAny*)aSeekToSyncPoint;
            iParam4 = NULL;
            iParam5 = NULL;
        }
        void Parse(PVMFTimestamp& aTargetNPT, PVMFTimestamp*& aActualNPT, bool& aSeekToSyncPoint)
        {
            aTargetNPT = (PVMFTimestamp)iParam1;
            aActualNPT = (PVMFTimestamp*)iParam2;
            aSeekToSyncPoint = (iParam3) ? true : false;
        }

        // Constructor and parser for SetDataSourceRate
        void Construct(PVMFSessionId s, int32 cmd, int32 aRate, PVMFTimebase* aTimebase, const OsclAny*aContext)
        {
            PVMFDownloadManagerNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aRate;
            iParam2 = (OsclAny*)aTimebase;
            iParam3 = NULL;
            iParam4 = NULL;
            iParam5 = NULL;
        }
        void Parse(int32& aRate, PVMFTimebase*& aTimebase)
        {
            aRate = (int32)iParam1;
            aTimebase = (PVMFTimebase*)iParam2;
        }

        //need to overload the base Destroy routine to cleanup metadata key.
        void Destroy()
        {
            PVMFGenericNodeCommand<OsclMemAllocator>::Destroy();
            switch (iCmd)
            {
                case PVDLM_NODE_CMD_GETNODEMETADATAKEY:
                    if (iParam4)
                    {
                        //cleanup the allocated string
                        Oscl_TAlloc<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> str;
                        str.destruct_and_dealloc(iParam4);
                    }
                    break;
                default:
                    break;
            }
        }

        //need to overlaod the base Copy routine to copy metadata key.
        void Copy(const PVMFGenericNodeCommand<OsclMemAllocator>& aCmd)
        {
            PVMFGenericNodeCommand<OsclMemAllocator>::Copy(aCmd);
            switch (aCmd.iCmd)
            {
                case PVDLM_NODE_CMD_GETNODEMETADATAKEY:
                    if (aCmd.iParam4)
                    {
                        //copy the allocated string
                        OSCL_HeapString<OsclMemAllocator>* aStr = (OSCL_HeapString<OsclMemAllocator>*)aCmd.iParam4;
                        Oscl_TAlloc<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> str;
                        iParam4 = str.ALLOC_AND_CONSTRUCT(*aStr);
                    }
                    break;
                default:
                    break;
            }
        }
        /* Constructor and parser for GetLicenseW */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       OSCL_wString& aContentName,
                       OsclAny* aLicenseData,
                       uint32 aDataSize,
                       int32 aTimeoutMsec,
                       OsclAny* aContext)
        {
            PVMFDownloadManagerNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aContentName;
            iParam2 = (OsclAny*)aLicenseData;
            iParam3 = (OsclAny*)aDataSize;
            iParam4 = (OsclAny*)aTimeoutMsec;
            iParam5 = NULL;
        }
        void Parse(OSCL_wString*& aContentName,
                   OsclAny*& aLicenseData,
                   uint32& aDataSize,
                   int32& aTimeoutMsec)
        {
            aContentName = (OSCL_wString*)iParam1;
            aLicenseData = (PVMFTimestamp*)iParam2;
            aDataSize = (uint32)iParam3;
            aTimeoutMsec = (int32)iParam4;
        }

        /* Constructor and parser for GetLicense */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       OSCL_String& aContentName,
                       OsclAny* aLicenseData,
                       uint32 aDataSize,
                       int32 aTimeoutMsec,
                       OsclAny* aContext)
        {
            PVMFDownloadManagerNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aContentName;
            iParam2 = (OsclAny*)aLicenseData;
            iParam3 = (OsclAny*)aDataSize;
            iParam4 = (OsclAny*)aTimeoutMsec;
            iParam5 = NULL;
        }
        void Parse(OSCL_String*& aContentName,
                   OsclAny*& aLicenseData,
                   uint32& aDataSize,
                   int32& aTimeoutMsec)
        {
            aContentName = (OSCL_String*)iParam1;
            aLicenseData = (PVMFTimestamp*)iParam2;
            aDataSize = (uint32)iParam3;
            aTimeoutMsec = (int32)iParam4;
        }

};


typedef PVMFNodeCommandQueue<PVMFDownloadManagerNodeCommand, OsclMemAllocator> PVMFDownloadManagerNodeCmdQueue;

class PVMFDownloadManagerNode;
class PVLogger;


/**
* Base for Containers for the protocol engine, socket, and parser nodes and CPM object
*/
class PVMFDownloadManagerSubNodeContainerBase
{
    public:
        PVMFDownloadManagerSubNodeContainerBase();

        /*
         *  EFormatParser indicates the format parser node
         *  EProtocolEngine indicates the protocol engine node
         *  ESocket indicates the socket node
         *  ECPM is the CPM container
         */
        enum NodeType {EFormatParser, EProtocolEngine, ESocket, ECPM, ERecognizer};

        void Construct(NodeType t, PVMFDownloadManagerNode* container);

        // Each subnode has a pointer to its container, which is the DLMGR node
        PVMFDownloadManagerNode* iContainer;

        NodeType iType;
        PVMFSessionId iSessionId;

        //Node Command processing
        PVMFCommandId iCmdId;

        //License Command Processing
        PVMFCommandId iCPMGetLicenseCmdId;
        PVMFCommandId iCPMCancelGetLicenseCmdId;

        enum CmdState
        {
            EIdle,   // no command
            EBusy    // command issued to the sub-node, completion pending.
        };

        CmdState iCmdState;

        enum CmdType
        {
            // common commands
            ECleanup = 0
            // node commands
            , EQueryDataSourceInit = 1
            , EQueryTrackSelection = 2
            , EQueryMetadata = 3
            , EQueryDataSourcePlayback = 4
            , EQueryFFProgDownload = 5
            , EQueryDownloadProgress = 6
            , EQueryProtocolEngine = 7
            , EQueryDatastreamUser = 8
            , EInit = 9
            , ERequestPort = 10
            , EReleasePort = 11
            , EPrepare = 12
            , EStop = 13
            , EStart = 14
            , EPause = 15
            , EFlush = 16
            , EReset = 17
            , EGetMetadataKey = 18
            , EGetMetadataValue = 19
            , ESetFFProgDownloadSupport = 20
            , ESetDataSourcePosition = 21
            , EQueryDataSourcePosition = 22
            , EParserCreate = 23
            //Recognizer module commands
            , ERecognizerStart = 24
            , ERecognizerClose = 25
            //License commands.
            , ECPMQueryLicenseInterface = 26
            , ECPMGetLicense = 27
            , ECPMGetLicenseW = 28
            , ECPMCancelGetLicense = 29
        };

        int32 iCmd;

        // The pure virtual method IssueCommand is called on a subnode container to request that the passed
        // command be issued to the contained node (or object).
        virtual PVMFStatus IssueCommand(int32) = 0;

        // CommandDone is a base class method, which is called by the subnode container when a command that was issued completes.
        // In this method we handle removing the completed subnode command from the subnode command vector, and then we call Run
        // on our container (the DLMGR) so that the next subnode command will be run. If we see that there are no more subnode
        // commands in the subnode command vector, we instead call CommandComplete on the DLMGR.
        void CommandDone(PVMFStatus, PVInterface*, OsclAny*);

        //for canceling commands.
        virtual bool CancelPendingCommand() = 0;
        PVMFCommandId iCancelCmdId;
        CmdState iCancelCmdState;
        void CancelCommandDone(PVMFStatus, PVInterface*, OsclAny*);

        bool CmdPending()
        {
            return iCmdState != EIdle || iCancelCmdState != EIdle;
        }
};

/*
* Containers for the protocol, socket, and parser nodes (but not the cpm object and not the recognizer)
*/
class PVMFDownloadManagerSubNodeContainer
            : public PVMFDownloadManagerSubNodeContainerBase
            , public PVMFNodeErrorEventObserver
            , public PVMFNodeInfoEventObserver
            , public PVMFNodeCmdStatusObserver
{
    public:
        PVMFDownloadManagerSubNodeContainer()
        {
            iDataSourceInit = NULL;
            iProtocolEngineExtensionInt = NULL;
            iDataSourcePlayback = NULL;
            iTrackSelection = NULL;
            iDatastreamUser = NULL;
            iMetadata = NULL;
            iFormatProgDownloadSupport = NULL;
            iDownloadProgress = NULL;
            iNode = NULL;
            iLicenseInterface = NULL;
        }

        ~PVMFDownloadManagerSubNodeContainer()
        {
            Cleanup();
        }

        void Cleanup();

        // pure virtuals from PVMFDownloadManagerSubNodeContainerBase
        PVMFStatus IssueCommand(int32);
        bool CancelPendingCommand();

        // node
        PVMFNodeInterface *iNode;
        void Connect();

        // Node data-- pointers to retrieved interfaces.
        PVInterface* iDataSourceInit;
        PVInterface* iProtocolEngineExtensionInt;
        PVInterface* iTrackSelection;
        PVInterface* iMetadata;
        PVInterface* iFormatProgDownloadSupport;  // The support interface provided by the format parser node
        PVInterface* iDownloadProgress;
        PVInterface* iDataSourcePlayback;
        PVInterface* iDatastreamUser;
        PVInterface* iLicenseInterface;

        PVMFDataSourceInitializationExtensionInterface *DataSourceInit()
        {
            return (PVMFDataSourceInitializationExtensionInterface*)iDataSourceInit;
        }
        PVMFProtocolEngineNodeExtensionInterface* ProtocolEngineExtension()
        {
            return (PVMFProtocolEngineNodeExtensionInterface*)iProtocolEngineExtensionInt;
        }
        PVMFTrackSelectionExtensionInterface* TrackSelection()
        {
            return (PVMFTrackSelectionExtensionInterface*)iTrackSelection;
        }
        PVMIDatastreamuserInterface* DatastreamUser()
        {
            return (PVMIDatastreamuserInterface*)iDatastreamUser;
        }
        PVMFMetadataExtensionInterface *Metadata()
        {
            return (PVMFMetadataExtensionInterface*)iMetadata;
        }
        PVMFFormatProgDownloadSupportInterface *FormatProgDownloadSupport()
        {
            return (PVMFFormatProgDownloadSupportInterface*)iFormatProgDownloadSupport;
        }
        PVMFDownloadProgressInterface *DownloadProgress()
        {
            return (PVMFDownloadProgressInterface*)iDownloadProgress;
        }
        PvmfDataSourcePlaybackControlInterface *DataSourcePlayback()
        {
            return (PvmfDataSourcePlaybackControlInterface*)iDataSourcePlayback;
        }
        PVMFCPMPluginLicenseInterface* LicenseInterface()
        {
            return (PVMFCPMPluginLicenseInterface*)iLicenseInterface;
        }

        //Node event observers
        void HandleNodeErrorEvent(const PVMFAsyncEvent& aEvent);
        void HandleNodeInformationalEvent(const PVMFAsyncEvent& aEvent);
        void NodeCommandCompleted(const PVMFCmdResp& aResponse);
};


/* Container for the recognizer
 */
class PVMFDownloadManagerRecognizerContainer
            : public PVMFDownloadManagerSubNodeContainerBase,
            public PVMFRecognizerCommmandHandler
{
    public:
        // Recognizer to use for determining mime type of downloaded data
        // PVMFRecognizer iRecognizer;
        PVMFDownloadManagerRecognizerContainer()
        {
            iRecognizerSessionId = 0;
        };

        ~PVMFDownloadManagerRecognizerContainer()
        {
            Cleanup();
        }
        void Cleanup();

        //pure virtuals from PVMFDownloadManagerSubNodeContainerBase
        PVMFStatus IssueCommand(int32);
        bool CancelPendingCommand();

        void RecognizerCommandCompleted(const PVMFCmdResp& aResponse);

        PVMFSessionId iRecognizerSessionId;
        Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator> iRecognizerResultVec;
};

/**
 * Download Manager Node.
 * This node contains the protocol engine node, the socket node, and a format parser node.
 * The format parser node is instantiated after using the recognizer to determine the
 * format type of the data stream.
 *
 * The usage sequence is as follows:
 *
 *    Create
 *    Connect
 *    ThreadLogon
 *
 *    (All extension intefaces are available for query now.)
 *
 *    QueryInterface
 *    SetSourceInitializationData
 *    Init (CPM authorization is done during this command)
 *
 *    (Track selection is available now)
 *
 *    GetMediaPresentationInfo
 *    SelectTracks
 *    Prepare
 *
 *    (The Metadata is available now)
 *    (Ports are available now)
 *
 *    RequestPort
 *    Configure & Connect ports
 *
 *    (App should wait on PVMFInfoDataReady before calling Start, but
 *      it is not a strict requirement.  If Start is called too soon,
 *      playback will likely underflow immediately.)
 *
 *    Start
 *
 *    (playback begins)
 *
 * This node does not create any ports.  Port requests are passed directly
 * to the format parser node.
 *
 */
class PVMFDownloadManagerNode
            : public PVMFNodeInterface
            , public OsclActiveObject
            , public PvmiCapabilityAndConfig
            //required extension interfaces for player source nodes.
            , public PVMFDataSourceInitializationExtensionInterface
            , public PVMFTrackSelectionExtensionInterface
            , public PvmfDataSourcePlaybackControlInterface
            , public PVMFMetadataExtensionInterface
            , public PVMFDataSourceNodeRegistryInitInterface
            , public PVMFCPMPluginLicenseInterface
            // For observing the playback clock states
            , public PVMFMediaClockStateObserver
{
    public:
        PVMFDownloadManagerNode(int32 aPriority = OsclActiveObject::EPriorityNominal);
        ~PVMFDownloadManagerNode();

        // From PVMFNodeInterface
        PVMFStatus ThreadLogon();
        PVMFStatus ThreadLogoff();
        PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);

        PVMFCommandId QueryUUID(PVMFSessionId aSessionId,
                                const PvmfMimeString& aMimeType,
                                Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                bool aExactUuidsOnly = false,
                                const OsclAny* aContext = NULL);
        PVMFCommandId QueryInterface(PVMFSessionId aSessionId,
                                     const PVUuid& aUuid,
                                     PVInterface*& aInterfacePtr,
                                     const OsclAny* aContext = NULL);

        PVMFCommandId RequestPort(PVMFSessionId aSessionId, int32 aPortTag,
                                  const PvmfMimeString* aPortConfig = NULL,
                                  const OsclAny* aContext = NULL);
        PVMFStatus ReleasePort(PVMFSessionId aSessionId, PVMFPortInterface& aPort,
                               const OsclAny* aContext = NULL);

        PVMFCommandId Init(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Prepare(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Start(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Stop(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Flush(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Pause(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Reset(PVMFSessionId aSessionId, const OsclAny* aContext = NULL);

        PVMFCommandId CancelAllCommands(PVMFSessionId aSessionId, const OsclAny* aContextData = NULL);
        PVMFCommandId CancelCommand(PVMFSessionId aSessionId, PVMFCommandId aCmdId, const OsclAny* aContextData = NULL);

        //From PVInterface
        void addRef();
        void removeRef();
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        //From PVMFDataSourceInitializationExtensionInterface
        PVMFStatus SetSourceInitializationData(OSCL_wString& aSourceURL, PVMFFormatType& aSourceFormat, OsclAny* aSourceData);
        PVMFStatus SetClientPlayBackClock(PVMFMediaClock* aClientClock);
        PVMFStatus SetEstimatedServerClock(PVMFMediaClock* aClientClock);

        //From PVMFTrackSelectionExtensionInterface
        PVMFStatus GetMediaPresentationInfo(PVMFMediaPresentationInfo& aInfo);
        PVMFStatus SelectTracks(PVMFMediaPresentationInfo& aInfo);

        // From PVMFMetadataExtensionInterface
        uint32 GetNumMetadataKeys(char* aQueryKeyString = NULL);
        uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList);
        PVMFCommandId GetNodeMetadataKeys(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList, uint32 aStartingKeyIndex, int32 aMaxKeyEntries,
                                          char* aQueryKeyString = NULL, const OsclAny* aContextData = NULL);
        PVMFCommandId GetNodeMetadataValues(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList,
                                            Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 aStartingValueIndex, int32 aMaxValueEntries, const OsclAny* aContextData = NULL);
        PVMFStatus ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList, uint32 aStartingKeyIndex, uint32 aEndKeyIndex);
        PVMFStatus ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 aStartingValueIndex, uint32 aEndValueIndex);


        // From PvmfDataSourcePositioningControlInterface
        PVMFCommandId SetDataSourcePosition(PVMFSessionId aSessionId, PVMFTimestamp aTargetNPT, PVMFTimestamp& aActualNPT,
                                            PVMFTimestamp& aActualMediaDataTS, bool aSeekToSyncPoint = true, uint32 aStreamId = 0, OsclAny* aContext = NULL);
        PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId, PVMFTimestamp aTargetNPT, PVMFTimestamp& aActualNPT,
                                              bool aSeekToSyncPoint = true, OsclAny* aContext = NULL);
        PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId, PVMFTimestamp aTargetNPT,
                                              PVMFTimestamp& aSeekPointBeforeTargetNPT, PVMFTimestamp& aSeekPointAfterTargetNPT,
                                              OsclAny* aContext = NULL, bool aSeekToSyncPoint = true);

        PVMFCommandId SetDataSourceRate(PVMFSessionId aSessionId, int32 aRate, PVMFTimebase* aTimebase = NULL, OsclAny* aContext = NULL);

        // From PVMFPortActivityHandler
        void HandlePortActivity(const PVMFPortActivity& aActivity)
        {
            OSCL_UNUSED_ARG(aActivity);
        }

        //From PVMFDataSourceNodeRegistryInitInterface
        PVMFStatus SetPlayerNodeRegistry(PVPlayerNodeRegistryInterface* aRegistry);

        void SetDebugMode()
        {
            iDebugMode = true;
        }

        //From capability and config interface
        virtual void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
        {
            ciObserver = aObserver;
        }

        virtual PVMFStatus getParametersSync(PvmiMIOSession aSession,
                                             PvmiKeyType aIdentifier,
                                             PvmiKvp*& aParameters,
                                             int& aNumParamElements,
                                             PvmiCapabilityContext aContext);
        virtual PVMFStatus releaseParameters(PvmiMIOSession aSession,
                                             PvmiKvp* aParameters,
                                             int num_elements);
        virtual void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        virtual void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                                          PvmiKvp* aParameters, int num_parameter_elements);
        virtual void DeleteContext(PvmiMIOSession aSession,
                                   PvmiCapabilityContext& aContext);
        virtual void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                       int num_elements, PvmiKvp * & aRet_kvp);
        virtual PVMFCommandId setParametersAsync(PvmiMIOSession aSession,
                PvmiKvp* aParameters,
                int num_elements,
                PvmiKvp*& aRet_kvp,
                OsclAny* context = NULL);
        virtual uint32 getCapabilityMetric(PvmiMIOSession aSession);
        virtual PVMFStatus verifyParametersSync(PvmiMIOSession aSession,
                                                PvmiKvp* aParameters,
                                                int num_elements);

        //from PVMFMediaClockStateObserver
        void ClockStateUpdated();
        void NotificationsInterfaceDestroyed();

        /* From PVMFCPMPluginLicenseInterface */
        PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                 OSCL_wString& aContentName,
                                 OSCL_wString& aLicenseURL)
        {
            OSCL_UNUSED_ARG(aSessionId);
            OSCL_UNUSED_ARG(aContentName);
            OSCL_UNUSED_ARG(aLicenseURL);
            //must use Async method.
            return PVMFErrNotSupported;
        }
        PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                 OSCL_String&  aContentName,
                                 OSCL_String&  aLicenseURL)
        {
            OSCL_UNUSED_ARG(aSessionId);
            OSCL_UNUSED_ARG(aContentName);
            OSCL_UNUSED_ARG(aLicenseURL);
            //must use Async method.
            return PVMFErrNotSupported;
        }

        PVMFCommandId GetLicense(PVMFSessionId aSessionId,
                                 OSCL_wString& aContentName,
                                 OsclAny* aData,
                                 uint32 aDataSize,
                                 int32 aTimeoutMsec,
                                 OsclAny* aContextData) ;

        PVMFCommandId GetLicense(PVMFSessionId aSessionId,
                                 OSCL_String&  aContentName,
                                 OsclAny* aData,
                                 uint32 aDataSize,
                                 int32 aTimeoutMsec,
                                 OsclAny* aContextData);

        PVMFCommandId CancelGetLicense(PVMFSessionId aSessionId
                                       , PVMFCommandId aCmdId
                                       , OsclAny* aContextData);

    private:
        bool iDebugMode;

        void ConstructL();

        // from OsclTimerObject
        void Run();

        //Command processing
        //There are 3 command queues.
        //iInputCommands is the input queue for the node.  It is N deep and allows for high priority commands to go to the front of the list.
        //iCurrentCommand is a 1-deep holding place for a command that has started executing but has asynchronous completion.
        //iCancelCommand is a 1-deep holding place for a cancel command that has started executing but has asynchronous completion.
        PVMFDownloadManagerNodeCmdQueue iInputCommands;
        PVMFDownloadManagerNodeCmdQueue iCurrentCommand;
        PVMFDownloadManagerNodeCmdQueue iCancelCommand;
        PVMFCommandId QueueCommandL(PVMFDownloadManagerNodeCommand& aCmd);
        void ProcessCommand();
        void CommandComplete(PVMFDownloadManagerNodeCmdQueue& aCmdQueue, PVMFDownloadManagerNodeCommand& aCmd, PVMFStatus , PVInterface*, OsclAny*);

        // Event reporting
        void ReportErrorEvent(PVMFEventType aEventType, PVInterface*aExt = NULL, OsclAny* aEventData = NULL);
        void ReportInfoEvent(PVMFAsyncEvent&);
        void ChangeNodeState(TPVMFNodeInterfaceState aNewState);

        // Node command handlers
        PVMFStatus DoQueryUuid(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoQueryInterface(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoRequestPort(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoReleasePort(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoInitNode(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoPrepareNode(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoStartNode(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoStopNode(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoFlushNode(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoPauseNode(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoResetNode(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoCancelAllCommands(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoCancelCommand(PVMFDownloadManagerNodeCommand& aCmd);
        // For metadata extention interface
        PVMFStatus DoGetNodeMetadataKey(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoGetNodeMetadataValue(PVMFDownloadManagerNodeCommand& aCmd);
        // For data source position extension interface
        PVMFStatus DoSetDataSourcePosition(PVMFDownloadManagerNodeCommand& aCmd);
        PVMFStatus DoQueryDataSourcePosition(PVMFDownloadManagerNodeCommand& aCmd);

        bool IsByteBasedDownloadProgress(OSCL_String &aDownloadProgressInfo);
        bool GetHttpExtensionHeaderParams(PvmiKvp &aParameter,
                                          OSCL_String &extensionHeaderKey,
                                          OSCL_String &extensionHeaderValue,
                                          HttpMethod  &httpMethod,
                                          bool &aPurgeOnRedirect);
        bool IsHttpExtensionHeaderValid(PvmiKvp &aParameter);
        // remove the ending ';', ',' or ' ' and calulate value length
        uint32 getItemLen(char *ptrItemStart, char *ptrItemEnd);

        bool IsDownloadExtensionHeaderValid(PvmiKvp &);

        // MIME type of the downloaded data
        OSCL_HeapString<OsclMemAllocator> iMimeType;

        // playback mode
        enum TDlMgrPlaybackMode
        {
            EPlayAsap
            , EDownloadThenPlay
            , EDownloadOnly
#if(PVMF_DOWNLOADMANAGER_SUPPORT_PPB)
            , EPlaybackOnly
#endif//PVMF_DOWNLOADMANAGER_SUPPORT_PPB
        };
        TDlMgrPlaybackMode iPlaybackMode;

        // Name of local file to store downloaded data
        OSCL_wHeapString<OsclMemAllocator> iDownloadFileName;

        // Format type of media
        PVMFFormatType iFmt;

        // The local data source info gleaned from the source data set via SetSourceInit interface
        PVMFSourceContextDataCommon iLocalDataSource;

        //Source data.
        PVMFFormatType iSourceFormat;
        OSCL_wHeapString<OsclMemAllocator> iSourceURL;
        OsclAny*iSourceData;

        PVMFNodeCapability iCapability;
        PVLogger* iLogger;
        friend class PVMFDownloadManagerSubNodeContainerBase;
        friend class PVMFDownloadManagerSubNodeContainer;
        friend class PVMFDownloadManagerRecognizerContainer;

        // Reference counter for extension
        uint32 iExtensionRefCount;

        //Sub-nodes.
        PVMFDownloadManagerSubNodeContainer iFormatParserNode;
        PVMFDownloadManagerSubNodeContainer iProtocolEngineNode;
        PVMFDownloadManagerSubNodeContainer iSocketNode;

        PVMFDownloadManagerSubNodeContainer& TrackSelectNode();

        // Recognizer
        PVMFDownloadManagerRecognizerContainer iRecognizerNode;

        // Filebufferdatastream object
        PVMFFileBufferDataStream* iFileBufferDatastreamFactory;
#if(PVMF_DOWNLOADMANAGER_SUPPORT_PPB)
        // MemoryBufferDataStream object
        PVMFMemoryBufferDataStream* iMemoryBufferDatastreamFactory;
#endif //PVMF_DOWNLOADMANAGER_SUPPORT_PPB
        PVMFDataStreamFactory* iReadFactory;
        PVMFDataStreamFactory* iWriteFactory;

        void NotifyDownloadComplete()
        {
#if(PVMF_DOWNLOADMANAGER_SUPPORT_PPB)
            if (EPlaybackOnly == iPlaybackMode)
            {
                if (iMemoryBufferDatastreamFactory != NULL)
                {
                    iMemoryBufferDatastreamFactory->NotifyDownloadComplete();
                }
            }
            else
#endif//PVMF_DOWNLOADMANAGER_SUPPORT_PPB
            {
                if (iFileBufferDatastreamFactory != NULL)
                {
                    iFileBufferDatastreamFactory->NotifyDownloadComplete();
                }
            }
        }

        //flags
        bool iParserInit;//set when file parse sequence is initiated.
        bool iDataReady;//set when initial data ready is received or generated.
        bool iDownloadComplete;//set when DL is complete.
        bool iMovieAtomComplete;//set when movie atom DL complete for fast-track.
        bool iNoPETrackSelect;//set after deciding that PE cannot to track selection.
        bool iParserInitAfterMovieAtom;
        bool iParserPrepareAfterMovieAtom;

        bool iRecognizerError;//set when recognizer fails.
        PVMFStatus iRecognizerStartStatus;

        bool iInitFailedLicenseRequired; //set when PVMFErrLicenseRequired failed

        void ContinueInitAfterTrackSelectDecision();
        void ContinueFromDownloadTrackSelectionPoint();
        void ContinueAfterMovieAtom();

        PVMFNodeInterface* CreateParser();

        //event handling
        void GenerateDataReadyEvent();
        bool FilterPlaybackEventsFromSubNodes(const PVMFAsyncEvent& aEvent);

        // Socket Config Info, for configuring socket node.
        OSCL_HeapString<OsclMemAllocator> iServerAddr;

        // Ports for the protocol node and the socket node
        PVMFPortInterface* iProtocolEngineNodePort;
        PVMFPortInterface* iSocketNodePort;

        //The sub-node command vec contains all the sub-node commands needed for a single node command.
        class CmdElem
        {
            public:
                PVMFDownloadManagerSubNodeContainerBase* iNC;
                PVMFDownloadManagerSubNodeContainerBase::CmdType iCmd;
        };
        Oscl_Vector<CmdElem, OsclMemAllocator> iSubNodeCmdVec;
        PVMFStatus ScheduleSubNodeCommands(PVMFDownloadManagerNodeCommand& aCmd);
        void Push(PVMFDownloadManagerSubNodeContainerBase&, PVMFDownloadManagerSubNodeContainerBase::CmdType);

        //Recognizer related
        PVPlayerNodeRegistryInterface* iPlayerNodeRegistry;

        //Vector to store the Uuids for the download manager nodes created throughout the playback
        Oscl_Vector<PVUuid, OsclMemAllocator> iDNodeUuids;

        //Count for Uuids
        uint32 iDNodeUuidCount;
        // playback clock which will be received form the engine.
        PVMFMediaClock *iPlayBackClock;
        PVMFMediaClockNotificationsInterface *iClockNotificationsInf;

        // HTTP Content-Type header MIME string hint from the server
        OSCL_HeapString<OsclMemAllocator> iContentTypeMIMEString;

        PVMFStatus DoGetLicense(PVMFDownloadManagerNodeCommand& aCmd,
                                bool aWideCharVersion = false);
        void CompleteGetLicense();

        PVMFStatus DoCancelGetLicense(PVMFDownloadManagerNodeCommand& aCmd);
};

///////////////////////////////////////////////////////////////////////////////
//
// Capability and config interface related constants and definitions
//   - based on pv_player_engine.h
//
///////////////////////////////////////////////////////////////////////////////
struct DownloadManagerKeyStringData
{
    char iString[64];
    PvmiKvpType iType;
    PvmiKvpValueType iValueType;
};

// The number of characters to allocate for the key string
#define DLMCONFIG_KEYSTRING_SIZE 128


///////////////////////////////////////////////////////////////////////////////
//
// Constants for setting up socket mem pool for progressive playback and shoutcast
// These constants are not tunables.
//
///////////////////////////////////////////////////////////////////////////////
#define PVMF_DOWNLOADMANAGER_TCP_BUFFER_SIZE_FOR_SC		1500
#define PVMF_DOWNLOADMANAGER_TCP_BUFFER_SIZE_FOR_PPB	64000
#define PVMF_DOWNLOADMANAGER_TCP_BUFFER_NOT_AVAILABLE	2
#define PVMF_DOWNLOADMANAGER_TCP_BUFFER_OVERHEAD		64
#define PVMF_DOWNLOADMANAGER_TCP_AVG_SMALL_PACKET_SIZE	250



#endif // PVMF_DOWNLOADMANAGER_NODE_H_INCLUDED

