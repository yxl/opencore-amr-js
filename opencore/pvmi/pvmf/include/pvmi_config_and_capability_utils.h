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
// -*- c++ -*-
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//               P V M I _ C A P A B I L I T Y _ A N D _ C O N F I G

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \addtogroup pvmicapability PvmiCapability
*
* @{
*/


/*! \file pvmi_config_and_capability_utils.h
\brief This file contains a simple implementation
of config and capability for setting port data format
only.
*/

#ifndef PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED
#define PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED

#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

//A static routine to set a port format using setParametersSync.
//The call may leave.
class PVMFPortInterface;
OSCL_IMPORT_REF void pvmiSetPortFormatSync(PvmiCapabilityAndConfig *aPort, const char* aFormatValType, PVMFFormatType aFormat);

// A routing to allocate a Kvp
OSCL_IMPORT_REF PVMFStatus AllocateKvp(OsclMemAllocator& aAlloc, PvmiKvp*& aKvp, PvmiKeyType aKey, int32 aNumParams);

/** This class will set port data format using
** the synchronous methods only.  The derived class
** must set the format type string and format val type string
** by calling Construct,
** and must implement the IsFormatSupported and FormatUpdated
** routines.
*/
class PvmiCapabilityAndConfigPortFormatImpl
            : public PvmiCapabilityAndConfig
{
    public:
        PvmiCapabilityAndConfigPortFormatImpl()
                : iFormat(PVMF_MIME_FORMAT_UNKNOWN)
        {}

        virtual ~PvmiCapabilityAndConfigPortFormatImpl() {};

        //Derived class must call this to set the format type strings
        //before the other methods can be used.
        void Construct(const char*aFormatType, const char* aFormatValType)
        {
            iFormatTypeString = aFormatType;
            iFormatValTypeString = aFormatValType;
        }

        //Derived class must implement this to verify the requested
        //format.
        virtual bool IsFormatSupported(PVMFFormatType aFormat) = 0;

        //Derived class must implement this to be notified when
        //the format has been updated through the setParametersSync
        //call.  If this function leaves, the leave will
        //propagate to the caller of setParametersSync
        virtual void FormatUpdated() = 0;

        // Implement pure virtuals from PvmiCapabilityAndConfig interface
        OSCL_IMPORT_REF virtual PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                PvmiKvp*& aParameters, int& num_parameter_elements,	PvmiCapabilityContext aContext);
        OSCL_IMPORT_REF virtual PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        OSCL_IMPORT_REF virtual void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                int num_elements, PvmiKvp * & aRet_kvp);
        OSCL_IMPORT_REF virtual PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);

        // Unsupported PvmiCapabilityAndConfig methods
        void virtual setObserver(PvmiConfigAndCapabilityCmdObserver*) {};
        void virtual createContext(PvmiMIOSession , PvmiCapabilityContext&) {};
        void virtual setContextParameters(PvmiMIOSession , PvmiCapabilityContext& , PvmiKvp* , int) {};
        void virtual DeleteContext(PvmiMIOSession , PvmiCapabilityContext&) {};
        PVMFCommandId virtual setParametersAsync(PvmiMIOSession , PvmiKvp* , int , PvmiKvp*& , OsclAny* context = NULL)
        {
            OSCL_UNUSED_ARG(context);
            return -1;
        }
        uint32 virtual getCapabilityMetric(PvmiMIOSession)
        {
            return 0;
        }


        PVMFFormatType iFormat;

    protected:
        //Format type and FormatValType strings.
        OSCL_HeapString<OsclMemAllocator> iFormatTypeString;
        OSCL_HeapString<OsclMemAllocator> iFormatValTypeString;
};

#endif //PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED


