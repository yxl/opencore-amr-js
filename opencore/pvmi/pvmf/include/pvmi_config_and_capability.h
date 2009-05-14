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


/*! \file pvmi_config_and_capability.h
\brief This file contains the abstractions for configuration and capability exchange
*/

#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#define PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_OBSERVER_H_INCLUDED
#include "pvmi_config_and_capability_observer.h"
#endif
#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif

#define PVMI_CAPABILITY_AND_CONFIG_PVUUID PVUuid(0xa07419ac,0x20c5,0x4c2e,0x85,0x56,0xc9,0x2f,0xc6,0x74,0xf3,0xa7)
typedef void* PvmiCapabilityContext;

/**
 * Abstract interface for capability and configuration of PVMI components
 */
class PvmiCapabilityAndConfig: public PVInterface
{
    public:
        virtual ~PvmiCapabilityAndConfig() {}

        /**
         * This method allows setting of an observer to listen for completion of asynchronous commands.
         *
         * @param aObserver     The observer for status
         */
        virtual void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver) = 0;

        /**
         * This method returns a list of parameters describing the attribute specified in the query.
         * The attributes that can be requested are capabilities, current parameter setting and default
         * setting.
         *
         * @param aSession                  An Opaque session ID
         * @param aIdentifier               MIME string that has the specific query
         * @param aParameters               An output pointer to an array of parameters relevant to the query
         * @param num_parameter_elements    Number of the elements in the returned array of parameters
         * @param aContext                  The context in which we expect to restrict our search
         * @returns                         PVMFSuccess if the operation is succesful or PVMFErrArgument
         *                                  if the format of the key(s) is invalid
         */
        virtual PVMFStatus getParametersSync(PvmiMIOSession aSession,
                                             PvmiKeyType aIdentifier,
                                             PvmiKvp*& aParameters,
                                             int& num_parameter_elements,
                                             PvmiCapabilityContext   aContext) = 0;


        /**
         * This method releases a list of parameters back to the component that allocated it.
         * The caller should remove any references to the parameters and structures contained within after a
         * call to this function.
         *
         * @param aSession                  The Opaque session ID
         * @param aParameters               A pointer to an array of parameters relevant to the query
         * @param num_elements              Number of the elements in the returned array of parameters
         * @returns                         PVMFSuccess if the operation is succesful or PVMFFailure
         *                                  if an error has occured during the operation
         */
        virtual PVMFStatus releaseParameters(PvmiMIOSession aSession,
                                             PvmiKvp* aParameters,
                                             int num_elements) = 0;

        /**
         * This method is used to create a context within a capability exchange session.
         *
         * @param aSession                  The Opaque session ID
         * @param aContext                  Will contain the handle to the newly created context
         * @returns                         This function can leave with the following error codes
         *                                  PVMFErrNotSupported, if the component does not suppport contexts
         *                                  PVMFErrExceeded,  if the number of allowable contexts is exceeded
         */
        virtual void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext) = 0;

        /**
         * This method defines the space of a context.When a context is created, its space is the gamut
         * of capabilities of the component.This API defines the new space of the context as the
         * intersection of the current space with the space defined by the parameters.
         *
         *
         * @param aSession                  The Opaque session ID
         * @param aContext                  The context that we expect to configure
         * @param aParameters               An array of key-value pairs that should be associated with this context
         * @param num_parameter_elements    Number of the elements in the array of parameters.
         * @returns                         The function can leave with the following error codes
         *                                  PVMFErrArgument if invalid arguments are passed
         *                                  PVMFFailure if there is not intersection between the parameter space and the space of the context.
         */
        virtual void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                                          PvmiKvp* aParameters, int num_parameter_elements) = 0;

        /**
         * A context can deleted using the deleteContext API.
         * A user of the context should remove all references to the context once it has been deleted.
         *
         * @param aSession                  The Opaque session ID
         * @param aContext                  Will contain a handle to the context
         * @returns                         This function can leave with the following error codes
         */
        virtual void DeleteContext(PvmiMIOSession aSession,
                                   PvmiCapabilityContext& aContext) = 0;

        /**
         * This method is to be used to set one or more parameter values, legal values for which
         * have been previously queried using the getParametersSync API.
         *
         * @param aSession                  The Opaque session ID
         * @param aParameters               An array of key-value pairs that should be used to set the parametres
         * @param num_elements              Number of the elements in the array of parameters.
         * @param aRet_kvp                  ret_kvp is an output parameter to be used in case of errors.
         *                                  It points to the first input key-value pair that caused an error
         * @returns                         The function can leave with the following error codes
         *                                  PVMFErrArgument if invalid arguments are passed.
         *                                  In this case, Ret_kvp output argument is set to point to
         *                                  the argument that caused the error.
         */
        virtual void setParametersSync(PvmiMIOSession aSession,
                                       PvmiKvp* aParameters,
                                       int num_elements,
                                       PvmiKvp * & aRet_kvp) = 0;

        /**
         * This is an asynchronous version the setParameters method.
         * The memory for the parameters shall be valid and not modified until
         * the completion of processing of this command
         *
         * @param aSession                  The Opaque session ID
         * @param aParameters               An array of key-value pairs that should be used to set the parametres
         * @param num_elements              Number of the elements in the array of parameters.
         * @param aRet_kvp                  ret_kvp is an output parameter to be used in case of errors.
         *                                  It points to the first input key-value pair that caused an error
         * @returns                         TBD. PVMFSuccess if the operation was succesful, PVMFFailure otherwise
         */
        virtual PVMFCommandId setParametersAsync(PvmiMIOSession aSession,
                PvmiKvp* aParameters,
                int num_elements,
                PvmiKvp*& aRet_kvp,
                OsclAny* context = NULL) = 0;

        /**
         * This method is used to query the complexity of a components capability tree.
         * The modulus shall be used to determine the master in a peer-peer capability exchange.
         *
         * @param aSession                  The Opaque session ID
         * @returns                         TBD.
         */
        virtual uint32 getCapabilityMetric(PvmiMIOSession aSession) = 0;

        /**
         * This method verifies if a mode of operation as identified by a set of key-value pairs
         * is within the capabilities of the component, based on its current state of operation.
         * If the call returns success, calls to SetParameter using the same parameters should also
         * be successful, given the same state of the component.
         *
         * @param aSession                  The Opaque session ID
         * @param aParameters               An array of key-value pairs that should be used to verify the parametres
         * @param num_elements              Number of the elements in the array of parameters.
         * @returns                         TBD. PVMFSuccess if the operation was succesful, PVMFFailure otherwise
         */
        virtual PVMFStatus verifyParametersSync(PvmiMIOSession aSession,
                                                PvmiKvp* aParameters, int num_elements) = 0;

        //from PVInterface
        virtual bool queryInterface(const PVUuid& aUuid
                                    , PVInterface*& aInterfacePtr)
        {
            if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
            {
                aInterfacePtr = this;
                return true;
            }
            return false;
        }
        virtual void addRef()
        {}
        virtual void removeRef()
        {}

    protected:
        PvmiConfigAndCapabilityCmdObserver* ciObserver;
};

#endif //PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
