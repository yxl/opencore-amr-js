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
#ifndef CPM_H_INCLUDED
#define CPM_H_INCLUDED

#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif
#ifndef PVMI_KVP_INCLUDED
#include "pvmi_kvp.h"
#endif
#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif
#ifndef CPM_TYPES_H_INCLUDED
#include "cpm_types.h"
#endif
#ifndef PVMF_CPMPLUGIN_ACCESS_INTERFACE_FACTORY_H_INCLUDED
#include "pvmf_cpmplugin_access_interface_factory.h"
#endif
#ifndef PVMF_CPMPLUGIN_ACCESS_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_access_interface.h"
#endif

typedef int32 PVMFCPMUsageID;

/* Forward declarations */
class CPMPluginRegistry;

/**
 * PVMFCPMStatusObserver Class
 *
 * PVMFCPMStatusObserver is the PVMF CPM Plugin observer class for
 * notifying the status of issued command messages. The API provides a mechanism
 * for the status of each command to be passed back along with context specific
 * information where applicable.
 * Applications using the module must have a class derived from
 * PVMFCPMStatusObserver and implement the pure virtual function in
 * order to receive event notifications from the plugin.
 **/
class PVMFCPMStatusObserver
{
    public:
        /**
         * Handle an event that has been generated.
         * @param "aResponse"	"The response to a previously issued command."
         */
        virtual void CPMCommandCompleted(const PVMFCmdResp& aResponse) = 0;
        virtual ~PVMFCPMStatusObserver() {}
};

class PVMFCPM : public PVInterface
{
    public:
        /**
         * Establishes usage thread context.
         * CPM should do all thread-context-sensitive initialization in this
         * call.
         **/
        virtual void ThreadLogon() = 0;
        /**
         * Exits usage thread context.
         * CPM should do all thread-context-sensitive cleanup in this
         * call.
         **/
        virtual void ThreadLogoff() = 0;

        /**
         * Starts initialization of the content policy manager.  At the minimum,
         * CPM should be ready to establish a session after initialization is complete
         **/
        virtual PVMFCommandId Init(const OsclAny* aContext = NULL) = 0;

        /**
         * Creates a session based on interactions with all the
         * registered plugins.
         **/
        virtual PVMFCommandId OpenSession(PVMFSessionId& aSessionId,
                                          const OsclAny* aContext = NULL) = 0;

        virtual PVMFCommandId RegisterContent(PVMFSessionId aSessionId,
                                              OSCL_wString& aSourceURL,
                                              PVMFFormatType& aSourceFormat,
                                              OsclAny* aSourceData,
                                              const OsclAny* aContext = NULL) = 0;

        /**
         * Retrieves the CPM content type for a passed in source. This could
         * be types based on DRM version or content rating etc
         **/
        virtual PVMFCPMContentType GetCPMContentType(PVMFSessionId aSessionId) = 0;


        /**
         * Optional API to retrieve the original filename of a protected file.
         **/
        virtual PVMFStatus GetCPMContentFilename(PVMFSessionId aSessionId,
                OSCL_wString& aFileName) = 0;

        /**
         * Retrieves the access interface factory to be used for all content
         * access. The usage of the access factory could sometimes depend on
         * a successful authorization.
         **/
        virtual PVMFStatus GetContentAccessFactory(PVMFSessionId aSessionId,
                PVMFCPMPluginAccessInterfaceFactory*& aContentAccessFactory) = 0;

        /**
         * Approves the specified usage of the content, based on the
         * interactions with all the authenticated plugins.
         **/
        virtual PVMFCommandId ApproveUsage(PVMFSessionId aSessionId,
                                           PvmiKvp& aRequestedUsage,
                                           PvmiKvp& aApprovedUsage,
                                           PvmiKvp& aAuthorizationData,
                                           PVMFCPMUsageID& aUsageID,
                                           const OsclAny* aContext = NULL) = 0;

        /**
         * Called by the user once the content usage is complete.
         **/
        virtual PVMFCommandId UsageComplete(PVMFSessionId aSessionId,
                                            PVMFCPMUsageID& aUsageID,
                                            OsclAny* aContext = NULL) = 0;

        virtual PVMFCommandId CloseSession(PVMFSessionId& aSessionId,
                                           const OsclAny* aContext = NULL) = 0;

        /**
         * Resets the content policy manager.  The CPM should relinquish all resources
         * that is has acquired as part of the initialization process and should be
         * ready to be deleted when this completes.
         **/
        virtual PVMFCommandId Reset(const OsclAny* aContext = NULL) = 0;

        /**
         * Returns a pointer to the current plugin registry, for use in the
         * rare case where code needs to access plugins directly.
         **/
        virtual CPMPluginRegistry* GetPluginRegistry() = 0;

        /**
         * Provides a means to query extension interfaces either from the CPM
         * directly or from underlying plugins.
         * Example: Metadata, License Acquisition etc
         **/
        virtual PVMFCommandId QueryInterface(PVMFSessionId aSessionId,
                                             const PVUuid& aUuid,
                                             PVInterface*& aInterfacePtr,
                                             const OsclAny* aContext = NULL) = 0;

        virtual ~PVMFCPM() {}
};

class PVMFCPMFactory
{
    public:
        /**
         * Creates an instance of a PVMFCPM.
         * If the creation fails, this function will leave.
         *
         * @param observer for PVMFCPM
         *
         * @returns A pointer to an instance of PVMFCPM
         * leaves if instantiation fails
         **/
        OSCL_IMPORT_REF static PVMFCPM* CreateContentPolicyManager(PVMFCPMStatusObserver& aObserver);
        /**
         * Deletes an instance of PVMFCPM
         * and reclaims all allocated resources.
         *
         * @param aNode The PVMFCPM instance to be deleted
         * @returns None
         **/
        OSCL_IMPORT_REF static void DestroyContentPolicyManager(PVMFCPM*);
};

#endif //CPM_H_INCLUDED


