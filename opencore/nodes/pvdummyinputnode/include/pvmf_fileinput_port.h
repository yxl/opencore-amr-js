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
 * @file pvmf_fileinput_port.h
 */

#ifndef PVMF_FILEINPUT_PORT_H_INCLUDED
#define PVMF_FILEINPUT_PORT_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif

//Default vector reserve size
#define PVMF_FILEINPUT_NODE_PORT_VECTOR_RESERVE 10

// Capability mime strings
#define PVMF_FILEINPUT_PORT_INPUT_FORMATS OUTPUT_FORMATS_CUR_QUERY
#define PVMF_FILEINPUT_PORT_INPUT_FORMATS_VALTYPE OUTPUT_FORMATS_VALTYPE
#define INPORT_CAPACITY 0
#define INPORT_RESERVE 0
#define INPORT_THRESOLD 0
#define OUTPORT_CAPACITY 10
#define OUTPORT_RESERVE 0
#define OUTPORT_THRESOLD 70
/**
 * An example of a PVMF port implementation.
 *
 * Input (sink) ports have a simple flow control scheme.
 * Ports report "busy" when their queue is full, then when the
 * queue goes to half-empty they issue a "get data" to the connected
 * port.  The media message in the "get data" is empty and is
 * meant to be discarded.
 * Output (source) ports assume the connected port uses the
 * same flow-control scheme.
 */
class PVMFFileDummyInputPort : public PvmfPortBaseImpl,
            public PvmiCapabilityAndConfig
{
    public:
        /**
         * Default constructor. Default settings will be used for the data queues.
         * @param aId ID assigned to this port
         * @param aTag Port tag
         * @param aNode Container node
         */
        PVMFFileDummyInputPort(int32 aTag
                               , PVMFNodeInterface* aNode);

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
        PVMFFileDummyInputPort(int32 aTag
                               , PVMFNodeInterface* aNode
                               , uint32 aInCapacity
                               , uint32 aInReserve
                               , uint32 aInThreshold
                               , uint32 aOutCapacity
                               , uint32 aOutReserve
                               , uint32 aOutThreshold);

        /** Destructor */
        ~PVMFFileDummyInputPort();

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

        // Implement virtuals from PVMFPortBaseImpl
        OSCL_IMPORT_REF PVMFStatus Connect(PVMFPortInterface* aPort);
        void QueryInterface(const PVUuid &aUuid, OsclAny*&aPtr)
        {
            if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
                aPtr = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, this);
            else
                aPtr = NULL;
        }
        bool IsFormatSupported(PVMFFormatType aFmt);
        void FormatUpdated();

    private:
        void Construct();
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

        bool pvmiGetPortFormatSpecificInfoSync(const char* aFormatValType, PvmiKvp*& aKvp);
        bool pvmiSetPortFormatSpecificInfoSync(PvmiCapabilityAndConfig *aPort, const char* aFormatValType);

        uint32 iNumFramesGenerated; //number of source frames generated.
        PVMFFormatType iFormat;
        OsclMemAllocator iAlloc;
        PVLogger *iLogger;
        uint32 iTrackConfigSizeFI;
        uint8* iTrackConfigFI;
        friend class PVMFDummyFileInputNode;
};

#endif // PVMF_FILEINPUT_PORT_H_INCLUDED



