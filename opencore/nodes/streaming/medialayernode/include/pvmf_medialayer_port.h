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
#ifndef PVMF_MEDIALAYER_PORT_H_INCLUDED
#define PVMF_MEDIALAYER_PORT_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif
#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#ifndef PAYLOAD_PARSER_H_INCLUDED
#include "payload_parser.h"
#endif
#ifndef PVMF_MEDIA_FRAG_GROU_H_INCLUDED
#include "pvmf_media_frag_group.h"
#endif

/**
 * Macros for calling PVLogger
 */
#define PVMF_MLNODE_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define PVMF_MLNODE_LOGWARNING(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_WARNING,m);
#define PVMF_MLNODE_LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_MLNODE_LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_MLNODE_LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_MLNODE_LOGINFO(m) PVMF_MLNODE_LOGINFOMED(m)
#define PVMF_MLNODE_LOGBIN(iPortLogger, m) PVLOGGER_LOGBIN(PVLOGMSG_INST_LLDBG, iPortLogger, PVLOGMSG_ERR, m);
#define PVMF_MLNODE_LOGDATATRAFFIC(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLogger,PVLOGMSG_INFO,m);
#define PVMF_MLNODE_LOGDATATRAFFIC_IN(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLoggerIn,PVLOGMSG_INFO,m);
#define PVMF_MLNODE_LOGDATATRAFFIC_OUT(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLoggerOut,PVLOGMSG_INFO,m);
#define PVMF_MLNODE_LOGDATATRAFFIC_E(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLogger,PVLOGMSG_ERR,m);
#define PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iDataPathLoggerFlowCtrl,PVLOGMSG_INFO,m);
#define PVMF_MLNODE_LOGDIAGNOSTICS(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF,iDiagnosticsLogger,PVLOGMSG_INFO,m);
#define PVMF_MLNODE_LOG_REPOS(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iReposLogger,PVLOGMSG_INFO,m);
#define PVMF_MLNODE_LOG_RUNL(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iRunlLogger,PVLOGMSG_INFO,m);


/** memory allocator type for this node */
typedef OsclMemAllocator PVMFMediaLayerNodeAllocator;

/** Enumerated list of port tags supported by this port */
typedef enum
{
    PVMF_MEDIALAYER_PORT_TYPE_UNKNOWN = -1,
    PVMF_MEDIALAYER_PORT_TYPE_INPUT = 0,
    PVMF_MEDIALAYER_PORT_TYPE_OUTPUT = 1
} PVMFMediaLayerNodePortTag;

// Capability mime strings
#define PVMF_MEDIA_LAYER_PORT_SPECIFIC_ALLOCATOR "x-pvmf/pvmfstreaming/socketmemallocator"
#define PVMF_MEDIA_LAYER_PORT_SPECIFIC_ALLOCATOR_VALTYPE "x-pvmf/pvmfstreaming/socketmemallocator;valtype=ksv"

/* Supported Port Input Formats */
#define RTP_PAYLOAD_RFC_3016_VISUAL 1
#define RTP_PAYLOAD_RFC_3016_AUDIO  2
#define RTP_PAYLOAD_RFC_3267_AMR    3
#define RTP_PAYLOAD_RFC_2429_H263   4
#define RTP_PAYLOAD_ASF             5

#define PVMF_MEDIA_LAYER_NODE_ASF_REPOS_TIME_OFFSET_IN_MS 2

/** values for FirstDataArrival */
#define PVMF_ASF_TRACK_NO_DATA 1
#define PVMI_ASF_TRACK_DATA_ARRIVAL 2
#define PVMI_ASF_TRACK_DROP 3

class PVMFMediaLayerPortContainer
{
    public:
        PVMFMediaLayerPortContainer()
        {
            id = -1;
            tag = PVMF_MEDIALAYER_PORT_TYPE_UNKNOWN;
            iPort = NULL;
            iPayLoadParser = NULL;
            oEOSReached = false;
            oFirstDataArrival = PVMF_ASF_TRACK_NO_DATA;
            oFormatSpecificInfoLogged = false;
            oUpStreamEOSRecvd = false;
            oProcessIncomingMessages = true;
            oProcessOutgoingMessages = true;
            iPrevMsgSeqNum = 0;
            iPrevMsgTimeStamp = 0;
            iPreRollInMS = 0;
            iLiveStream = false;
            iContinuousTimeStamp = 0;
            iFirstFrameAfterRepositioning = false;
            iFirstFrameAfterReposTimeStamp = 0;

            ipFragGroupMemPool  = NULL;
            ipFragGroupAllocator = NULL;

            iReConfig = false;
            oDetectBrokenTrack = false;
            oDisableTrack = false;
            oReconfigId = 0;

            iIsOneToN = false;
        };


        PVMFMediaLayerPortContainer(const PVMFMediaLayerPortContainer& a)
        {
            id = a.id;
            tag = a.tag;
            iPort = a.iPort;
            iPayLoadParser = a.iPayLoadParser;
            oEOSReached = a.oEOSReached;
            oFirstDataArrival = a.oFirstDataArrival;
            oFormatSpecificInfoLogged = a.oFormatSpecificInfoLogged;
            oUpStreamEOSRecvd = a.oUpStreamEOSRecvd;
            oProcessIncomingMessages = a.oProcessIncomingMessages;
            oProcessOutgoingMessages = a.oProcessOutgoingMessages;
            oDetectBrokenTrack = a.oDetectBrokenTrack;
            oDisableTrack = a.oDisableTrack;
            oReconfigId = a.oReconfigId;
            pvOutPortLookupTable = a.pvOutPortLookupTable;
            iPrevMsgSeqNum = a.iPrevMsgSeqNum;
            iPrevMsgTimeStamp = a.iPrevMsgTimeStamp;
            iPreRollInMS = a.iPreRollInMS;
            iLiveStream = a.iLiveStream;
            iIsOneToN = a.iIsOneToN;
            iMimeType = a.iMimeType;
            iTransportType = a.iTransportType;
            iTrackConfig = a.iTrackConfig;
            iPortLogger = a.iPortLogger;
            iLogFile = a.iLogFile;
            iBinAppenderPtr = a.iBinAppenderPtr;
            vCounterPorts = a.vCounterPorts;
            iContinuousTimeStamp = a.iContinuousTimeStamp;
            iFirstFrameAfterRepositioning = a.iFirstFrameAfterRepositioning;
            iFirstFrameAfterReposTimeStamp = a.iFirstFrameAfterReposTimeStamp;
            vAccessUnits = Oscl_Vector<IPayloadParser::Payload, OsclMemAllocator>(a.vAccessUnits);
            ipFragGroupMemPool  = a.ipFragGroupMemPool;;
            ipFragGroupAllocator = a.ipFragGroupAllocator;
            iReConfig = a.iReConfig;
        };

        PVMFMediaLayerPortContainer& operator=(const PVMFMediaLayerPortContainer& a)
        {
            if (&a != this)
            {
                id = a.id;
                tag = a.tag;
                iPort = a.iPort;
                iPayLoadParser = a.iPayLoadParser;
                oEOSReached = a.oEOSReached;
                oFirstDataArrival = a.oFirstDataArrival;
                oFormatSpecificInfoLogged = a.oFormatSpecificInfoLogged;
                oUpStreamEOSRecvd = a.oUpStreamEOSRecvd;
                oProcessIncomingMessages = a.oProcessIncomingMessages;
                oProcessOutgoingMessages = a.oProcessOutgoingMessages;
                oDetectBrokenTrack = a.oDetectBrokenTrack;
                oDisableTrack = a.oDisableTrack;
                oReconfigId = a.oReconfigId;
                pvOutPortLookupTable.clear();
                pvOutPortLookupTable = a.pvOutPortLookupTable;
                iPrevMsgSeqNum = a.iPrevMsgSeqNum;
                iPrevMsgTimeStamp = a.iPrevMsgTimeStamp;
                iPreRollInMS = a.iPreRollInMS;
                iLiveStream = a.iLiveStream;
                iIsOneToN = a.iIsOneToN;
                iMimeType = a.iMimeType;
                iTransportType = a.iTransportType;
                iTrackConfig = a.iTrackConfig;
                iPortLogger = a.iPortLogger;
                iLogFile = a.iLogFile;
                iBinAppenderPtr = a.iBinAppenderPtr;
                vCounterPorts = a.vCounterPorts;
                iContinuousTimeStamp = a.iContinuousTimeStamp;
                iFirstFrameAfterRepositioning = a.iFirstFrameAfterRepositioning;
                iFirstFrameAfterReposTimeStamp = a.iFirstFrameAfterReposTimeStamp;
                vAccessUnits = Oscl_Vector<IPayloadParser::Payload, OsclMemAllocator>(a.vAccessUnits);
                ipFragGroupMemPool  = a.ipFragGroupMemPool;
                ipFragGroupAllocator = a.ipFragGroupAllocator;
                iReConfig = a.iReConfig;
            }
            return *this;
        };

        virtual ~PVMFMediaLayerPortContainer()
        {
            pvOutPortLookupTable.clear();
            vAccessUnits.clear();
        };

        void CleanUp()
        {
            pvOutPortLookupTable.clear();
            vAccessUnits.clear();
        };

        void ResetParams()
        {
            oEOSReached = false;
            oFirstDataArrival = PVMF_ASF_TRACK_NO_DATA;
            oFormatSpecificInfoLogged = false;
            oUpStreamEOSRecvd = false;
            oProcessIncomingMessages = true;
            oProcessOutgoingMessages = true;
            oDetectBrokenTrack = false;
            oDisableTrack = false;
            oReconfigId = 0;
            iPrevMsgSeqNum = 0;
            iPrevMsgTimeStamp = 0;
            iFirstFrameAfterRepositioning = false;
            iFirstFrameAfterReposTimeStamp = 0;
            iReConfig = false;
            iContinuousTimeStamp = 0;
        }

        int32                      id;
        PVMFMediaLayerNodePortTag  tag;
        PVMFPortInterface*     iPort;

        // for input ports:
        //     - contains a vector of the associated output port indices
        // for output ports:
        //     - contains a single element, the associated input port index
        Oscl_Vector<int, OsclMemAllocator> vCounterPorts;
        // for 1-n flows using stream id based lookups
        Oscl_Vector<int, OsclMemAllocator> pvOutPortLookupTable;


        //
        // data for parsed payloads waiting to be sent
        //
        Oscl_Vector<IPayloadParser::Payload, OsclMemAllocator> vAccessUnits;
        PVUid32 iCurrFormatId;
        OsclRefCounterMemFrag iCurrFormatSpecInfo;


        IPayloadParser* iPayLoadParser;
        bool iIsOneToN;
        OSCL_HeapString<PVMFMediaLayerNodeAllocator> iMimeType;
        OSCL_HeapString<PVMFMediaLayerNodeAllocator> iTransportType;
        OsclRefCounterMemFrag iTrackConfig;
        PVLogger* iPortLogger;
        OSCL_HeapString<PVMFMediaLayerNodeAllocator> iLogFile;
        bool oUpStreamEOSRecvd;
        bool oEOSReached;
        uint32 oFirstDataArrival;
        bool oFormatSpecificInfoLogged;
        OsclSharedPtr<PVLoggerAppender> iBinAppenderPtr;
        bool oProcessIncomingMessages;
        bool oProcessOutgoingMessages;
        bool oDetectBrokenTrack;
        bool oDisableTrack;

        uint32 iPrevMsgSeqNum;
        uint32 iPrevMsgTimeStamp;
        uint32 iPreRollInMS;
        bool   iLiveStream;
        uint64 iContinuousTimeStamp;
        bool   iFirstFrameAfterRepositioning;
        uint32 iFirstFrameAfterReposTimeStamp;

        /*Send Reconfig*/
        uint32 oReconfigId;
        PVMFSharedMediaMsgPtr oMsgReconfig;

        // allocator for outgoing media frag groups
        OsclMemPoolFixedChunkAllocator* ipFragGroupMemPool;
        PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>* ipFragGroupAllocator;

        /* stream switching related */
        bool iReConfig;
};

/**
 *
 *
 */
class PVMFMediaLayerNode;
class PVMFMediaLayerPort : public PvmfPortBaseImpl,
            public PvmiCapabilityAndConfig,
            public OsclMemPoolFixedChunkAllocatorObserver
{
    public:
        /**
         * Default constructor. Default settings will be used for the data queues.
         * @param aId ID assigned to this port
         * @param aTag Port tag
         * @param aNode Container node
         */
        PVMFMediaLayerPort(int32 aTag,
                           PVMFNodeInterface* aNode,
                           const char*);

        /**
         * Constructor that allows the node to configure the data queues of this port.
         * @param aTag Port tag
         * @param aNode Container node
         * @param aSize Data queue capacity. The data queue size will not grow beyond this capacity.
         * @param aReserve Size of data queue for which memory is reserved. This must be
         * less than or equal to the capacity. If this is less than capacity, memory will be
         * allocated when the queue grows beyond the reserve size, but will stop growing at
         * capacity.
         * @param aThreshold Ready-to-receive threshold, in terms of percentage of the data queue capacity.
         * This value should be between 0 - 100.
         */
        PVMFMediaLayerPort(int32 aTag,
                           PVMFNodeInterface* aNode,
                           uint32 aInCapacity,
                           uint32 aInReserve,
                           uint32 aInThreshold,
                           uint32 aOutCapacity,
                           uint32 aOutReserve,
                           uint32 aOutThreshold,
                           const char*);

        /** Destructor */
        ~PVMFMediaLayerPort();

        /* Over ride QueryInterface - this port supports config interface */
        void QueryInterface(const PVUuid &aUuid, OsclAny*&aPtr)
        {
            if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
                aPtr = (PvmiCapabilityAndConfig*)this;
            else
                aPtr = NULL;
        }

        /* Over ride Connect() */
        PVMFStatus Connect(PVMFPortInterface* aPort);

        /* Implement pure virtuals from PvmiCapabilityAndConfig interface */
        PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                                     PvmiKvp*& aParameters, int& num_parameter_elements,	PvmiCapabilityContext aContext);
        PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                               int num_elements, PvmiKvp * & aRet_kvp);
        PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);

        /* Unsupported PvmiCapabilityAndConfig methods */
        void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
        {
            OSCL_UNUSED_ARG(aObserver);
        };
        void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
        }
        void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                                  PvmiKvp* aParameters, int num_parameter_elements)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(num_parameter_elements);
        }
        void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
        }
        PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                         int num_elements, PvmiKvp*& aRet_kvp, OsclAny* context = NULL)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(num_elements);
            OSCL_UNUSED_ARG(aRet_kvp);
            OSCL_UNUSED_ARG(context);
            return -1;
        }
        uint32 getCapabilityMetric(PvmiMIOSession aSession)
        {
            OSCL_UNUSED_ARG(aSession);
            return 0;
        }


        //from OsclMemPoolFixedChunkAllocatorObserver
        void freechunkavailable(OsclAny*) ;

        bool peekHead(PVMFSharedMediaDataPtr& dataPtr, bool& bEos);


        PVMFStatus pvmiVerifyPortFormatSpecificInfoSync(const char* aFormatValType,
                OsclAny* aConfig);

        //overrides from PVMFPortInterface
        PVMFStatus QueueOutgoingMsg(PVMFSharedMediaMsgPtr aMsg);
        bool IsOutgoingQueueBusy();

    private:
        bool pvmiSetPortFormatSpecificInfoSync(PvmiCapabilityAndConfig *aPort,
                                               const char* aFormatValType);

        bool pvmiGetPortFormatSpecificInfoSync(const char* aFormatValType,
                                               PvmiKvp*& aKvp);

        void Construct();

        PVLogger *iLogger;

        friend class Oscl_TAlloc<PVMFMediaLayerPort, PVMFMediaLayerNodeAllocator>;
        friend class PVMFMediaLayerNodeExtensionInterfaceImpl;

        PVMFMediaLayerNode *iMLNode;
        PVMFMediaLayerNodePortTag iPortType;



};

#endif // PVMF_MEDIALAYER_PORT_H_INCLUDED



