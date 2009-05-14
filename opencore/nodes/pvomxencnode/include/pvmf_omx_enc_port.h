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
#ifndef PVMF_OMX_ENC_PORT_H_INCLUDED
#define PVMF_OMX_ENC_PORT_H_INCLUDED


#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PVMF_PORT_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

#ifndef PVMF_PORT_BASE_IMPL_H_INCLUDED
#include "pvmf_port_base_impl.h"
#endif

#ifndef PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
#endif

#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif

#ifndef PV_MIME_STRING_UTILS_H_INCLUDED
#include "pv_mime_string_utils.h"
#endif

class PVMFOMXEncNode;
class PVMFOMXEncInputFormatCompareLess;
//Default vector reserve size
#define PVMF_OMX_ENC_NODE_PORT_VECTOR_RESERVE 10



typedef enum
{
    PVMF_OMX_ENC_NODE_PORT_TYPE_INPUT,
    PVMF_OMX_ENC_NODE_PORT_TYPE_OUTPUT,
} PVMFOMXEncPortType;

class PVMFOMXEncPort : public PvmfPortBaseImpl
            , public PvmiCapabilityAndConfigPortFormatImpl
{
    public:
        PVMFOMXEncPort(int32 aTag, PVMFNodeInterface* aNode, const char*);
        ~PVMFOMXEncPort();

        // Implement pure virtuals from PvmiCapabilityAndConfigPortFormatImpl interface
        bool IsFormatSupported(PVMFFormatType);
        void FormatUpdated();

        // this port supports config interface
        void QueryInterface(const PVUuid &aUuid, OsclAny*&aPtr)
        {
            if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
                aPtr = (PvmiCapabilityAndConfig*)this;
            else
                aPtr = NULL;
        }

        bool pvmiSetPortFormatSpecificInfoSync(OsclRefCounterMemFrag& aMemFrag);

        PVMFStatus Connect(PVMFPortInterface* aPort);
        void setParametersSync(PvmiMIOSession aSession,
                               PvmiKvp* aParameters,
                               int num_elements,
                               PvmiKvp * & aRet_kvp);

        PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        PVMFStatus verifyConnectedPortParametersSync(const char* aFormatValType, OsclAny* aConfig);
        uint32 getTrackConfigSize()
        {
            return iTrackConfigSize;
        }
        uint8* getTrackConfig()
        {
            return iTrackConfig;
        }

        OSCL_IMPORT_REF PVMFStatus getParametersSync(PvmiMIOSession session,
                PvmiKeyType identifier,
                PvmiKvp*& parameters,
                int& num_parameter_elements,
                PvmiCapabilityContext context);

        OSCL_IMPORT_REF PVMFStatus releaseParameters(PvmiMIOSession session,
                PvmiKvp* parameters,
                int num_elements);

        // For AVC specific call
        void SendSPS_PPS(OsclMemoryFragment *aSPSs, int aNumSPSs, OsclMemoryFragment *aPPSs, int aNumPPSs);

    private:
        void Construct();

/////////////////////////////////////////////////////////////////////////////////////
        ///encoder specific
        /**
        * Synchronous query of input port parameters
        */
        PVMFStatus GetInputParametersSync(PvmiKeyType identifier, PvmiKvp*& parameters,
                                          int& num_parameter_elements);

        /**
         * Synchronous query of output port parameters
         */
        PVMFStatus GetOutputParametersSync(PvmiKeyType identifier, PvmiKvp*& parameters,
                                           int& num_parameter_elements);
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
         * Negotiates output port settings and configures the peer port using settings
         * from the container node.
         *
         * @param aConfig Capability and config object for peer port
         * @return PVMFSuccess if settings are successfully negotiated.
         */
        PVMFStatus NegotiateOutputSettings(PvmiCapabilityAndConfig* aConfig);
        int32 Config_ParametersSync(PvmiCapabilityAndConfig*&, PvmiKvp*&, PvmiKvp*&);
        int32 PushKVP(OsclPriorityQueue<PvmiKvp*, OsclMemAllocator, Oscl_Vector<PvmiKvp*, OsclMemAllocator>, PVMFOMXEncInputFormatCompareLess>&, PvmiKvp*);

        OsclMemAllocator iAlloc;
//////////////////////////////////////////////////////////////////////////

        PVLogger *iLogger;
        uint32 iNumFramesGenerated; //number of source frames generated.
        uint32 iNumFramesConsumed; //number of frames consumed & discarded.
        uint32 iTrackConfigSize;
        uint8* iTrackConfig;
        uint32 iTimescale;
        friend class PVMFOMXEncNode;
        PVMFOMXEncNode* iOMXNode;
};

class PVMFOMXEncInputFormatCompareLess
{
    public:
        /**
         * The algorithm used in OsclPriorityQueue needs a compare function
         * that returns true when A's priority is less than B's
         * @return true if A's priority is less than B's, else false
         */
        int compare(PvmiKvp*& a, PvmiKvp*& b) const
        {
            return (PVMFOMXEncInputFormatCompareLess::GetPriority(a) <
                    PVMFOMXEncInputFormatCompareLess::GetPriority(b));
        }

        /**
         * Returns the priority of each command
         * @return A 0-based priority number. A lower number indicates lower priority.
         */
        static int GetPriority(PvmiKvp*& aKvp)
        {
            if (pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_YUV420) == 0)
            {
                return 3;
            }
            else if (pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_YUV422) == 0)
            {
                return 2;
            }
            else if (pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_RGB12) == 0)
            {
                return 1;
            }
            else if (pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_RGB24) == 0)
            {
                return 0;
            }
            else
            {
                return 0;
            }
        }
};
#endif // PVMF_OMX_ENC_PORT_H_INCLUDED
