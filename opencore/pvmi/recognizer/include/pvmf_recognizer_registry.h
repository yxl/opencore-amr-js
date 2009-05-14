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
/*
* ==============================================================================
*  Name        : pvmf_recognizer_registry.h
*  Part of     :
*  Interface   :
*  Description : Interface class with static methods for PVMF recognizer registry
*  Version     : (see RELEASE field in copyright header above)
*
* ==============================================================================
*/

#ifndef PVMF_RECOGNIZER_REGISTRY_H_INCLUDED
#define PVMF_RECOGNIZER_REGISTRY_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif

#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef PVMF_RECOGNIZER_TYPES_H_INCLUDED
#include "pvmf_recognizer_types.h"
#endif

#ifndef PVMF_RECOGNIZER_PLUGIN_H_INCLUDED
#include "pvmf_recognizer_plugin.h"
#endif

#ifndef PVMI_DATASTREAMUSER_INTERFACE_H_INCLUDED
#include "pvmi_datastreamuser_interface.h"
#endif


/**
 * A class of static methods that provides the interface to the PVMF recognizer registry..
 * With these static methods, the recognizer registry can be initialized and shutdown, recognizer
 * plug-ins can be registered and unregistered, and format of a content can be recognized.
 **/
class PVMFRecognizerRegistry
{
    public:
        /**
         * This static method initializes the recognizer registry for use. This method must be called once
         * and succeed before calling any other methods from PVMFRecognizerRegistry.
         *
         * @exception This method can leave with one of the following error codes
         *         OsclErrNoMemory if memory cannot be allocated for the registry implementation
         * @returns A PVMF status code to report result of method
         **/
        OSCL_IMPORT_REF static PVMFStatus Init();

        /**
         * This static methods shuts down and cleans up the recognizer registry. This method must be called once
         * after there is no more use for the recognizer registry to properly release the memory allocated for the
         * registry
         **/
        OSCL_IMPORT_REF static void Cleanup();

        /**
         * This static method adds the specified recognizer plug-in factory to the list of available recognizers.
         * The passed-in plug-in factory would be used to create and destroy the recognizer plug-in in the Recognize() methods.
         * The passed-in plug-in factory reference must be valid until it is removed from the list by the RemovePlugin() method.
         *
         * @param aPluginFactory
         *        A reference to a recognizer plug-in factory to add to the list of registered recognizers
         *
         * @exception This method can leave with one of the following error codes
         *         OsclErrNoMemory if memory cannot be allocated for vector holding the plug-in factory pointers
         * @returns A PVMF status code to report result of method
         **/
        OSCL_IMPORT_REF static PVMFStatus RegisterPlugin(PVMFRecognizerPluginFactory& aPluginFactory);

        /**
         * This static method removes the specified recognizer plug-in factory from the list of available recognizers.
         * The passed-in plug-in factory must have been added before with RegisterPlugin() for this method to succeed.
         * After the plug-in factory is successfully removed, the factory instance can be deleted.
         *
         * @param aPluginFactory
         *        A reference to a recognizer plug-in factory to remove from the list of registered recognizers
         *
         * @returns A PVMF status code to report result of method
         **/
        OSCL_IMPORT_REF static PVMFStatus RemovePlugin(PVMFRecognizerPluginFactory& aPluginFactory);

        /**
         * This static method creates a recognition session with the recognizer framework.
         *
         * @param aSessionId
         *        A reference to a PVMFSessionId which will be set to the session's unique identifier when this method
         *        completes successfully.
         * @param aCmdHandler
         *        A reference to a PVMFRecognizerCommmandHandler which will receive asynchronous command completion notification.
         *
         * @returns A PVMF status code to report result of method
         **/
        OSCL_IMPORT_REF static PVMFStatus OpenSession(PVMFSessionId& aSessionId, PVMFRecognizerCommmandHandler& aCmdHandler);

        /**
         * This static method shuts down a recognition session with the recognizer framework.
         *
         * @param aSessionId
         *        The unique identifier of the session to close
         *
         * @returns A PVMF status code to report result of method
         **/
        OSCL_IMPORT_REF static PVMFStatus CloseSession(PVMFSessionId aSessionId);

        /**
         * This static method asynchronously determines the format of the specified content using the currently registered
         * recognizer plug-ins. When the recognizer request completes, the user would be notified via the PVMFRecognizerCommandHandler
         * callback handler set when opening the session.
         *
         * @param aSessionId
         *        The unique identifier for the recognizer session
         * @param aSourceDataStreamFactory
         *        A reference to a PVMFDataStreamFactory representing the content to recognize
         * @param aFormatHintList
         *        An optional input parameter expressed as a list of MIME string which provides a priori hint for the format
         *        of the content specified by aSourceDataStreamFactory.
         * @param aRecognizerResult
         *        An output parameter which is a reference to a vector of PVMFRecognizerResult that will contain the recognition
         *        result if the Recognize() method succeeds.
         * @param aCmdContext
         *        Optional pointer to opaque data that will be returned in the command completion response
         * @param aTimeout
         *        Optional timeout value for the recognition request. If the recognition operation takes more time than the timeout
         *        value, the operation will be cancelled with PVMFErrTimeout status code. If the timeout value is set to 0, the
         *        recognition operation will go to completion.
         *
         * @exception This method can leave with one of the following error codes
         *         OsclErrNoMemory if memory cannot allocated to process this request
         *         OsclErrArgument if one or more of the passed-in parameters is invalid
         *
         * @returns A PVMF command ID for the recognize request
         **/
        OSCL_IMPORT_REF static PVMFCommandId Recognize(PVMFSessionId aSessionId, PVMFDataStreamFactory& aSourceDataStreamFactory, PVMFRecognizerMIMEStringList* aFormatHintList,
                Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator>& aRecognizerResult, OsclAny* aCmdContext = NULL, uint32 aTimeout = 0);

        /**
         * This static method cancels a previously issued asynchronous request that hasn't completed yet.
         *
         * @param aSessionId
         *        The unique identifier for the recognizer session
         * @param aCommandToCancelId
         *        Unique identifier for the asynchronous request to cancel.
         * @param aCmdContext
         *        Optional pointer to opaque data that will be returned in the command completion response
         *
         * @exception This method can leave with one of the following error codes
         *         OsclErrNoMemory if memory cannot allocated to process this request
         *         OsclErrArgument if one or more of the passed-in parameters is invalid
         *
         * @returns A PVMF command ID for the cancel request
         **/
        OSCL_IMPORT_REF static PVMFCommandId CancelCommand(PVMFSessionId aSessionId, PVMFCommandId aCommandToCancelId, OsclAny* aCmdContext = NULL);
};

#endif // PVMF_RECOGNIZER_REGISTRY_H_INCLUDED




