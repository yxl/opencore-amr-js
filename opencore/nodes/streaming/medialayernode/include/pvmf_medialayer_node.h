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
#ifndef PVMF_MEDIALAYER_NODE_H_INCLUDED
#define PVMF_MEDIALAYER_NODE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STR_PTR_LEN_H_INCLUDED
#include "oscl_str_ptr_len.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_EXCLUSIVE_PTR_H_INCLUDED
#include "oscl_exclusive_ptr.h"
#endif
#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif
#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVMF_MEDIALAYER_PORT_H_INCLUDED
#include "pvmf_medialayer_port.h"
#endif
#ifndef PVMF_SM_NODE_EVENTS_H_INCLUDED
#include "pvmf_sm_node_events.h"
#endif
#ifndef PAYLOAD_PARSER_REGISTRY_H_INCLUDED
#include "payload_parser_registry.h"
#endif
#ifndef PVMF_CPMPLUGIN_ACCESS_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_access_interface.h"
#endif

///////////////////////////////////////////////
//IDs for all of the asynchronous node commands.
///////////////////////////////////////////////
enum TPVMFMediaLayerNodeCommand
{
    PVMF_MEDIA_LAYER_NODE_QUERYUUID,
    PVMF_MEDIA_LAYER_NODE_QUERYINTERFACE,
    PVMF_MEDIA_LAYER_NODE_REQUESTPORT,
    PVMF_MEDIA_LAYER_NODE_RELEASEPORT,
    PVMF_MEDIA_LAYER_NODE_INIT,
    PVMF_MEDIA_LAYER_NODE_PREPARE,
    PVMF_MEDIA_LAYER_NODE_START,
    PVMF_MEDIA_LAYER_NODE_STOP,
    PVMF_MEDIA_LAYER_NODE_FLUSH,
    PVMF_MEDIA_LAYER_NODE_PAUSE,
    PVMF_MEDIA_LAYER_NODE_RESET,
    PVMF_MEDIA_LAYER_NODE_CANCELALLCOMMANDS,
    PVMF_MEDIA_LAYER_NODE_CANCELCOMMAND,
    //add media layer node specific commands here
    PVMF_MEDIA_LAYER_NODE_COMMAND_LAST
};

#define PVMF_MEDIA_LAYER_NEW(auditCB,T,params,ptr)\
{\
ptr = OSCL_NEW(T,params);\
}

#define PVMF_MEDIA_LAYER_DELETE(auditCB,T,ptr)\
{\
OSCL_DELETE(ptr);\
}

/** Node command type */
typedef PVMFGenericNodeCommand<PVMFMediaLayerNodeAllocator> PVMFMediaLayerNodeCommandBase;

class PVMFMediaLayerNodeCommand: public PVMFMediaLayerNodeCommandBase
{
    public:
        void Construct(PVMFSessionId s, int32 cmd, int32 arg1, int32 arg2, int32& arg3, const OsclAny*aContext)
        {
            PVMFMediaLayerNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)arg1;
            iParam2 = (OsclAny*)arg2;
            iParam3 = (OsclAny*) & arg3;
        }
        void Parse(int32&arg1, int32&arg2, int32*&arg3)
        {
            arg1 = (int32)iParam1;
            arg2 = (int32)iParam2;
            arg3 = (int32*)iParam3;
        }
};

class PVMFMediaLayerNodeLoggerDestructDealloc : public OsclDestructDealloc
{
    public:
        void destruct_and_dealloc(OsclAny* ptr)
        {
            PVLoggerAppender* p = OSCL_REINTERPRET_CAST(PVLoggerAppender*, ptr);
            BinaryFileAppender* binPtr = OSCL_REINTERPRET_CAST(BinaryFileAppender*, p);
            if (!binPtr)
                return;
            OSCL_DELETE(binPtr);
        }
};

/** Command queue type */
typedef PVMFNodeCommandQueue < PVMFMediaLayerNodeCommand,
PVMFMediaLayerNodeAllocator > MediaLayerNodeCmdQ;

/** Starting value for command IDs */
#define MEDIA_LAYER_NODE_CMD_START 6200

/** Default vector reserve size */
#define MEDIA_LAYER_NODE_CMD_QUE_RESERVE 10

/** Default vector reserve size */
#define MEDIA_LAYER_NODE_VECTOR_RESERVE 20

class mediaInfo;

class PVMFMediaLayerNodeExtensionInterface : public PVInterface
{
    public:
        OSCL_IMPORT_REF virtual PVMFStatus setDRMDecryptionInterface(uint32 maxPacketSize, PVMFCPMPluginAccessUnitDecryptionInterface* aDecryptionInterface = NULL) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus setPayloadParserRegistry(PayloadParserRegistry*) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus setPortDataLogging(bool logEnable, OSCL_String* logPath = NULL) = 0;
        OSCL_IMPORT_REF virtual bool setPlayRange(int32 aStartTimeInMS,
                int32 aStopTimeInMS,
                bool oRepositioning = false) = 0;
        OSCL_IMPORT_REF virtual bool setPortMediaParams(PVMFPortInterface* aPort,
                OsclRefCounterMemFrag& aConfig,
                mediaInfo* aMediaInfo = NULL) = 0;
        OSCL_IMPORT_REF virtual bool setOutPortStreamParams(PVMFPortInterface* aPort,
                uint streamid,
                uint32 aPreroll,
                bool aLiveStream = false) = 0;
        OSCL_IMPORT_REF virtual PVMFStatus verifyPortConfigParams(const char* aFormatValType,
                PVMFPortInterface* aPort,
                OsclAny* aConfig,
                mediaInfo* aMediaInfo = NULL) = 0;
        OSCL_IMPORT_REF virtual void setInPortReposFlag(PVMFPortInterface* aPort, uint32 aSeekTimeInMS = 0) = 0;
        OSCL_IMPORT_REF virtual uint32 getMaxOutPortTimestamp(PVMFPortInterface* aPort,
                bool oPeek = false) = 0;
        OSCL_IMPORT_REF virtual bool setClientPlayBackClock(PVMFMediaClock* aClientPlayBackClock) = 0;
        OSCL_IMPORT_REF virtual void addRef() = 0;
        OSCL_IMPORT_REF virtual void removeRef() = 0;
        OSCL_IMPORT_REF virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;
        OSCL_IMPORT_REF virtual bool setTrackDisable(PVMFPortInterface* aPort) = 0;
        OSCL_IMPORT_REF virtual void setMediaLayerTimerDurationMS(uint32 aTimer) = 0;
};

/** Mimetype and Uuid for the extension interface */
#define PVMF_MEDIALAYER_CUSTOMINTERFACE_MIMETYPE "pvxxx/PVMFMediaLayerNode/CustomInterface"
#define PVMF_MEDIALAYER_MIMETYPE "pvxxx/PVMFMediaLayerNode"
#define PVMF_MEDIALAYER_BASEMIMETYPE "pvxxx"
#define PVMF_MEDIALAYERNODE_EXTENSIONINTERFACE_UUID PVUuid(0x78361150,0x8d35,0x48de,0x8f,0xc8,0x85,0xcb,0xe7,0xbf,0x1c,0x8a)

class PVMFMediaLayerNode;

class PVMFMediaLayerNodeExtensionInterfaceImpl :
            public PVInterfaceImpl<PVMFMediaLayerNodeAllocator>,
            public PVMFMediaLayerNodeExtensionInterface
{
    public:
        PVMFMediaLayerNodeExtensionInterfaceImpl(PVMFMediaLayerNode*);
        ~PVMFMediaLayerNodeExtensionInterfaceImpl();

        virtual PVMFStatus setDRMDecryptionInterface(uint32 maxPacketSize, PVMFCPMPluginAccessUnitDecryptionInterface* aDecryptionInterface = NULL);

        virtual PVMFStatus setPayloadParserRegistry(PayloadParserRegistry*);

        virtual PVMFStatus setPortDataLogging(bool logEnable, OSCL_String* logPath = NULL);

        virtual bool setClientPlayBackClock(PVMFMediaClock* aClientPlayBackClock);

        void addRef()
        {
            PVInterfaceImpl<PVMFMediaLayerNodeAllocator>::addRef();
        }
        void removeRef()
        {
            PVInterfaceImpl<PVMFMediaLayerNodeAllocator>::removeRef();
        }
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface)
        {
            if (uuid == Uuid())
            {
                addRef();
                iface = this;
                return true;
            }
            else
            {
                iface = NULL;
                return false;
            }
        }

        bool setPlayRange(int32 aStartTimeInMS,
                          int32 aStopTimeInMS,
                          bool oRepositioning = false);

        bool setPortMediaParams(PVMFPortInterface* aPort,
                                OsclRefCounterMemFrag& aConfig,
                                mediaInfo* aMediaInfo = NULL);

        PVMFStatus verifyPortConfigParams(const char* aFormatValType,
                                          PVMFPortInterface* aPort,
                                          OsclAny* aConfig,
                                          mediaInfo* aMediaInfo = NULL);

        bool setOutPortStreamParams(PVMFPortInterface* aPort, uint streamid, uint32 aPreroll, bool aLiveStream = false);

        void setInPortReposFlag(PVMFPortInterface* aPort, uint32 aSeekTimeInMS = 0);
        uint32 getMaxOutPortTimestamp(PVMFPortInterface* aPort, bool oPeek = false);
        bool setTrackDisable(PVMFPortInterface* aPort);
        void setMediaLayerTimerDurationMS(uint32 aTimer);

    private:
        PVMFMediaLayerNode *iContainer;
        friend class PVMFMediaLayerNode;
};

class PVLogger;

class PVMFMediaLayerNode : public PVInterface,
            public PVMFNodeInterface,
            public OsclActiveObject,
            public PVMFNodeErrorEventObserver,
            public PVMFNodeInfoEventObserver,
            public PVMFNodeCmdStatusObserver
{
    public:
        OSCL_IMPORT_REF PVMFMediaLayerNode(int32 aPriority);
        OSCL_IMPORT_REF virtual ~PVMFMediaLayerNode();

        OSCL_IMPORT_REF PVMFStatus ThreadLogon();
        OSCL_IMPORT_REF PVMFStatus ThreadLogoff();
        OSCL_IMPORT_REF PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        OSCL_IMPORT_REF PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);
        OSCL_IMPORT_REF PVMFCommandId RequestPort(PVMFSessionId aSession,
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


        OSCL_IMPORT_REF PVMFCommandId QueryUUID(PVMFSessionId, const PvmfMimeString& aMimeType,
                                                Oscl_Vector<PVUuid, PVMFMediaLayerNodeAllocator>& aUuids,
                                                bool aExactUuidsOnly = false,
                                                const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFCommandId QueryInterface(PVMFSessionId, const PVUuid& aUuid,
                PVInterface*& aInterfacePtr,
                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF void setInPortReposFlag(PVMFPortInterface* aPort, uint32 aSeekTimeInMS = 0);
        OSCL_IMPORT_REF uint32 getMaxOutPortTimestamp(PVMFPortInterface* aPort, bool oPeek = false);

        /**
         * Handle an error event that has been generated.
         *
         * @param "aEvent" "The event to be handled."
         */
        virtual void HandleNodeErrorEvent(const PVMFAsyncEvent& aEvent)
        {
            OSCL_UNUSED_ARG(aEvent);
        }
        /**
         * Handle an informational event that has been generated.
         *
         * @param "aEvent" "The event to be handled."
         */
        virtual void HandleNodeInformationalEvent(const PVMFAsyncEvent& aEvent)
        {
            OSCL_UNUSED_ARG(aEvent);
        }
        /**
         * Handle an event that has been generated.
         *
         * @param "aResponse"	"The response to a previously issued command."
         */
        virtual void NodeCommandCompleted(const PVMFCmdResp& aResponse)
        {
            OSCL_UNUSED_ARG(aResponse);
        }

        /** from PVMFPortActivityHandler */
        void HandlePortActivity(const PVMFPortActivity& aActivity);

        //callback from the port when memory is available in the rtp
        //payload parser.
        void freechunkavailable(PVMFPortInterface*);
        virtual void addRef()
        {
        }
        virtual void removeRef()
        {
        }
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface)
        {
            iface = NULL;
            if (uuid == PVUuid(PVMF_MEDIALAYERNODE_EXTENSIONINTERFACE_UUID))
            {
                if (!iExtensionInterface)
                {
                    PVMFMediaLayerNodeAllocator alloc;
                    int32 err;
                    OsclAny*ptr = NULL;
                    OSCL_TRY(err,
                             ptr = alloc.ALLOCATE(sizeof(PVMFMediaLayerNodeExtensionInterfaceImpl));
                            );
                    if (err != OsclErrNone || !ptr)
                    {
                        PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::queryInterface: Error - Out of memory"));
                        OSCL_LEAVE(OsclErrNoMemory);
                    }
                    iExtensionInterface =
                        OSCL_PLACEMENT_NEW(ptr, PVMFMediaLayerNodeExtensionInterfaceImpl(this));
                }
                return (iExtensionInterface->queryInterface(uuid, iface));
            }
            else
            {
                return false;
            }
        }

    private:
        /* from OsclActiveObject */
        void Run();
        void DoCancel();

        /**
         * Process a port activity. This method is called by Run to process a port activity.
         *
         */
        bool ProcessPortActivity(PVMFMediaLayerPortContainer*);

        /**
         * Retrieve and process an incoming message from a port.
         */
        PVMFStatus ProcessIncomingMsg(PVMFMediaLayerPortContainer*);
        PVMFStatus ProcessInputMsg_OneToOne(PVMFMediaLayerPortContainer* pinputPort,
                                            PVMFSharedMediaMsgPtr msgIn);
        PVMFStatus ProcessInputMsg_OneToN(PVMFMediaLayerPortContainer* pinputPort,
                                          PVMFSharedMediaMsgPtr msgIn);
        bool checkOutputPortsBusy(PVMFMediaLayerPortContainer* pinputPort);
        PVMFStatus sendAccessUnits(PVMFMediaLayerPortContainer* pinputPort);
        PVMFStatus dispatchAccessUnits(PVMFMediaLayerPortContainer* pinputPort,
                                       PVMFMediaLayerPortContainer* poutPort);

        bool Allocate(OsclSharedPtr<PVMFMediaDataImpl>& mediaDataImplOut, PVMFMediaLayerPortContainer* poutPort);
        bool Allocate(OsclAny*& ptr);
        bool Push(PVMFMediaLayerPortContainer portParams);
        bool AddPort(PVMFMediaLayerPort* port);

        /**
         * Process an outgoing message of a the specified port by sending the message to
         * the receiving side.
         */
        PVMFStatus ProcessOutgoingMsg(PVMFMediaLayerPortContainer*);

        void QueuePortActivity(PVMFMediaLayerPortContainer*,
                               const PVMFPortActivity&);

        bool CheckForPortRescheduling();
        bool CheckForPortActivityQueues();


        /* Port related routines */
        IPayloadParser* CreatePayLoadParser(PvmfMimeString* aPortConfig);
        void DestroyPayLoadParser(PvmfMimeString* aPortConfig,
                                  IPayloadParser* aParser);
        bool GetPortContainer(PVMFPortInterface* aPort,
                              PVMFMediaLayerPortContainer& aContainer);
        bool GetPortContainer(PVMFPortInterface* aPort, int& index);
        bool GetPortContainer(PVMFPortInterface* aPort,
                              PVMFMediaLayerPortContainer*& aContainer);

        PVMFPortInterface* getPortCounterpart(PVMFMediaLayerPortContainer);
        PVMFPortInterface* getPortCounterpart(PVMFMediaLayerPortContainer*);

        /* Command processing */
        PVMFCommandId QueueCommandL(PVMFMediaLayerNodeCommand&);
        void MoveCmdToCurrentQueue(PVMFMediaLayerNodeCommand& aCmd);
        bool ProcessCommand(PVMFMediaLayerNodeCommand&);
        void CommandComplete(MediaLayerNodeCmdQ&,
                             PVMFMediaLayerNodeCommand&,
                             PVMFStatus,
                             OsclAny* aData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);
        bool FlushPending();

        /** Command handlers */
        void DoReset(PVMFMediaLayerNodeCommand&);
        void DoQueryUuid(PVMFMediaLayerNodeCommand&);
        void DoQueryInterface(PVMFMediaLayerNodeCommand&);
        void DoRequestPort(PVMFMediaLayerNodeCommand&);
        void DoReleasePort(PVMFMediaLayerNodeCommand&);
        void DoInit(PVMFMediaLayerNodeCommand&);
        void DoPrepare(PVMFMediaLayerNodeCommand&);
        void DoStart(PVMFMediaLayerNodeCommand&);
        void DoStop(PVMFMediaLayerNodeCommand&);
        void DoFlush(PVMFMediaLayerNodeCommand&);
        void DoPause(PVMFMediaLayerNodeCommand&);
        void DoCancelAllCommands(PVMFMediaLayerNodeCommand&);
        void DoCancelCommand(PVMFMediaLayerNodeCommand&);

        /* Event reporting */
        void ReportErrorEvent(PVMFEventType aEventType,
                              OsclAny* aEventData = NULL,
                              PVUuid* aEventUUID = NULL,
                              int32* aEventCode = NULL);
        void ReportInfoEvent(PVMFEventType aEventType,
                             OsclAny* aEventData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);
        void SetState(TPVMFNodeInterfaceState);

    private:
        PVMFPortVector<PVMFMediaLayerPort, PVMFMediaLayerNodeAllocator> iPortVector;
        /*
         * Queue holding port params - one per every port
         */
        Oscl_Vector<PVMFMediaLayerPortContainer, PVMFMediaLayerNodeAllocator> iPortParamsQueue;

        MediaLayerNodeCmdQ iInputCommands;
        MediaLayerNodeCmdQ iCurrentCommand;
        uint32 iStreamID;

        PVMFNodeCapability iCapability;
        PVLogger *iLogger;
        PVLogger *iDataPathLogger;
        PVLogger *iDataPathLoggerIn;
        PVLogger *iDataPathLoggerOut;
        PVLogger *iDataPathLoggerFlowCtrl;
        PVLogger *iClockLogger;
        PVLogger *iReposLogger;
        PVLogger *iRunlLogger;
        OsclErrorTrapImp* iOsclErrorTrapImp;
        bool     oPortDataLog;
        char     iLogFileIndex;
        OSCL_HeapString<PVMFMediaLayerNodeAllocator> portLogPath;

        PVMFMediaLayerNodeExtensionInterfaceImpl* iExtensionInterface;
        /*
         * Extension interface related
         */
        uint8* srcPtr;
        uint32 maxPacketSize;
        PVMFCPMPluginAccessUnitDecryptionInterface* iDecryptionInterface;
        PVMFStatus setDRMDecryptionInterface(uint32 packetsize, PVMFCPMPluginAccessUnitDecryptionInterface* aDecryptionInterface)
        {
            maxPacketSize = packetsize;
            iDecryptionInterface = aDecryptionInterface;
            srcPtr = (uint8 *)oscl_malloc(maxPacketSize * sizeof(uint8));
            return PVMFSuccess;
        }

        PayloadParserRegistry* iPayLoadParserRegistry;
        void setPayloadParserRegistry(PayloadParserRegistry* registry)
        {
            iPayLoadParserRegistry = registry;
        }
        void createRTPPayloadParserRegistry();
        void destroyRTPPayloadParserRegistry();

        IPayloadParser::Payload iPayLoad;

        PVMFStatus setPortDataLogging(bool logEnable, OSCL_String* logPath)
        {
            oPortDataLog = logEnable;
            if (logPath != NULL)
            {
                portLogPath = logPath->get_cstr();
            }
            return PVMFSuccess;
        }

        void LogMediaData(PVMFSharedMediaDataPtr, PVMFPortInterface*);

        friend class PVMFMediaLayerPort;
        friend class PVMFMediaLayerNodeExtensionInterfaceImpl;

        int32 iPlayStartTime;
        int32 iPlayStopTime;
        uint64 preroll64;

        bool CheckForEOS();
        bool setPlayRange(int32 aStartTimeInMS,
                          int32 aStopTimeInMS,
                          bool oRepositioning = false);

        PVMFMediaClock* iClientPlayBackClock;
        bool setClientPlayBackClock(PVMFMediaClock* aClientPlayBackClock)
        {
            iClientPlayBackClock = aClientPlayBackClock;
            return true;
        }

        bool setPortMediaParams(PVMFPortInterface* aPort,
                                OsclRefCounterMemFrag& aConfig,
                                mediaInfo* aMediaInfo = NULL);

        PVMFStatus verifyPortConfigParams(const char* aFormatValType,
                                          PVMFPortInterface* aPort,
                                          OsclAny* aConfig,
                                          mediaInfo* aMediaInfo = NULL);

        bool setOutPortStreamParams(PVMFPortInterface* aPort, uint streamid, uint32 aPreroll, bool aLiveStream = false);

        bool parseOutputPortMime(OSCL_String* pmime,
                                 uint& inputPort);

        /*reposiiotning*/
        uint32 diffAudioVideoTS;
        uint32* iTimeTakenTSptr;
        bool iAdjustTimeReady;
        uint32 iReposTime;

        /* Send EOS*/
        PVMFStatus checkPortCounterpartAccessUnitQueue(PVMFMediaLayerPortContainer* pinputPort, bool* IsAccessUnitsEmpty);
        PVMFStatus sendEndOfTrackCommand(PVMFMediaLayerPortContainer* poutputPort);
        bool iDetectBrokenTrack;
        bool setTrackDisable(PVMFPortInterface* aPort);
        bool oEOSsendunits;

        /* Set timer */
        uint32 iTimerNoDataTrack;
        void setMediaLayerTimerDurationMS(uint32 aTimer)
        {
            iTimerNoDataTrack = aTimer;
        }

        PVMFStatus LogPayLoadParserStats();

        PVLogger* iDiagnosticsLogger;
        bool iDiagnosticsLogged;
        void LogSessionDiagnostics();
        uint32 iNumRunL;
};

#endif //MEDIALAYER_NODE_H_INCLUDED


