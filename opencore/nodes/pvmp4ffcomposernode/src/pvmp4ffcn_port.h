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
 * @file pvmp4ffcn_port.h
 * @brief Port for PVMp4FFComposerNode
 */

#ifndef PVMP4FFCN_PORT_H_INCLUDED
#define PVMP4FFCN_PORT_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#ifndef TEXTSAMPLEDESCINFO_H
#include "textsampledescinfo.h"
#endif

class PVMp4FFComposerNode;
class PVLogger;

typedef enum
{
    PVMP4FFCN_RATE_CONTROL_UNKNOWN = 0,
    PVMP4FFCN_RATE_CONTROL_CBR = 1,
    PVMP4FFCN_RATE_CONTROL_VBR = 2,
    PVMP4FFCN_RATE_CONTROL_LOW_DELAY = 3
} PVMP4FFCNRateControlType;

struct PVMP4FFCNFormatSpecificConfig
{
    uint32 iBitrate;
    uint32 iTimescale;

    // Video configuration
    OsclFloat iFrameRate;
    uint32 iIFrameInterval;
    uint32 iWidth;
    uint32 iHeight;
    PVMP4FFCNRateControlType iRateControlType;

    // H263 configuration
    uint8 iH263Profile;
    uint8 iH263Level;
};

class PVMp4FFComposerPort : public PvmfPortBaseImpl,
            public PvmiCapabilityAndConfig,
            public PVMFPortActivityHandler,
            public OsclActiveObject
{
    public:
        PVMp4FFComposerPort(int32 aTag, PVMp4FFComposerNode* aNode, int32 aPriority, const char* aName = NULL);
        ~PVMp4FFComposerPort();

        // Overload PvmfPortBaseImpl methods
        OSCL_IMPORT_REF PVMFStatus Connect(PVMFPortInterface* aPort);
        OSCL_IMPORT_REF PVMFStatus PeerConnect(PVMFPortInterface* aPort);
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

        // For node to notify the port that it's ready to process incoming message
        void ProcessIncomingMsgReady();

        // From PVMFPortActivityHandler
        void HandlePortActivity(const PVMFPortActivity& aActivity);

        // Track ID
        void SetTrackId(int32 aTrackId)
        {
            iTrackId = aTrackId;
        }
        int32 GetTrackId()
        {
            return iTrackId;
        }

        // Provide port configuration to node
        void SetFormat(PVMFFormatType aFormat)
        {
            iFormat = aFormat;
            iMimeType = aFormat.getMIMEStrPtr();
        }
        PVMFFormatType GetFormat()
        {
            return iFormat;
        }
        OSCL_String& GetMimeType()
        {
            return iMimeType;
        }
        void SetCodecType(int32 aCodecType)
        {
            iCodecType = aCodecType;
        }
        int32 GetCodecType()
        {
            return iCodecType;
        }
        PVMP4FFCNFormatSpecificConfig* GetFormatSpecificConfig()
        {
            PvmiCapabilityAndConfig* config = NULL;
            if (iConnectedPort)
            {
                OsclAny* temp = NULL;
                iConnectedPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
                config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);
                if (config)
                {
                    GetInputParametersFromPeer(config);
                }
            }
            return &iFormatSpecificConfig;
        }


        // Reference track settings
        void SetReferencePort(PVMp4FFComposerPort* aPort)
        {
            iReferencePort = aPort;
        }
        const PVMp4FFComposerPort* GetReferencePort()
        {
            return iReferencePort;
        }

        // Real-time TS routines
        uint32 GetLastTS()
        {
            return iLastTS;
        }
        void SetLastTS(uint32 aTS)
        {
            iLastTS = aTS;
        }

        friend class PVMp4FFComposerNode;
    private:
        // Implement pure virtuals from OsclActiveObject
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

        /**
         * Negotiates input port settings (format, video size and frame rate) and
         * configures the peer port and the container node with these settings
         *
         * @param aConfig Capability and config object for peer port
         * @return PVMFSuccess if settings are successfully negotiated.
         */
        PVMFStatus NegotiateInputSettings(PvmiCapabilityAndConfig* aConfig);

        /**
         * Query peer port for properties of incoming data.
         * @return Completion status
         */
        PVMFStatus GetInputParametersFromPeer(PvmiCapabilityAndConfig* aConfig);

        /**
         * Query peer port for properties of incoming data.
         * @return Completion status
         */
        PVMFStatus GetVideoInputParametersFromPeer(PvmiCapabilityAndConfig* aConfig);

    private:

        // Container node
        PVMp4FFComposerNode* iNode;

        // Port configuration
        int32 iTrackId;
        PVMFFormatType iFormat;
        int32 iCodecType; //integer value to avoid formatype comparisons
        PVMP4FFCNFormatSpecificConfig iFormatSpecificConfig;
        PVMp4FFComposerPort* iReferencePort;
        OsclMemoryFragment* memfrag_sps;
        OsclMemoryFragment* memfrag_pps;
        // Real-time TS
        uint32 iLastTS;

        OsclMemAllocator iAlloc;
        PVLogger* iLogger;
        bool iEndOfDataReached;

        //logging
        OSCL_HeapString<OsclMemAllocator> iMimeType;
};

#endif // PVMP4FFCN_PORT_H_INCLUDED
