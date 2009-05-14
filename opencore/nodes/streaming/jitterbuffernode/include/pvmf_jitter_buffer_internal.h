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
#ifndef PVMF_JITTER_BUFFER_INTERNAL_H_INCLUDED
#define PVMF_JITTER_BUFFER_INTERNAL_H_INCLUDED

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif
#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_JITTER_BUFFER_EXT_INTERFACE_H_INCLUDED
#include "pvmf_jitter_buffer_ext_interface.h"
#endif

/**
 * Macros for calling PVLogger
 */
#define PVMF_JBNODE_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipLogger,PVLOGMSG_ERR,m);
#define PVMF_JBNODE_LOGWARNING(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipLogger,PVLOGMSG_WARNING,m);
#define PVMF_JBNODE_LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,ipLogger,PVLOGMSG_INFO,m);
#define PVMF_JBNODE_LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,ipLogger,PVLOGMSG_INFO,m);
#define PVMF_JBNODE_LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,ipLogger,PVLOGMSG_INFO,m);
#define PVMF_JBNODE_LOGINFO(m) PVMF_JBNODE_LOGINFOMED(m)
#define PVMF_JBNODE_LOGDATATRAFFIC(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipDataPathLogger,PVLOGMSG_INFO,m);
#define PVMF_JBNODE_LOGDATATRAFFIC_IN(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipDataPathLoggerIn,PVLOGMSG_INFO,m);
#define PVMF_JBNODE_LOGDATATRAFFIC_IN_E(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipDataPathLoggerIn,PVLOGMSG_ERR,m);
#define PVMF_JBNODE_LOGDATATRAFFIC_OUT(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipDataPathLoggerOut,PVLOGMSG_INFO,m);
#define PVMF_JBNODE_LOGDATATRAFFIC_OUT_E(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipDataPathLoggerOut,PVLOGMSG_ERR,m);
#define PVMF_JBNODE_LOGCLOCK(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipClockLogger,PVLOGMSG_INFO,m);
#define PVMF_JBNODE_LOGCLOCK_SESSION_DURATION(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipClockLoggerSessionDuration,PVLOGMSG_INFO,m);
#define PVMF_JBNODE_LOGCLOCK_REBUFF(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipClockLoggerRebuff,PVLOGMSG_INFO,m);
#define PVMF_JBNODE_LOGDIAGNOSTICS(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF,ipDiagnosticsLogger,PVLOGMSG_INFO,m);
#define PVMF_JBNODE_LOGDATATRAFFIC_FLOWCTRL(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipDataPathLoggerFlowCtrl,PVLOGMSG_INFO,m);
#define PVMF_JBNODE_LOGDATATRAFFIC_FLOWCTRL_E(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipDataPathLoggerFlowCtrl,PVLOGMSG_ERR,m);
#define PVMF_JBNODE_LOG_RTCP_DATAPATH(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipDataPathLoggerRTCP,PVLOGMSG_ERR,m);
#define PVMF_JBNODE_LOG_EVENTS_CLOCK(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,ipJBEventsClockLogger ,PVLOGMSG_INFO,m);
#define PVMF_JBNODE_LOG_RTCP_AVSYNC(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,ipRTCPAVSyncLogger,PVLOGMSG_INFO,m);

#define PVMF_JITTER_BUFFER_NEW(auditCB,T,params,ptr)\
{\
ptr = OSCL_NEW(T,params);\
}

#define PVMF_JITTER_BUFFER_DELETE(auditCB,T,ptr)\
{\
OSCL_DELETE(ptr);\
}

#define PVMF_JITTER_BUFFER_TEMPLATED_DELETE(auditCB, T, Tsimple, ptr)\
{\
OSCL_DELETE(ptr);\
}

//Default vector reserve size
#define PVMF_JITTER_BUFFER_VECTOR_RESERVE 10

//Starting value for command IDs
#define PVMF_JITTER_BUFFER_NODE_COMMAND_ID_START 10000

#define PVMF_JITTER_BUFFER_INTERNAL_CMDQ_SIZE 20

//Node command type.
typedef PVMFGenericNodeCommand<OsclMemAllocator> PVMFJitterBufferNodeCommandBase;

class PVMFJitterBufferNodeCommand : public PVMFJitterBufferNodeCommandBase
{
    public:
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       int32 arg1,
                       int32 arg2,
                       int32& arg3,
                       const OsclAny*aContext)
        {
            PVMFJitterBufferNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)arg1;
            iParam2 = (OsclAny*)arg2;
            iParam3 = (OsclAny*) & arg3;
        };

        void Parse(int32&arg1, int32&arg2, int32*&arg3)
        {
            arg1 = (int32)iParam1;
            arg2 = (int32)iParam2;
            arg3 = (int32*)iParam3;
        };
};

//Command queue type
typedef PVMFNodeCommandQueue<PVMFJitterBufferNodeCommand, OsclMemAllocator> PVMFJitterBufferNodeCmdQ;

typedef struct tagPVMFJBCommandContext
{
    int32 cmd;
    bool  oFree;
} PVMFJBCommandContext;

///////////////////////////////////////////////////////
// For Extension Interface implementation
///////////////////////////////////////////////////////

//implementation class for extension interface
class PVMFJitterBufferNode;

class PVMFJitterBufferExtensionInterfaceImpl :
            public PVInterfaceImpl<OsclMemAllocator>,
            public PVMFJitterBufferExtensionInterface
{
    public:
        PVMFJitterBufferExtensionInterfaceImpl(PVMFJitterBufferNode*);
        ~PVMFJitterBufferExtensionInterfaceImpl();

        OSCL_IMPORT_REF void setRTCPIntervalInMicroSecs(uint32 aRTCPInterval);

        OSCL_IMPORT_REF bool setPortParams(PVMFPortInterface* aPort,
                                           uint32 aTimeScale,
                                           uint32 aBitRate,
                                           OsclRefCounterMemFrag& aConfig,
                                           bool aRateAdaptation = false,
                                           uint32 aRateAdaptationFeedBackFrequency = 0);

        OSCL_IMPORT_REF bool setPlayRange(int32 aStartTimeInMS,
                                          int32 aStopTimeInMS,
                                          bool oPlayAfterASeek,
                                          bool aStopTimeAvailable = true);

        OSCL_IMPORT_REF void setPlayBackThresholdInMilliSeconds(uint32 threshold);

        OSCL_IMPORT_REF void setJitterBufferRebufferingThresholdInMilliSeconds(uint32 aThreshold);
        OSCL_IMPORT_REF void getJitterBufferRebufferingThresholdInMilliSeconds(uint32& aThreshold);
        OSCL_IMPORT_REF void setJitterBufferDurationInMilliSeconds(uint32 duration);
        OSCL_IMPORT_REF void getJitterBufferDurationInMilliSeconds(uint32& duration);

        OSCL_IMPORT_REF void setEarlyDecodingTimeInMilliSeconds(uint32 duration);
        OSCL_IMPORT_REF void setBurstThreshold(float burstThreshold);

        OSCL_IMPORT_REF void setClientPlayBackClock(PVMFMediaClock* clientClock);
        OSCL_IMPORT_REF void setMaxInactivityDurationForMediaInMs(uint32 duration);
        OSCL_IMPORT_REF void getMaxInactivityDurationForMediaInMs(uint32& duration);

        OSCL_IMPORT_REF bool PrepareForRepositioning(bool oUseExpectedClientClockVal = false,
                uint32 aExpectedClientClockVal = 0);

        OSCL_IMPORT_REF bool setPortSSRC(PVMFPortInterface* aPort, uint32 aSSRC);

        OSCL_IMPORT_REF bool setPortRTPParams(PVMFPortInterface* aPort,
                                              bool   aSeqNumBasePresent,
                                              uint32 aSeqNumBase,
                                              bool   aRTPTimeBasePresent,
                                              uint32 aRTPTimeBase,
                                              bool   aNPTTimeBasePresent,
                                              uint32 aNPTInMS,
                                              bool oPlayAfterASeek = false);

        OSCL_IMPORT_REF bool setPortRTCPParams(PVMFPortInterface* aPort,
                                               int aNumSenders,
                                               uint32 aRR,
                                               uint32 aRS);

        OSCL_IMPORT_REF PVMFTimestamp getActualMediaDataTSAfterSeek();
        OSCL_IMPORT_REF PVMFTimestamp getMaxMediaDataTS();

        void addRef()
        {
            PVInterfaceImpl<OsclMemAllocator>::addRef();
        }
        void removeRef()
        {
            PVInterfaceImpl<OsclMemAllocator>::removeRef();
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

        OSCL_IMPORT_REF PVMFStatus setServerInfo(PVMFJitterBufferFireWallPacketInfo& aServerInfo);

        OSCL_IMPORT_REF PVMFStatus NotifyOutOfBandEOS();
        OSCL_IMPORT_REF PVMFStatus SendBOSMessage(uint32 aStramID);

        OSCL_IMPORT_REF void SetSharedBufferResizeParams(uint32 maxNumResizes, uint32 resizeSize);
        OSCL_IMPORT_REF void GetSharedBufferResizeParams(uint32& maxNumResizes, uint32& resizeSize);

        OSCL_IMPORT_REF bool ClearJitterBuffer(PVMFPortInterface* aPort, uint32 aSeqNum);
        OSCL_IMPORT_REF void FlushJitterBuffer();

        OSCL_IMPORT_REF bool NotifyAutoPauseComplete();

        OSCL_IMPORT_REF bool NotifyAutoResumeComplete();

        OSCL_IMPORT_REF PVMFStatus SetInputMediaHeaderPreParsed(PVMFPortInterface* aPort,
                bool aHeaderPreParsed);

        OSCL_IMPORT_REF PVMFStatus HasSessionDurationExpired(bool& aExpired);
        OSCL_IMPORT_REF bool PurgeElementsWithNPTLessThan(NptTimeFormat &aNPTTime);

        OSCL_IMPORT_REF void SetBroadCastSession();
        OSCL_IMPORT_REF void DisableFireWallPackets();


        OSCL_IMPORT_REF void StartOutputPorts();
        OSCL_IMPORT_REF void StopOutputPorts();
        OSCL_IMPORT_REF void UpdateJitterBufferState();
        OSCL_IMPORT_REF virtual void SetJitterBufferMemPoolInfo(const PvmfPortBaseImpl* aPort, uint32 aSize, uint32 aResizeSize, uint32 aMaxNumResizes, uint32 aExpectedNumberOfBlocksPerBuffer);
        OSCL_IMPORT_REF virtual void GetJitterBufferMemPoolInfo(const PvmfPortBaseImpl* aPort, uint32& aSize, uint32& aResizeSize, uint32& aMaxNumResizes, uint32& aExpectedNumberOfBlocksPerBuffer) const;
        OSCL_IMPORT_REF void SetJitterBufferChunkAllocator(OsclMemPoolResizableAllocator* aDataBufferAllocator, const PVMFPortInterface* aPort);
        OSCL_IMPORT_REF virtual bool PrepareForPlaylistSwitch();

    private:
        PVMFJitterBufferNode *iContainer;
        friend class PVMFJitterBufferNode;
};

#endif


