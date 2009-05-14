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

#ifndef PVMF_MEDIA_INPUT_NODE_OUTPORT_H_INCLUDED
#define PVMF_MEDIA_INPUT_NODE_OUTPORT_H_INCLUDED

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
#ifndef PVMF_MEDIA_INPUT_DATA_BUFFER_H_INCLUDED
#include "pvmf_media_input_data_buffer.h"
#endif

// Forward declaration
class PvmfMediaInputNode;

class PvmfMediaInputNodeOutPort : public OsclTimerObject,
            public OsclMemPoolFixedChunkAllocatorObserver,
            public PvmfPortBaseImpl,
            public PvmiMediaTransfer,
            public PVMFPortActivityHandler,
            public PvmiCapabilityAndConfig
{
    public:
        PvmfMediaInputNodeOutPort(PvmfMediaInputNode* aNode, const char* aName = NULL);
        ~PvmfMediaInputNodeOutPort();

        void Start();
        void MediaIOStarted();
        void Pause();
        void Stop();

        PVMFStatus Configure(PVMFFormatType aPortProperty, OSCL_String* aMime);

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
        void SendEndOfTrackCommand(const PvmiMediaXferHeader& data_header_info);
    private:

        void Run();
        void freechunkavailable(OsclAny*);
        // Container node
        PvmfMediaInputNode* iNode;

        //Format
        PVMFFormatType iFormatType;

        PvmiMediaTransfer* iMediaInput;
        uint32 iRemoteStatus;

        uint32 iCmdId;

        PvmfMediaInputDataBufferAlloc* iMediaDataAlloc;
        OsclMemPoolFixedChunkAllocator* iMediaDataAllocMemPool;
        OsclMemPoolFixedChunkAllocator* iMediaDataMemPool;

        enum WriteState {EWriteBusy, EWriteOK};
        WriteState iWriteState;
        enum PortState
        {
            PORT_STATE_BUFFERING = 0,
            PORT_STATE_STARTED,
            PORT_STATE_ENDOFTRACK
        };
        PortState iState;
        PVMFFormatType iFormat;

        //for flow control

        PvmiMediaTransfer* iPeer;


        Oscl_Vector<int32, OsclMemAllocator> itext_sample_index;
        Oscl_Vector<uint32, OsclMemAllocator> istart_text_sample;
        Oscl_Vector<uint32, OsclMemAllocator> iend_text_sample;

        // Format specific info
        OsclRefCounterMemFrag iFormatSpecificInfo;
        uint32 inum_text_sample;
        uint32 imax_num_sample;
#ifdef _TEST_AE_ERROR_HANDLING
        uint32 iTimeStampJunk;
#endif
        //logging
        OSCL_HeapString<OsclMemAllocator> iMimeType;
        PVLogger* iDataPathLogger;
};

#endif // PVMF_MEDIA_INPUT_NODE_INPORT_H_INCLUDED
