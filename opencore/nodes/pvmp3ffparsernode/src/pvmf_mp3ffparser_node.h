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
#ifndef PVMF_MP3FFPARSER_NODE_H_INCLUDED
#define PVMF_MP3FFPARSER_NODE_H_INCLUDED


#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVMF_MEMPOOL_H_INCLUDED
#include "pvmf_mempool.h"
#endif

#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

#ifndef PVMF_RESIZABLE_SIMPLE_MEDIAMSG_H_INCLUDED
#include "pvmf_resizable_simple_mediamsg.h"
#endif

#ifndef PVMF_FFPARSERNODE_PORT_H_INCLUDED
#include "pvmf_mp3ffparser_outport.h"
#endif

#ifndef PVMF_META_DATA_EXTENSION_H_INCLUDED
#include "pvmf_meta_data_extension.h"
#endif

#ifndef PVMF_DATA_SOURCE_INIT_EXTENSION_H_INCLUDED
#include "pvmf_data_source_init_extension.h"
#endif

#ifndef PVMF_DATA_SOURCE_PLAYBACK_CONTROL_H_INCLUDED
#include "pvmf_data_source_playback_control.h"
#endif

#ifndef PVMF_TRACK_SELECTION_EXTENSION_H_INCLUDED
#include "pvmf_track_selection_extension.h"
#endif

#ifndef PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED
#include "pvmf_media_presentation_info.h"
#endif

#ifndef PVMF_MP3FFPARSER_NODE_INTERNAL_H_INCLUDED
#include "pvmf_mp3ffparser_node_internal.h"
#endif

#ifndef PVMF_COMMON_AUDIO_DECNODE_H_INCLUDE
#include "pvmf_common_audio_decnode.h"
#endif

#ifndef PVMF_LOCAL_DATA_SOURCE_H_INCLUDED
#include "pvmf_local_data_source.h"
#endif

#ifndef PVMF_CPMPLUGIN_LICENSE_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_license_interface.h"
#endif

#ifndef PVMF_FORMAT_PROGDOWNLOAD_SUPPORT_EXTENSION_H_INCLUDED
#include "pvmf_format_progdownload_support_extension.h"
#endif

#ifndef PVMF_DOWNLOAD_PROGRESS_EXTENSION_H
#include "pvmf_download_progress_interface.h"
#endif

#ifndef PVMF_SOURCE_CONTEXT_DATA_H_INCLUDED
#include "pvmf_source_context_data.h"
#endif

#ifndef PVMI_DATASTREAMUSER_INTERFACE_H_INCLUDED
#include "pvmi_datastreamuser_interface.h"
#endif

#ifndef PVFILE_H_INCLUDED
#include "pvfile.h"
#endif

#ifndef IMP3FF_H_INCLUDED
#include "imp3ff.h"  // Includes for the core file format mp3 parser library
#endif

#ifndef USE_CML2_CONFIG
#ifndef PVMF_MP3FFPASER_NODE_CONFIG_H_INCLUDED
#include "pvmf_mp3ffparser_node_config.h"
#endif
#endif

#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED // include scsp only when SHOUTCAST support is enabled
#ifndef PVMF_SHOUTCAST_STREAM_PARSER_H_INCLUDED
#include "pvmf_shoutcast_stream_parser.h"
#endif
#endif

#define PVMF_MP3_PARSER_NODE_MAX_CPM_METADATA_KEYS 256

/**
* Container for the CPM object
*/
#ifndef CPM_H_INCLUDED
#include "cpm.h"
#endif

// Forward declaration
class PVMFMP3FFParserNode;
class MediaClockConverter;

class PVMFSubNodeContainerBaseMp3
{
    public:
        PVMFSubNodeContainerBaseMp3()
        {
            iCmdState = EIdle;
            iCancelCmdState = EIdle;
            iContainer = NULL;
        }
        virtual ~PVMFSubNodeContainerBaseMp3()
        {
        }
        enum NodeType {ECPM};
        enum CmdType
        {
            //CPM commands
            ECPMCleanup = 0
            , ECPMInit
            , ECPMOpenSession
            , ECPMRegisterContent
            , ECPMGetLicenseInterface
            , ECPMGetLicense
            , ECPMGetLicenseW
            , ECPMApproveUsage
            , ECPMCheckUsage
            , ECPMUsageComplete
            , ECPMCloseSession
            , ECPMReset
            , ECPMCancelGetLicense
        };

        void Construct(NodeType n, PVMFMP3FFParserNode* c)
        {
            iType = n;
            iContainer = c;
        }

        virtual void Cleanup() = 0;
        virtual PVMFStatus IssueCommand(int32) = 0;
        virtual bool CancelPendingCommand() = 0;

        void CommandDone(PVMFStatus, PVInterface*, OsclAny*);
        void CancelCommandDone(PVMFStatus, PVInterface*, OsclAny*);

        bool CmdPending()
        {
            return iCancelCmdState != EIdle
                   || iCmdState != EIdle;
        }

    protected:
        PVMFMP3FFParserNode*iContainer;
        NodeType iType;

        //Command processing .
        PVMFCommandId iCmdId;
        enum CmdState
        {
            EIdle, //no command
            EBusy //command issued to the sub-node, completion pending.
        };
        CmdState iCmdState;
        int32 iCmd;
        PVMFCommandId iCancelCmdId;
        CmdState iCancelCmdState;
};

class PVMFCPMContainerMp3: public PVMFSubNodeContainerBaseMp3,
            public PVMFCPMStatusObserver
{
    public:
        PVMFCPMContainerMp3()
        {
            iRequestedUsage.key = NULL;
            iApprovedUsage.key = NULL;
            iAuthorizationDataKvp.key = NULL;
            iCPMContentAccessFactory = NULL;
            iCPM = NULL;
        }

        ~PVMFCPMContainerMp3()
        {
            Cleanup();
        }

        // From PVMFCPMStatusObserver
        OSCL_IMPORT_REF void CPMCommandCompleted(const PVMFCmdResp& aResponse) ;

        //from PVMFSubNodeContainerBase
        PVMFStatus IssueCommand(int32);
        bool CancelPendingCommand();
        void Cleanup();

        //CPM session ID
        PVMFSessionId iSessionId;

        //CPM object
        PVMFCPM* iCPM;

        //CPM data
        PVMFCPMContentType iCPMContentType;
        PVMFCPMPluginAccessInterfaceFactory* iCPMContentAccessFactory;
        PvmiKvp iRequestedUsage;
        PvmiKvp iApprovedUsage;
        PvmiKvp iAuthorizationDataKvp;
        PVMFCPMUsageID iUsageID;
        PVMFCPMPluginLicenseInterface* iCPMLicenseInterface;
        PVInterface* iCPMLicenseInterfacePVI;
        PVMFMetadataExtensionInterface* iCPMMetaDataExtensionInterface;

        PVMFStatus CreateUsageKeys();
        PVMFStatus CheckApprovedUsage();
        PVMFCommandId GetCPMLicenseInterface();
        bool GetCPMMetaDataExtensionInterface();
};


class PVMP3FFNodeTrackPortInfo
{
    public:
        enum TrackState
        {
            TRACKSTATE_UNINITIALIZED,
            TRACKSTATE_INITIALIZED,
            TRACKSTATE_TRANSMITTING_GETDATA,
            TRACKSTATE_TRANSMITTING_SENDDATA,
            TRACKSTATE_TRANSMITTING_SENDBOS,
            TRACKSTATE_SEND_ENDOFTRACK,
            TRACKSTATE_TRACKDATAPOOLEMPTY,
            TRACKSTATE_MEDIADATAPOOLEMPTY,
            TRACKSTATE_DESTFULL,
            TRACKSTATE_SOURCEEMPTY,
            TRACKSTATE_ENDOFTRACK,
            TRACKSTATE_DOWNLOAD_AUTOPAUSE,
            TRACKSTATE_ERROR
        };

        PVMP3FFNodeTrackPortInfo()
        {

            iClockConverter = NULL;
            iPort = NULL;
            iState = TRACKSTATE_UNINITIALIZED;
            iTrackDataMemoryPool = NULL;
            iMediaDataImplAlloc = NULL;
            iMediaDataMemPool = NULL;
            timestamp_offset = 0;
            iSeqNum = 0;
            iSendBOS = false;
            iFirstFrame = false;
        }

        PVMP3FFNodeTrackPortInfo(const PVMP3FFNodeTrackPortInfo& aSrc)
        {
            iPort = aSrc.iPort;
            iClockConverter = aSrc.iClockConverter;
            iMediaData = aSrc.iMediaData;
            iState = aSrc.iState;
            iTrackDataMemoryPool = aSrc.iTrackDataMemoryPool;
            iMediaDataImplAlloc = aSrc.iMediaDataImplAlloc;
            iMediaDataMemPool = aSrc.iMediaDataMemPool;
            timestamp_offset = aSrc.timestamp_offset;
            iSeqNum = aSrc.iSeqNum;
            iSendBOS = aSrc.iSendBOS;
            iFirstFrame = aSrc.iFirstFrame;
        }

        ~PVMP3FFNodeTrackPortInfo()
        {
        }

        // Output port to send the data downstream
        PVMFMP3FFParserPort* iPort;

        // Shared memory pointer holding the currently retrieved track data
        PVMFSharedMediaDataPtr iMediaData;

        // Current state of this track
        TrackState iState;

        // Output buffer memory pool
        OsclMemPoolResizableAllocator *iTrackDataMemoryPool;

        // Allocator for simple media data buffer impl
        PVMFResizableSimpleMediaMsgAlloc *iMediaDataImplAlloc;

        // Memory pool for simple media data
        PVMFMemPoolFixedChunkAllocator *iMediaDataMemPool;

        // Converter to convert from track timescale to milliseconds
        MediaClockConverter* iClockConverter;


        PVMFFormatType iFormatType;
        uint32 iBitrate;
        int32 timestamp_offset;

        // Sequence number
        uint32 iSeqNum;
        //bos flag
        bool iSendBOS;
        // Random access point idenfier
        bool iFirstFrame;
};  // end class PVMP3FFNodeTrackPortInfo

//Forward Declarations
class IMpeg3File;
class PVLogger;

/*
* The class PVMp3DurationCalculator is the external interface for calculating
* the clip duration as a background AO.
**/
class PVMp3DurationCalculator : public OsclTimerObject
{
    public:
        PVMp3DurationCalculator(int32 aPriority, IMpeg3File* aMP3File, PVMFNodeInterface* aNode, bool aScanEnabled = true);
        ~PVMp3DurationCalculator();
        void Run();
        void ScheduleAO();
    private:
        PVFile* iFile;
        bool iScanComplete;
        bool iScanEnabled;
        MP3ErrorType iErrorCode;
        IMpeg3File* iMP3File;
        PVMFNodeInterface* iNode;
        uint32 totalticks;
};
/*
* The class PVMFMP3FFParserNode is the external interface for using the node.
* Move this to a separate interface file and all the stuff above can go in a
* private header in the src directory.
* */

class PVMFMP3FFParserNode : public OsclTimerObject,
            public PVMFNodeInterface,
            public PVMFDataSourceInitializationExtensionInterface,
            public PVMFTrackSelectionExtensionInterface,
            public PVMFMetadataExtensionInterface,
            public OsclMemPoolFixedChunkAllocatorObserver,
            public PVMFFormatProgDownloadSupportInterface,
            public PvmiDataStreamObserver,
            public PVMIDatastreamuserInterface,
            public OsclMemPoolResizableAllocatorObserver,
            public PvmfDataSourcePlaybackControlInterface,
            public PVMFCPMPluginLicenseInterface
#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED
            , public PVMFMetadataUpdatesObserver
#endif
{
    public:
        PVMFMP3FFParserNode(int32 aPriority = OsclActiveObject::EPriorityNominal);
        ~PVMFMP3FFParserNode();

        //from PVMFNodeInterface
        PVMFStatus ThreadLogon();
        PVMFStatus ThreadLogoff();
        PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);
        PVMFCommandId QueryUUID(PVMFSessionId, const PvmfMimeString& aMimeType,
                                Oscl_Vector<PVUuid, PVMFMP3FFParserNodeAllocator>& aUuids,
                                bool aExactUuidsOnly = false, const OsclAny* aContext = NULL);
        PVMFCommandId QueryInterface(PVMFSessionId, const PVUuid& aUuid,
                                     PVInterface*& aInterfacePtr, const OsclAny* aContext = NULL);
        PVMFCommandId RequestPort(PVMFSessionId, int32 aPortTag, const PvmfMimeString* aPortConfig = NULL, const OsclAny* aContext = NULL);
        PVMFCommandId ReleasePort(PVMFSessionId, PVMFPortInterface& aPort, const OsclAny* aContext = NULL);
        PVMFCommandId Init(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Prepare(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Start(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Stop(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Flush(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Pause(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Reset(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId CancelAllCommands(PVMFSessionId, const OsclAny* aContextData = NULL);
        PVMFCommandId CancelCommand(PVMFSessionId, PVMFCommandId aCmdId, const OsclAny* aContextData = NULL);
        PVMFStatus QueryInterfaceSync(PVMFSessionId aSession,
                                      const PVUuid& aUuid,
                                      PVInterface*& aInterfacePtr);

        //From PVMFDataSourceInitializationExtensionInterface
        void addRef();
        void removeRef();
        bool queryInterface(const PVUuid& uuid, PVInterface *& iface);

        PVMFStatus SetSourceInitializationData(OSCL_wString& aSourceURL, PVMFFormatType& aSourceFormat, OsclAny* aSourceData);
        PVMFStatus SetClientPlayBackClock(PVMFMediaClock* aClientClock);
        PVMFStatus SetEstimatedServerClock(PVMFMediaClock* aClientClock);

        //From PVMFTrackSelectionExtensionInterface
        PVMFStatus GetMediaPresentationInfo(PVMFMediaPresentationInfo& aInfo);
        PVMFStatus SelectTracks(PVMFMediaPresentationInfo& aInfo);

#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED
        //From PVMFMetadataUpdatesObserver
        void MetadataUpdated(uint32 aMetadataSize);
#endif
        // From PVMFMetadataExtensionInterface
        uint32 GetNumMetadataKeys(char* aQueryKeyString = NULL);
        uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList);
        PVMFCommandId GetNodeMetadataKeys(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList,
                                          uint32 aStartingKeyIndex, int32 aMaxValueEntries, char* aQueryKeyString = NULL, const OsclAny* aContextData = NULL);
        PVMFCommandId GetNodeMetadataValues(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList,
                                            Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 aStartingValueIndex, int32 aMaxValueEntries, const OsclAny* aContextData = NULL);
        PVMFStatus ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList, uint32 aStartingKeyIndex, uint32 aEndKeyIndex);
        PVMFStatus ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 aStartingValueIndex, uint32 aEndValueIndex);

        // From PvmfDataSourcePlaybackControlInterface
        virtual PVMFCommandId SetDataSourcePosition(PVMFSessionId aSessionId,
                PVMFTimestamp aTargetNPT,
                PVMFTimestamp& aActualNPT,
                PVMFTimestamp& aActualMediaDataTS,
                bool aSeekToSyncPoint = true,
                uint32 aStreamID = 0,
                OsclAny* aContext = NULL);

        virtual PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId,
                PVMFTimestamp aTargetNPT,
                PVMFTimestamp& aActualNPT,
                bool aSeekToSyncPoint = true,
                OsclAny* aContext = NULL);

        virtual PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId,
                PVMFTimestamp aTargetNPT,
                PVMFTimestamp& aSeekPointBeforeTargetNPT,
                PVMFTimestamp& aSeekPointAfterTargetNPT,
                OsclAny* aContext = NULL,
                bool aSeekToSyncPoint = true);

        virtual PVMFCommandId SetDataSourceRate(PVMFSessionId aSessionId,
                                                int32 aRate,
                                                PVMFTimebase* aTimebase = NULL,
                                                OsclAny* aContext = NULL);

        /* From PVMFFormatProgDownloadSupportInterface */
        int32 convertSizeToTime(uint32 fileSize, uint32& aNPTInMS);
        bool setProtocolInfo(Oscl_Vector<PvmiKvp*, OsclMemAllocator>& aInfoKvpVec);
        void setFileSize(const uint32 aFileSize);
        void setDownloadProgressInterface(PVMFDownloadProgressInterface* download_progress);
        void playResumeNotification(bool aDownloadComplete);
        void notifyDownloadComplete()
        {
            playResumeNotification(true);
        };

        /* From PVMIDatastreamuserInterface */
        void PassDatastreamFactory(PVMFDataStreamFactory& aFactory,
                                   int32 aFactoryTag,
                                   const PvmfMimeString* aFactoryConfig = NULL);

        void PassDatastreamReadCapacityObserver(PVMFDataStreamReadCapacityObserver* aObserver);

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
        PVMFStatus CheckForMP3HeaderAvailability();
        PVMFStatus GetFileOffsetForAutoResume(uint32& aOffset, PVMP3FFNodeTrackPortInfo* aTrackPortInfo);
        PVMFStatus ParseShoutcastMetadata(char* aMetadataBuf, uint32 aMetadataSize, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aKvpVector);

        void Construct();

        //from OsclActiveObject
        void Run();

        // Port processing
        PVMFStatus ProcessOutgoingMsg(PVMFPortInterface* aPort);
        bool HandleOutgoingQueueReady(PVMFPortInterface* aPortInterface);

        //Command processing
        PVMFCommandId QueueCommandL(PVMFMP3FFParserNodeCommand&);
        void ProcessCommand();
        void CommandComplete(PVMFMP3FFParserNodeCmdQ&, PVMFMP3FFParserNodeCommand&, PVMFStatus, PVInterface*extmsg, OsclAny* aData);
        void CompleteInit(PVMFStatus aStatus);
        bool FlushPending();

        //Command handlers.
        PVMFStatus DoReset(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoQueryUuid(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoQueryInterface(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoRequestPort(PVMFMP3FFParserNodeCommand&, PVMFPortInterface*&);
        PVMFStatus DoReleasePort(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoInit(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoPrepare(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoStart(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoStop(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoFlush(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoPause(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoCancelAllCommands(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoCancelCommand(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoGetMetadataKeys(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoGetMetadataValues(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoSetDataSourceRate(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoSetDataSourcePosition(PVMFMP3FFParserNodeCommand&);
        PVMFStatus DoQueryDataSourcePosition(PVMFMP3FFParserNodeCommand&);

        // Event reporting
        void ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL);
        void ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, PVInterface*aExtMsg = NULL);
        void SetState(TPVMFNodeInterfaceState);

        bool HandleTrackState();
        bool RetrieveTrackData(PVMP3FFNodeTrackPortInfo& aTrackPortInfo);
        bool SendTrackData(PVMP3FFNodeTrackPortInfo& aTrackPortInfo);
        bool SendEndOfTrackCommand(PVMP3FFNodeTrackPortInfo& aTrackPortInfo);
        bool SendBeginOfMediaStreamCommand(PVMP3FFNodeTrackPortInfo& aTrackPortInfo);
        int32 FindTrackID(PVMFFormatType aFormatType);
        PVMFStatus ParseFile();
        bool CreateFormatSpecificInfo(uint32 numChannels, uint32 samplingRate);

        void ResetTrack();
        void ReleaseTrack();
        void RemoveAllCommands();
        void CleanupFileSource();

        //from PVMFPortActivityHandler
        void HandlePortActivity(const PVMFPortActivity& aActivity);

        // From OsclMemPoolFixedChunkAllocatorObserver
        void freechunkavailable(OsclAny*);
        // From OsclMemPoolResizableAllocatorObserver
        void freeblockavailable(OsclAny*);

        // From PvmiDataStreamObserver
        void DataStreamCommandCompleted(const PVMFCmdResp& aResponse);
        void DataStreamInformationalEvent(const PVMFAsyncEvent& aEvent);
        void DataStreamErrorEvent(const PVMFAsyncEvent& aEvent);
        PVMFStatus CreateMP3FileObject(MP3ErrorType &aSuccess, PVMFCPMPluginAccessInterfaceFactory*aCPM);
        PVMFStatus PushBackCPMMetadataKeys(PVMFMetadataList *&aKeyListPtr, uint32 aLcv);

    protected:
        void Push(PVMFSubNodeContainerBaseMp3&, PVMFSubNodeContainerBaseMp3::CmdType);
        PVMFCPMContainerMp3 iCPMContainer;

// private member variables
    private:

#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED
        // shoutcast related
        int32 iClipByteRate;
        int32 iMetadataBufSize;
        uint32 iMetadataSize;
        int32 iMetadataInterval;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataVector;

        PVMFShoutcastStreamParserFactory* iSCSPFactory;
        PVMFShoutcastStreamParser* iSCSP;
        uint8 *iMetadataBuf;
#endif

        // Vector of ports contained in this node
        PVMFPortVector<PVMFMP3FFParserPort, PVMFMP3FFParserNodeAllocator> iPortVector;

        // command queues
        PVMFMP3FFParserNodeCmdQ iInputCommands;
        PVMFMP3FFParserNodeCmdQ iCurrentCommand;
        PVMFMP3FFParserNodeCmdQ iCancelCommand;

        // stream id
        uint32 iStreamID;
        // Parse status
        bool iParseStatus;

        PVMFNodeCapability iCapability;

        OSCL_wHeapString<OsclMemAllocator> iSourceURL;
        bool iSourceURLSet;
        PVMFFormatType iSourceFormat;
        PVMFSourceContextData iSourceContextData;
        bool iSourceContextDataValid;
        OsclFileHandle* iFileHandle;
        PVMFLocalDataSource iCPMSourceData;
        Oscl_FileServer iFileServer;
        IMpeg3File* iMP3File;
        //MP3Parser* pMP3Parser;
        uint32 iConfigOk;
        uint32 iExtensionRefCount;
        int iMaxFrameSize;
        PVMP3FFNodeTrackPortInfo iTrack; //The track that this node is streaming. Current assumption is one track supported per node.
        uint32 iMP3FormatBitrate;
        bool iFileSizeRecvd;
        uint32 iFileSize;
        // Logger
        PVLogger *iLogger;

        // Channel sample info stored in a OsclRefCounterMemFrag
        OsclMemAllocDestructDealloc<uint8> iDecodeFormatSpecificInfoAlloc;
        OsclRefCounterMemFrag iDecodeFormatSpecificInfo;
        bool iSendDecodeFormatSpecificInfo;

        /* These vars are used for the prog. download to auto pause*/
        static const uint32 iTIMESTAMPDELTA;
        OsclSharedPtr<PVMFMediaClock> iDownloadProgressClock;
        PVMFDownloadProgressInterface* iDownloadProgressInterface;
        bool iAutoPaused;
        bool iDownloadComplete;
        PvmiDataStreamCommandId iRequestReadCapacityNotificationID;
        uint32 iMP3MetaDataSize;

        // Data Stream vars
        PVMIDataStreamSyncInterface* iDataStreamInterface;
        PVMFDataStreamFactory* iDataStreamFactory;
        PVMFDataStreamReadCapacityObserver* iDataStreamReadCapacityObserver;
        PvmiDataStreamSession iDataStreamSessionID;

        //metadata related
        uint32 iMP3ParserNodeMetadataValueCount;
        Oscl_Vector<OSCL_HeapString<PVMFMP3FFParserNodeAllocator>, PVMFMP3FFParserNodeAllocator> iCPMMetadataKeys;
        PVMFStatus CompleteGetMetadataKeys(PVMFMP3FFParserNodeCommand& aCmd);

        //for CPM
        bool oWaitingOnLicense;
        PVMFCommandId iCPMGetMetaDataKeysCmdId;
        PVMFCommandId iCPMGetMetaDataValuesCmdId;
        class SubNodeCmd
        {
            public:
                PVMFSubNodeContainerBaseMp3* iSubNodeContainer;
                PVMFSubNodeContainerBaseMp3::CmdType iCmd;
        };
        Oscl_Vector<SubNodeCmd, OsclMemAllocator> iSubNodeCmdVec;

        PVMFStatus DoGetLicense(PVMFMP3FFParserNodeCommand& aCmd,
                                bool aWideCharVersion = false);
        PVMFStatus DoCancelGetLicense(PVMFMP3FFParserNodeCommand& aCmd);
        void CompleteGetLicense();
        void GetCPMMetaDataKeys();

        PVMp3DurationCalculator* iDurationCalcAO;
        friend class PVMFSubNodeContainerBaseMp3;
        friend class PVMFCPMContainerMp3;
        friend class PVMFMP3FFParserPort;
        friend class PVMp3DurationCalculator;

        bool iCheckForMP3HeaderDuringInit;
};

#endif // PVMF_MP3FFPARSER_NODE_H_INCLUDED

