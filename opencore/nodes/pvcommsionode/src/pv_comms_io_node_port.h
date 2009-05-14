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
/**
 *
 * @file pvmf_media_input_node_outport.h
 * @brief Output port for media io interface wrapper node
 *
 */

#ifndef PV_COMMS_IO_NODE_PORT_H_INCLUDED
#define PV_COMMS_IO_NODE_PORT_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif
#ifndef PVMI_MEDIA_TRANSFER_H_INCLUDED
#include "pvmi_media_transfer.h"
#endif
#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
#endif
#ifndef PVMI_MIO_COMM_DATA_BUFFER_H_INCLUDED
#include "pvmi_mio_comm_data_buffer.h"
#endif

// TEMP -RH TSC deletes the allocators while comm node holds buffers
// so copy them for now to avoid holding onto TSC fragments.
#define USE_COPY_BUFFER
#ifdef USE_COPY_BUFFER
#define NUM_COPY_BUFFERS 2
#define COPY_BUFFER_SIZE 1024
#endif

// Forward declaration
class PVCommsIONode;

class PVCommsIONodePort : public OsclTimerObject,
            public PvmfPortBaseImpl,
            public PvmiMediaTransfer,
            public PVMFPortActivityHandler,
            public PvmiCapabilityAndConfig
{
    public:
        PVCommsIONodePort(int32 aPortTag, PVCommsIONode* aNode);
        ~PVCommsIONodePort();

        OSCL_IMPORT_REF void Start();
        OSCL_IMPORT_REF void MediaIOStarted();
        OSCL_IMPORT_REF void Pause();
        OSCL_IMPORT_REF void Stop();

        OSCL_IMPORT_REF PVMFStatus Configure(PVMFFormatType aPortProperty);

        // these override the PvmfPortBaseImpl routines
        OSCL_IMPORT_REF PVMFStatus Connect(PVMFPortInterface* aPort);
        OSCL_IMPORT_REF PVMFStatus Disconnect();
        OSCL_IMPORT_REF PVMFStatus PeerConnect(PVMFPortInterface* aPort);
        OSCL_IMPORT_REF PVMFStatus PeerDisconnect();
        void QueryInterface(const PVUuid &aUuid, OsclAny*&aPtr)
        {
            if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
            {
                aPtr = (PvmiCapabilityAndConfig*)this;
            }
            else
            {
                aPtr = NULL;
            }
        }

        //from PVMFPortActivityHandler
        void HandlePortActivity(const PVMFPortActivity& aActivity);

        // Pure virtuals from PvmiMediaTransfer
        void setPeer(PvmiMediaTransfer *aPeer);
        void useMemoryAllocators(OsclMemAllocator* write_alloc = NULL);
        PVMFCommandId writeAsync(uint8 format_type, int32 format_index, uint8* data, uint32 data_len,
                                 const PvmiMediaXferHeader& data_header_info, OsclAny* aContext = NULL);
        void writeComplete(PVMFStatus aStatus, PVMFCommandId  write_cmd_id, OsclAny* aContext);
        PVMFCommandId readAsync(uint8* data, uint32 max_data_len, OsclAny* aContext = NULL,
                                int32* formats = NULL, uint16 num_formats = 0);
//	void readComplete(PVMFStatus aStatus, PVMFCommandId  read_cmd_id, int32 format_index,
        //uint32 seq_num, uint32 flags, const PVMFTimestamp& timestamp, OsclAny* aContext);
        void readComplete(PVMFStatus aStatus, PVMFCommandId  read_cmd_id, int32 format_index,
                          const PvmiMediaXferHeader& data_header_info, OsclAny* aContext);

        void statusUpdate(uint32 status_flags);
        void cancelCommand(PVMFCommandId  command_id);
        void cancelAllCommands();

        // Implement pure virtuals from PvmiCapabilityAndConfig interface
        OSCL_IMPORT_REF void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver);
        OSCL_IMPORT_REF PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                PvmiKvp*& aParameters, int& num_parameter_elements,
                PvmiCapabilityContext aContext);
        OSCL_IMPORT_REF PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        OSCL_IMPORT_REF void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        OSCL_IMPORT_REF void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                PvmiKvp* aParameters, int num_parameter_elements);
        OSCL_IMPORT_REF void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        OSCL_IMPORT_REF void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                               int num_elements, PvmiKvp * & aRet_kvp);
        OSCL_IMPORT_REF PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                int num_elements, PvmiKvp*& aRet_kvp, OsclAny* context = NULL);
        OSCL_IMPORT_REF uint32 getCapabilityMetric(PvmiMIOSession aSession);
        OSCL_IMPORT_REF PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);

        OSCL_IMPORT_REF bool isActiveCommsRead()
        {
            return iActiveCommsRead;
        }
        OSCL_IMPORT_REF bool isActiveCommsWrite()
        {
            return iActiveCommsWrite;
        }

        OSCL_EXPORT_REF bool isIncomingFull();

    private:
        int32 WriteAsync(int32& cmdId,
                         OsclRefCounterMemFrag frag,
                         PvmiMediaXferHeader data_hdr);

        void Run();

        void QueueData(PVMFSharedMediaDataPtr aData, bool requeue = false, uint32 fragindex = 0);
        PVMFStatus DequeueData(PVMFSharedMediaDataPtr &aData, bool &aResend, uint32 &aFragment);

        void SetDataTransferModels();

        // Container node
        PVCommsIONode* iNode;

        // Rx/Tx bitstream loggers;
        PVLogger *iRxLogger;
        PVLogger *iTxLogger;

        class AsyncIOCmd
        {
            public:
                AsyncIOCmd(PVMFCommandId id, OsclAny* context = NULL): iID(id), iContext(context) {}
                PVMFCommandId iID;
                OsclAny *iContext;
        };
        Oscl_Vector<AsyncIOCmd, OsclMemAllocator> iReadAsyncCmds;

        //Format
        PVMFFormatType iFormatType;

        PvmiMediaTransfer* iMediaOutputTransfer;
        PvmiMediaTransfer* iMediaInputTransfer;
        uint32 iRemoteStatus;

        uint32 cmdId;

        //media data re-send queue
        bool iResend;
        uint32 iResendSeqNum;
        uint32 iResendFragment;
        uint32 iWriteAsyncContext;

        PvmiMIOCommDataBufferAlloc iMediaDataAlloc;
        OsclMemPoolFixedChunkAllocator iMediaDataMemPool;

        enum PortState
        {
            PORT_STATE_BUFFERING = 0,
            PORT_STATE_STARTED,
        };
        PortState iState;
        PVMFFormatType iFormat;

        enum WriteState {EWriteBusy, EWriteWait, EWriteOK};
        WriteState iWriteState;

        //media data cleanup queue
        class CleanupQueueElement
        {
            public:
                CleanupQueueElement(PVMFSharedMediaDataPtr d, PVMFCommandId id): iData(d), iCmdId(id) {}
                PVMFSharedMediaDataPtr iData;
                PVMFCommandId iCmdId;
        };
        Oscl_Vector<CleanupQueueElement, OsclMemAllocator> iCleanupQueue;

        // Receive and Send models for the MediaDataTransfer
        // interface.  Threading model will likely dictate
        // how these are set.
        bool iActiveCommsWrite;
        bool iActiveCommsRead;

        uint32 iQueueLimit;

        bool iEndOfInputPortDataReached;
        bool iEndOfOutputPortDataReached;

        //for flow control
        bool iWriteFailed;
        PvmiMediaTransfer* iPeer;

        void ClearCleanupQueue();
        void CleanupMediaTransfer();

        bool CanSendCommsData();
        void SendCommsData();

        bool EndOfData(int32 aTag);

        //for datapath logging
        void LogMediaDataInfo(const char*msg, PVMFSharedMediaDataPtr mediaData, int32 p1, int32 p2);
        void LogMediaDataInfo(const char*msg, PVMFSharedMediaDataPtr mediaData);
        void LogDatapath(const char*msg);

#ifdef USE_COPY_BUFFER
        uint8 iCopyBuffer[NUM_COPY_BUFFERS][COPY_BUFFER_SIZE];
        uint32 iCopyBufferSize[NUM_COPY_BUFFERS];
        uint32 iCopyBufferIndex;
        bool iCopyBufferSent;

        void CopyBuffer();
        void SendCopyBuffer();
#endif
};

#endif // PV_COMMS_IO_NODE_PORT_H_INCLUDED
