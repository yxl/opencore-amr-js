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
 * @file pvmf_fileoutput_inport.h
 * @brief Input port for simple file output node
 *
 */

#ifndef PVMF_FILEOUTPUT_INPORT_H_INCLUDED
#define PVMF_FILEOUTPUT_INPORT_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVMF_PORT_INTERFACE_H_INCLUDED
#include "pvmf_port_interface.h"
#endif
#ifndef PVMF_NODES_SYNC_CONTROL_H_INCLUDED
#include "pvmf_nodes_sync_control.h"
#endif
#ifndef PVMF_SYNC_UTIL_DATA_QUEUE_H_INCLUDED
#include "pvmf_sync_util_data_queue.h"
#endif
#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
#endif

class PVMFFileOutputNode;

//Default vector reserve size
#define PVMF_FILE_OUTPUT_NODE_PORT_VECTOR_RESERVE 10

// Capability mime strings
#define PVMF_FILE_OUTPUT_PORT_INPUT_FORMATS "x-pvmf/file/encode/input_formats"
#define PVMF_FILE_OUTPUT_PORT_INPUT_FORMATS_VALTYPE "x-pvmf/port/formattype;valtype=char*"

class PVMFFileOutputInPort : public PvmfPortBaseImpl
            , public PvmiCapabilityAndConfig
            , public PvmfSyncUtilDataQueueObserver
            , public OsclTimerObject
            , public PvmfNodesSyncControlInterface
{
    public:
        PVMFFileOutputInPort(int32 aTag, PVMFNodeInterface* aNode);
        ~PVMFFileOutputInPort();

        void Start();
        void Pause();
        void Stop();

        // Pure virtual from PVInterface
        void addRef();
        void removeRef();
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        // Pure virtuals from PvmfNodesSyncControlInterface
        PVMFStatus SetClock(PVMFMediaClock* aClock);
        PVMFStatus ChangeClockRate(int32 aRate);
        PVMFStatus SetMargins(int32 aEarlyMargin, int32 aLateMargin);
        void ClockStarted(void) {} // Not needed
        void ClockStopped(void) {} // Not needed
        PVMFCommandId SkipMediaData(PVMFSessionId aSessionId,
                                    PVMFTimestamp aResumeTimestamp,
                                    uint32 aStreamID,
                                    bool aPlayBackPositionContinuous = false,
                                    OsclAny* aContext = NULL);

        // Pure virtuals from PvmfSyncUtilDataQueueObserver
        void ScheduleProcessData(PvmfSyncUtilDataQueue* aDataQueue, uint32 aTimeMilliseconds);
        void SkipMediaDataComplete();
        void CancelSkipMediaData();

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

        // Pure virtuals from PVMFPortInterface
        void QueryInterface(const PVUuid &aUuid, OsclAny*&aPtr);

        bool IsFormatSupported(PVMFFormatType);
        void FormatUpdated();

        PvmfSyncUtilDataQueue iDataQueue;

        //overload of the routine in PVMFPortBaseImpl
        PVMFStatus ClearMsgQueues()
        {//when the port queue is cleared, also clear the sync queue.
            PvmfPortBaseImpl::ClearMsgQueues();
            iDataQueue.Clear();
            return PVMFSuccess;
        }

    private:

        void ConstructL(PVMFNodeInterface* aContainerNode);
        void Run();

        /**
         * Allocate a specified number of key-value pairs and set the keys
         *
         * @param aKvp Output parameter to hold the allocated key-value pairs
         * @param aKey Key for the allocated key-value pairs
         * @param aNumParams Number of key-value pairs to be allocated
         * @return Completion status
         */
        PVMFStatus AllocateKvp(PvmiKvp*& aKvp, PvmiKeyType aKey, int32 aNumParams);

        /**
         * Verify one key-value pair parameter against capability of the port and
         * if the aSetParam flag is set, set the value of the parameter corresponding to
         * the key.
         *
         * @param aKvp Key-value pair parameter to be verified
         * @param aSetParam If true, set the value of parameter corresponding to the key.
         * @return PVMFSuccess if parameter is supported, else PVMFFailure
         */
        PVMFStatus VerifyAndSetParameter(PvmiKvp* aKvp, bool aSetParam = false);

        PVMFFileOutputNode* iNode;
        PvmfSyncUtil iSyncUtil;

        bool iSkipMediaDataPending;
        PVMFSessionId iSkipMediaDataSessionId;
        PVMFCommandId iSkipMediaDataCmdId;
        OsclAny* iSkipMediaDataContext;
        uint32 iExtensionRefCount;

        enum PortState
        {
            PORT_STATE_BUFFERING = 0,
            PORT_STATE_STARTED,
        };
        PortState iState;

        PVMFFormatType iFormat;
        OsclMemAllocator iAlloc;

        bool iSkipAlreadyComplete;
        PVMFTimestamp iSkipResumeTimestamp;
        bool iLastDataTimestampSet;
        PVMFTimestamp iLastDataTimestamp;

        friend class PVMFFileOutputNode;
};

#endif // PVMF_FILEOUTPUT_INPORT_H_INCLUDED
