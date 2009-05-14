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
#ifndef PVMF_BUFFER_DATA_SOURCE_H_INCLUDED
#define PVMF_BUFFER_DATA_SOURCE_H_INCLUDED

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif

#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
#endif

class PVMFBufferDataSource : public PvmfPortBaseImpl,
            public OsclTimerObserver,
            public PVMFPortActivityHandler,
            public PvmiCapabilityAndConfig
{
    public:
        OSCL_IMPORT_REF PVMFBufferDataSource(int32 aPortTag,
                                             unsigned bitrate,
                                             unsigned min_sample_sz,
                                             unsigned max_sample_sz,
                                             uint8*   fsi = NULL,
                                             unsigned fsi_len = 0);
        OSCL_IMPORT_REF virtual ~PVMFBufferDataSource();

        void HandlePortActivity(const PVMFPortActivity &);

        // Start/stop the source
        OSCL_IMPORT_REF void Start();
        OSCL_IMPORT_REF void Stop();

        // PVMFPortInterface virtuals
        PVMFStatus PutData(PVMFSharedMediaMsgPtr aMsg);
        PVMFStatus GetData(PVMFSharedMediaMsgPtr aMsg);

        // timer observer
        virtual void TimeoutOccurred(int32 timerID, int32 timeoutInfo);

        OSCL_IMPORT_REF void QueryInterface(const PVUuid& aUuid, OsclAny*& aPtr);

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
    protected:
        unsigned iBitrate;
        unsigned iMinSampleSz;
        unsigned iMaxSampleSz;
        OsclTimer<OsclMemAllocator> iTimer;
        unsigned iSampleInterval;
        PVMFTimestamp iTimestamp;
        PVMFSimpleMediaBufferCombinedAlloc* iMediaDataAlloc;
        OsclMemAllocator iMemAlloc;
        uint8* iFsi;
        unsigned iFsiLen;
};
#endif
