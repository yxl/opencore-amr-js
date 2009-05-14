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
#ifndef PVMF_WAVFFPARSER_NODE_H_INCLUDED
#define PVMF_WAVFFPARSER_NODE_H_INCLUDED


#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVMF_MEMPOOL_H_INCLUDED
#include "pvmf_mempool.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
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

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

#ifndef PVMF_META_DATA_EXTENSION_H_INCLUDED
#include "pvmf_meta_data_extension.h"
#endif

#ifndef PVMF_DATA_SOURCE_PLAYBACK_CONTROL_H_INCLUDED
#include "pvmf_data_source_playback_control.h"
#endif

#ifndef WAV_PARSERNODE_TUNABLES_H_INCLUDED
#include "wav_parsernode_tunables.h"
#endif

#ifndef PVWAVFILEPARSER_H_INCLUDED
#include "pvwavfileparser.h"
#endif

class MediaClockConverter;
class PVMFWAVFFParserNode;
class PVWAVFFNodeTrackPortInfo : public OsclMemPoolFixedChunkAllocatorObserver
{
    public:
        enum TrackState
        {
            TRACKSTATE_UNINITIALIZED,
            TRACKSTATE_INITIALIZED,
            TRACKSTATE_TRANSMITTING_GETDATA,
            TRACKSTATE_TRANSMITTING_SENDDATA,
            TRACKSTATE_SEND_ENDOFTRACK,
            TRACKSTATE_MEDIADATAPOOLEMPTY,
            TRACKSTATE_DESTFULL,
            TRACKSTATE_SOURCEEMPTY,
            TRACKSTATE_ENDOFTRACK,
            TRACKSTATE_ERROR
        };

        PVWAVFFNodeTrackPortInfo()
        {
            iTrackId = -1;
            iPort = NULL;
            iClockConverter = NULL;
            iState = TRACKSTATE_UNINITIALIZED;
            iTrackDataMemoryPool = NULL;
            iMediaDataImplAlloc = NULL;
            iMediaDataMemPool = NULL;
            iNode = NULL;
            iSeqNum = 0;
            iSendBOS = false;
        }

        PVWAVFFNodeTrackPortInfo(const PVWAVFFNodeTrackPortInfo& aSrc):
                OsclMemPoolFixedChunkAllocatorObserver()
        {
            iTrackId = aSrc.iTrackId;
            iPort = aSrc.iPort;
            iTag = aSrc.iTag;
            iClockConverter = aSrc.iClockConverter;
            iMediaData = aSrc.iMediaData;
            iState = aSrc.iState;
            iTrackDataMemoryPool = aSrc.iTrackDataMemoryPool;
            iMediaDataImplAlloc = aSrc.iMediaDataImplAlloc;
            iMediaDataMemPool = aSrc.iMediaDataMemPool;
            iNode = aSrc.iNode;
            iSeqNum = aSrc.iSeqNum;
            iSendBOS = aSrc.iSendBOS;
        }

        virtual ~PVWAVFFNodeTrackPortInfo()
        {
        }

        // From OsclMemPoolFixedChunkAllocatorObserver
        // Callback handler when mempool's deallocate() is called after
        // calling notifyfreechunkavailable() on the mempool
        void freechunkavailable(OsclAny*);

        // Track ID number in WAV FF
        int32 iTrackId;
        // Output port to send the data downstream
        PVMFPortInterface* iPort;

        // Settings for the output port
        //PVMFPortProperty iPortProperty;

        int32 iTag;

        // Converter to convert from track timescale to milliseconds
        MediaClockConverter* iClockConverter;
        // Shared memory pointer holding the currently retrieved track data
        PVMFSharedMediaDataPtr iMediaData;
        // Current state of this track
        TrackState iState;
        // Output buffer memory pool
        OsclMemPoolFixedChunkAllocator *iTrackDataMemoryPool;
        // Allocator for simple media data buffer impl
        PVMFSimpleMediaBufferCombinedAlloc *iMediaDataImplAlloc;
        // Memory pool for simple media data
        PVMFMemPoolFixedChunkAllocator *iMediaDataMemPool;
        // WAV FF parser node handle
        PVMFWAVFFParserNode* iNode;
        // Sequence number
        uint32 iSeqNum;
        // bos flag
        bool iSendBOS;
};

//memory allocator type for this node.
typedef OsclMemAllocator PVMFWAVFFNodeAllocator;

//Node command type.
enum PVMFWAVFFNodeCommandType
{
    PVWAVFF_NODE_CMD_SETDATASOURCEPOSITION = PVMF_GENERIC_NODE_COMMAND_LAST,
    PVWAVFF_NODE_CMD_QUERYDATASOURCEPOSITION,
    PVWAVFF_NODE_CMD_SETDATASOURCERATE,
    PVWAVFF_NODE_CMD_GETNODEMETADATAKEY,
    PVWAVFF_NODE_CMD_GETNODEMETADATAVALUE
};
#define PVMFWAVFFNodeCommandBase PVMFGenericNodeCommand<PVMFWAVFFNodeAllocator>  // to remove typedef warning on symbian
class PVMFWAVFFNodeCommand: public PVMFWAVFFNodeCommandBase
{

    public:
        // Constructor and parser for GetNodeMetadataKeys
        void Construct(PVMFSessionId s, int32 cmd
                       , PVMFMetadataList& aKeyList
                       , uint32 aStartingIndex
                       , int32 aMaxEntries
                       , char* aQueryKey
                       , const OsclAny* aContext)
        {
            PVMFWAVFFNodeCommandBase::Construct(s, cmd, aContext);
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
            PVMFWAVFFNodeCommandBase::Construct(s, cmd, aContext);
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
        void Construct(PVMFSessionId s, int32 cmd, PVMFTimestamp aTargetNPT, PVMFTimestamp* aActualNPT, PVMFTimestamp* aActualMediaDataTS,
                       bool aSeekToSyncPoint, uint32 aStreamID, const OsclAny*aContext)
        {
            PVMFWAVFFNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aTargetNPT;
            iParam2 = (OsclAny*)aActualNPT;
            iParam3 = (OsclAny*)aActualMediaDataTS;
            iParam4 = (OsclAny*)aSeekToSyncPoint;
            iParam5 = (OsclAny*)aStreamID;
        }
        void Parse(PVMFTimestamp& aTargetNPT, PVMFTimestamp* &aActualNPT, PVMFTimestamp* &aActualMediaDataTS, bool& aSeekToSyncPoint, uint32& aStreamID)
        {
            aTargetNPT = (PVMFTimestamp)iParam1;
            aActualNPT = (PVMFTimestamp*)iParam2;
            aActualMediaDataTS = (PVMFTimestamp*)iParam3;
            aSeekToSyncPoint = (iParam4 ? true : false);
            aStreamID = (uint32)iParam5;
        }

        // Constructor and parser for QueryDataSourcePosition
        void Construct(PVMFSessionId s, int32 cmd, PVMFTimestamp aTargetNPT, PVMFTimestamp* aActualNPT, bool aSeekToSyncPoint, const OsclAny*aContext)
        {
            PVMFWAVFFNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)aTargetNPT;
            iParam2 = (OsclAny*)aActualNPT;
            iParam3 = (OsclAny*)aSeekToSyncPoint;
            iParam4 = NULL;
            iParam5 = NULL;
        }
        void Parse(PVMFTimestamp& aTargetNPT, PVMFTimestamp* &aActualNPT, bool& aSeekToSyncPoint)
        {
            aTargetNPT = (PVMFTimestamp)iParam1;
            aActualNPT = (PVMFTimestamp*)iParam2;
            aSeekToSyncPoint = (iParam3 ? true : false);
        }

        // Constructor and parser for SetDataSourceRate
        void Construct(PVMFSessionId s, int32 cmd, int32 aRate, PVMFTimebase* aTimebase, const OsclAny* aContext)
        {
            PVMFWAVFFNodeCommandBase::Construct(s, cmd, aContext);
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

        //need to overlaod the base Destroy routine to cleanup metadata key.
        void Destroy()
        {
            PVMFGenericNodeCommand<OsclMemAllocator>::Destroy();
            switch (iCmd)
            {
                case PVWAVFF_NODE_CMD_GETNODEMETADATAKEY:
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
                case PVWAVFF_NODE_CMD_GETNODEMETADATAKEY:
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

};

//Command queue type
typedef PVMFNodeCommandQueue<PVMFWAVFFNodeCommand, PVMFWAVFFNodeAllocator> PVMFWAVFFNodeCmdQ;

class PV_Wav_Parser;
class PVMFWAVFFParserOutPort;
class PVLogger;

class PVMFWAVFFParserNode : public OsclTimerObject,
            public PVMFNodeInterface,
            public PVMFDataSourceInitializationExtensionInterface,
            public PVMFTrackSelectionExtensionInterface,
            public PvmfDataSourcePlaybackControlInterface,
            public PVMFMetadataExtensionInterface
{
    public:
        PVMFWAVFFParserNode(int32 aPriority = OsclActiveObject::EPriorityNominal);
        ~PVMFWAVFFParserNode();

        // From PVMFNodeInterface
        PVMFStatus ThreadLogon();
        PVMFStatus ThreadLogoff();
        PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);
        PVMFCommandId QueryUUID(PVMFSessionId, const PvmfMimeString& aMimeType,
                                Oscl_Vector<PVUuid, PVMFWAVFFNodeAllocator>& aUuids,
                                bool aExactUuidsOnly = false,
                                const OsclAny* aContext = NULL);
        PVMFCommandId QueryInterface(PVMFSessionId, const PVUuid& aUuid,
                                     PVInterface*& aInterfacePtr,
                                     const OsclAny* aContext = NULL);
        virtual PVMFCommandId RequestPort(PVMFSessionId aSession,
                                          int32 aPortTag,
                                          const PvmfMimeString* aPortConfig = NULL,
                                          const OsclAny* aContext = NULL);
        PVMFCommandId ReleasePort(PVMFSessionId, PVMFPortInterface& aPort, const OsclAny* aContext = NULL);
        PVMFCommandId Init(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Start(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Stop(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Flush(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Pause(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Reset(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Prepare(PVMFSessionId aSession, const OsclAny* aContext = NULL);
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

        // From PVMFMetadataExtensionInterface
        uint32 GetNumMetadataKeys(char* aQueryKeyString = NULL);
        uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList);
        PVMFCommandId GetNodeMetadataKeys(PVMFSessionId aSessionId,
                                          PVMFMetadataList& aKeyList,
                                          uint32 aStartingKeyIndex,
                                          int32 aMaxKeyEntries,
                                          char* aQueryKeyString = NULL,
                                          const OsclAny* aContextData = NULL);
        PVMFCommandId GetNodeMetadataValues(PVMFSessionId aSessionId,
                                            PVMFMetadataList& aKeyList,
                                            Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                                            uint32 aStartingValueIndex,
                                            int32 aMaxValueEntries,
                                            const OsclAny* aContextData = NULL);
        PVMFStatus ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList,
                                           uint32 aStartingKeyIndex,
                                           uint32 aEndKeyIndex) ;

        PVMFStatus ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                                             uint32 aStartingValueIndex,
                                             uint32 aEndValueIndex) ;

        // From PvmfDataSourcePlaybackControlInterface
        PVMFCommandId SetDataSourcePosition(PVMFSessionId aSessionId, PVMFTimestamp aTargetNPT, PVMFTimestamp& aActualNPT, PVMFTimestamp& aActualMediaDataTS, bool aSeekToSyncPoint = true, uint32 aStreamID = 0, OsclAny* aContext = NULL);
        PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId, PVMFTimestamp aTargetNPT, PVMFTimestamp& aActualNPT, bool aSeekToSyncPoint = true, OsclAny* aContext = NULL);
        PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId, PVMFTimestamp aTargetNPT,
                                              PVMFTimestamp& aSeekPointBeforeTargetNPT, PVMFTimestamp& aSeekPointAfterTargetNPT,
                                              OsclAny* aContext = NULL, bool aSeekToSyncPoint = true);

        PVMFCommandId SetDataSourceRate(PVMFSessionId aSession, int32 aRate, PVMFTimebase* aTimebase = NULL, OsclAny* aContext = NULL);


    private:

        //from PVMFPortActivityHandler
        void HandlePortActivity(const PVMFPortActivity& aActivity);

        void Construct();
        void Run();

        // Port processing
        bool ProcessPortActivity();
        void QueuePortActivity(const PVMFPortActivity &aActivity);
        PVMFStatus ProcessOutgoingMsg(PVMFPortInterface* aPort);
        bool HandleOutgoingQueueReady(PVMFPortInterface* aPortInterface);

        /**
         * Queue holding port activity. Only incoming and outgoing msg activity are
         * put on the queue.  For each port, there should only be at most one activity
         * of each type on the queue.
         */
        Oscl_Vector<PVMFPortActivity, PVMFWAVFFNodeAllocator> iPortActivityQueue;

        //Command processing
        PVMFCommandId QueueCommandL(PVMFWAVFFNodeCommand& aCmd);
        bool ProcessCommand(PVMFWAVFFNodeCommand&);
        void CommandComplete(PVMFWAVFFNodeCmdQ&, PVMFWAVFFNodeCommand&, PVMFStatus, OsclAny* aData = NULL,
                             PVUuid* aEventUUID = NULL, int32* aEventCode = NULL);
        bool FlushPending();

        //Command handlers.
        void DoQueryUuid(PVMFWAVFFNodeCommand&);
        void DoQueryInterface(PVMFWAVFFNodeCommand&);
        void DoInit(PVMFWAVFFNodeCommand&);
        void DoPrepare(PVMFWAVFFNodeCommand&);
        void DoStart(PVMFWAVFFNodeCommand&);
        void DoStop(PVMFWAVFFNodeCommand&);
        void DoPause(PVMFWAVFFNodeCommand&);
        void DoReset(PVMFWAVFFNodeCommand&);
        void DoFlush(PVMFWAVFFNodeCommand&);
        void DoCancelAllCommands(PVMFWAVFFNodeCommand&);
        void DoCancelCommand(PVMFWAVFFNodeCommand&);
        void DoRequestPort(PVMFWAVFFNodeCommand&);
        void DoReleasePort(PVMFWAVFFNodeCommand&);
        void DoSetDataSourcePosition(PVMFWAVFFNodeCommand& aCmd);
        void DoQueryDataSourcePosition(PVMFWAVFFNodeCommand& aCmd);
        void DoSetDataSourceRate(PVMFWAVFFNodeCommand& aCmd);
        PVMFStatus DoGetNodeMetadataKey(PVMFWAVFFNodeCommand& aCmd);
        PVMFStatus DoGetNodeMetadataValue(PVMFWAVFFNodeCommand& aCmd);
        void InitializeTrackStructure();

        bool verify_supported_format();
        // used to configure downstream ports
        PVMFStatus NegotiateSettings(PvmiCapabilityAndConfig* configInterface);

        // Event reporting
        void ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL);
        void ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL);
        void SetState(TPVMFNodeInterfaceState);

        bool MapWAVErrorCodeToEventCode(int32 aWAVErrCode, PVUuid& aEventUUID, int32& aEventCode);

        // Track data processing
        bool HandleTrackState();
        bool RetrieveTrackData(PVWAVFFNodeTrackPortInfo& aTrackPortInfo);
        bool SendTrackData(PVWAVFFNodeTrackPortInfo& aTrackPortInfo);
        bool SendEndOfTrackCommand(PVWAVFFNodeTrackPortInfo& aTrackPortInfo);
        bool SendBeginOfMediaStreamCommand(PVWAVFFNodeTrackPortInfo& aTrackPortInfo);
        bool CheckAvailabilityForSendingNewTrackData(PVWAVFFNodeTrackPortInfo& aTrackPortInfo);
        void ResetAllTracks();
        bool ReleaseAllPorts();
        void CleanupFileSource();
        int32 PushBackPortActivity(PVMFPortActivity &aActivity);
        int32 CreateNewArray(char*& aPtr, int32 aLen);
        int32 PushBackKeyVal(Oscl_Vector<PvmiKvp, OsclMemAllocator>*& aValueListPtr, PvmiKvp &aKeyVal);
        PVMFStatus PushBackMetadataKeys(PVMFMetadataList *&aKeyListPtr, uint32 aLcv);

    private: // private member variables

        //stream id
        uint32 iStreamID;
        //bos flag
        bool iSendBOS;

        PVMFWAVFFNodeCmdQ iInputCommands;
        PVMFWAVFFNodeCmdQ iCurrentCommand;

        PVMFWAVFFParserOutPort* iOutPort;
        friend class PVMFWAVFFParserOutPort;

        PVMFNodeCapability iCapability;
        PVLogger *iLogger;

        PVWAVFileInfo wavinfo;

        uint32 trackdata_bufsize;
        uint32 trackdata_num_samples;

        PVMFPortInterface* iCmdRespPort;

        OSCL_wHeapString<OsclMemAllocator> iFilename;
        Oscl_FileServer iFileServer;
        PV_Wav_Parser* iWAVParser;

        Oscl_Vector<PVWAVFFNodeTrackPortInfo, OsclMemAllocator> iSelectedTrackList;

        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iAvailableMetadataKeys;

        // Reference counter for extension
        uint32 iExtensionRefCount;
        bool oOutgoingQueueBusy;
};

#endif // PVMF_WAVFFPARSER_NODE_H_INCLUDED

