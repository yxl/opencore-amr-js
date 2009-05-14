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
#ifndef PVMF_CPMPLUGIN_INTERFACE_H_INCLUDED
#define PVMF_CPMPLUGIN_INTERFACE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef CPM_TYPES_H
#include "cpm_types.h"
#endif

/**
 * PVMFCpmPluginCmdStatusObserver Class
 *
 * PVMFCpmPluginCmdStatusObserver is the PVMF CPM Plugin observer class for
 * notifying the status of issued command messages. The API provides a mechanism
 * for the status of each command to be passed back along with context specific
 * information where applicable.
 * Applications using the module must have a class derived from
 * PVMFCpmPluginCmdStatusObserver and implement the pure virtual function in
 * order to receive event notifications from the plugin.
 **/
class PVMFCPMPluginCmdStatusObserver
{
    public:
        /**
         * Handle an event that has been generated.
         * @param "aResponse"	"The response to a previously issued command."
         */
        virtual void CPMPluginCommandCompleted(const PVMFCmdResp& aResponse) = 0;
        virtual ~PVMFCPMPluginCmdStatusObserver() {}
};


#define PVMF_CPMPLUGIN_DEFAULT_SESSION_RESERVE 10

class PVMFCPMPluginSession
{
    public:
        PVMFCPMPluginSession(PVMFSessionId aId, PVMFCPMPluginCmdStatusObserver& aObs)
                : iId(aId)
                , iCmdStatusObserver(&aObs)
        {}
        PVMFSessionId iId;
        PVMFCPMPluginCmdStatusObserver* iCmdStatusObserver;
};

/**
 * Content Policy Manager Service Plugin interface
 */
class PVMFCPMPluginInterface
{
    public:
        virtual ~PVMFCPMPluginInterface()
        {
            iSessions.clear();
        }

        virtual PVMFSessionId Connect(PVMFCPMPluginCmdStatusObserver &aObs)
        {
            PVMFCPMPluginSession session(iSessions.size(), aObs);
            iSessions.push_back(session);
            if (!iLoggedOn)
            {
                ThreadLogon();
                iLoggedOn = true;
            }
            return session.iId;
        }

        virtual PVMFStatus Disconnect(PVMFSessionId aSessionId)
        {
            for (uint32 i = 0; i < iSessions.size(); i++)
            {
                if (iSessions[i].iId == aSessionId)
                {
                    iSessions.erase(&iSessions[i]);
                    if (iLoggedOn
                            && iSessions.size() == 0)
                    {
                        ThreadLogoff();
                        iLoggedOn = false;
                    }
                    return PVMFSuccess;
                }
            }
            return PVMFFailure;
        }

        /**
         * This API is to allow for extensibility of the CPM plugin interface.
         * It allows a caller to ask for all UUIDs associated with a particular MIME type.
         * If interfaces of the requested MIME type are found within the system, they are added
         * to the UUIDs array.
         *
         * Also added to the UUIDs array will be all interfaces which have the requested MIME
         * type as a base MIME type.  This functionality can be turned off.
         *
         * @param aMimeType The MIME type of the desired interfaces
         * @param aUuids A vector to hold the discovered UUIDs
         * @param aExactUuidsOnly Turns on/off the retrival of UUIDs with aMimeType as a base type
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVMFCommandId QueryUUID(PVMFSessionId aSession,
                                        const PvmfMimeString& aMimeType,
                                        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                        bool aExactUuidsOnly = true,
                                        const OsclAny* aContext = NULL) = 0;

        /**
         * This API is to allow for extensibility of the CPM Plugin interface.
         * It allows a caller to ask for an instance of a particular interface object to be returned.
         * The mechanism is analogous to the COM IUnknown method.  The interfaces are identified with
         * an interface ID that is a UUID as in DCE and a pointer to the interface object is
         * returned if it is supported.  Otherwise the returned pointer is NULL.
         *
         * @param aUuid The UUID of the desired interface
         * @param aInterfacePtr The output pointer to the desired interface
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVMFCommandId QueryInterface(PVMFSessionId aSession,
                                             const PVUuid& aUuid,
                                             PVInterface*& aInterfacePtr,
                                             const OsclAny* aContext = NULL) = 0;

        /**
         * Starts initialization of the plugin.
         * At the minimum, the plugin should be
         * ready to authenticate user after initialization is complete
         **/
        virtual PVMFCommandId Init(PVMFSessionId aSession,
                                   const OsclAny* aContext = NULL) = 0;

        /**
         * Resets the plugin.
         * The plugin should relinquish all resources that is
         * has acquired as part of the initialization process and should be ready
         * to be deleted when this completes.
         **/
        virtual PVMFCommandId Reset(PVMFSessionId aSession,
                                    const OsclAny* aContext = NULL) = 0;

        virtual PVMFStatus SetSourceInitializationData(OSCL_wString& aSourceURL,
                PVMFFormatType& aSourceFormat,
                OsclAny* aSourceData) = 0;

        /**
         * Retrieves the mapped CPM content type for a passed in source. This could
         * be types based on DRM version or content rating etc
         **/
        virtual PVMFCPMContentType GetCPMContentType() = 0;

        /**
         * Optional API to retrieve the original filename of a protected file.
         **/
        virtual PVMFStatus GetCPMContentFilename(OSCL_wString& aFileName)
        {
            OSCL_UNUSED_ARG(aFileName);
            return PVMFErrNotSupported;
        }

        /**
         * Method to cancel an ongoing asynchronous command.
         * This will attempt to interrupt the command and complete it
         * as soon as possible.
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] The command ID of the command to cancel.
         * @param [in] Optional context data.
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId CancelCommand(PVMFSessionId, PVMFCommandId, const OsclAny* aContext = NULL)
        {
            OSCL_UNUSED_ARG(aContext);
            return PVMFErrNotSupported;
        }

        /**
         * Method to cancel all ongoing asynchronous commands.
         * This will attempt to interrupt the command and complete it
         * as soon as possible.
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] The command ID of the command to cancel.
         * @param [in] Optional context data.
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId CancelAllCommands(PVMFSessionId, const OsclAny* aContext = NULL)
        {
            OSCL_UNUSED_ARG(aContext);
            return PVMFErrNotSupported;
        }

    protected:
        PVMFCPMPluginInterface(int32 aSessionReserve =
                                   PVMF_CPMPLUGIN_DEFAULT_SESSION_RESERVE)
        {
            iSessions.reserve(aSessionReserve);
            iLoggedOn = false;
        }

        /**
         * Establishes the usage thread context for the plug-in.
         * The plug-in should do any thread-context sensitive initialization
         * in this call.
         **/
        virtual void ThreadLogon() = 0;

        /**
         * Exits the usage thread context for the plug-in.
         * The plug-in should do any thread-context sensitive cleanup
         * in this call.
         **/
        virtual void ThreadLogoff() = 0;

        Oscl_Vector<PVMFCPMPluginSession, OsclMemAllocator> iSessions;
        bool iLoggedOn;

        virtual void ReportCmdCompleteEvent(PVMFSessionId s,
                                            PVMFCmdResp &resp)
        {
            for (uint32 i = 0;i < iSessions.size();i++)
            {
                if (iSessions[i].iId == s)
                {
                    iSessions[i].iCmdStatusObserver->CPMPluginCommandCompleted(resp);
                    break;
                }
            }
        }
};

/**
 * Content Policy Manager Plugin Factory interface
 */
class PVMFCPMPluginFactory
{
    public:
        virtual PVMFCPMPluginInterface* CreateCPMPlugin() = 0;
        virtual void DestroyCPMPlugin(PVMFCPMPluginInterface*) = 0;
        virtual ~PVMFCPMPluginFactory() {}
};

/**
 * Content Policy Manager Plugin Registry Populator interface for use
 * with dynamic loading.
 */
class OSCL_String;
class PVMFCPMPluginRegistryPopulator
{
    public:
        /*
        ** GetFactoryAndMimeString.  Called by CPM framework to retrieve the plugin factory and
        **   plugin mimestring.  Note this will be called twice-- when creating the plugin and
        **   again when destroying the plugin.
        **
        ** @param (out) aMimestring: the plugin mimestring
        ** @return : factory pointer.
        */
        virtual PVMFCPMPluginFactory* GetFactoryAndMimeString(OSCL_String& aMimestring) = 0;
        /*
        ** ReleaseFactory.  Called by CPM framework to indicate that the the plugin factory
        **   retrieved by prior calls to GetFactoryAndMimeString is no longer needed.
        */
        virtual void ReleaseFactory() = 0;
};

#define PVMF_CPM_PLUGIN_REGISTRY_POPULATOR_UUID OsclUuid(0x8c988150,0x9b1b,0x11dd,0xad,0x8b,0x08,0x00,0x20,0x0c,0x9a,0x66)

#endif //PVMF_CPMPLUGIN_INTERFACE_H_INCLUDED

