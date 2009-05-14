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
#ifndef PVMF_CPMPLUGIN_FACTORY_REGISTRY_H_INCLUDED
#define PVMF_CPMPLUGIN_FACTORY_REGISTRY_H_INCLUDED

#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif

class PVMFCPMPluginFactory;
class OsclRegistryClient;

/**
 * MIME type for CPM-plugins.  The complete mime-string must include
 *  this string, followed by "/", followed by the plugin mime-string,
 *  e.g. X-CPM-PLUGIN/SOMEVENDOR-OMA1
 */
#define PVMF_MIME_CPM_PLUGIN  	"X-CPM-PLUGIN"

/**
 * Content Policy Manager Plugin Factory Registry .
 */
class PVMFCPMPluginFactoryRegistryClient
{
    public:
        OSCL_IMPORT_REF PVMFCPMPluginFactoryRegistryClient();
        OSCL_IMPORT_REF ~PVMFCPMPluginFactoryRegistryClient();

        /*!
         * Connect
         *
         *    The application code calls this to connect to the registry
         *    server.
         *    @param: flag to select per-thread registry as opposed to global
         *       registry.
         *    @return: PVMFSuccess, or one of the PVMF return codes for errors.
         */
        OSCL_IMPORT_REF PVMFStatus Connect(bool aPerThreadRegistry = false);

        /*!
         * RegisterPlugin
         *
         *    The application code calls this to register a plugin factory
         *    function.
         *
         *    @param: Plugin mime type, of the form X-CPM-PLUGIN/...
         *    @param: Plugin factory implementation.
         *    @return: PVMFSuccess, or one of the PVMF return codes for errors.
         */
        OSCL_IMPORT_REF PVMFStatus RegisterPlugin(OSCL_String& aMimeType,
                PVMFCPMPluginFactory& aFactory) ;
        /*!
         * UnRegisterPlugin
         *
         *    The application code can call this to un-register a CPM
         *    plugin factory.
         *
         *    Note: This call is optional.  The plugin registry will
         *      be automatically cleaned up when Close is called.
         *
         *    @param: Plugin mime type
         *    @return: PVMFSuccess, or one of the PVMF return codes for errors.
         */
        OSCL_IMPORT_REF PVMFStatus UnRegisterPlugin(OSCL_String& aMimeType) ;

        /*!
         * Close
         *
         *    The application code calls this to close the registry client
         *    session and cleanup all registered plugins.
         *
         */
        OSCL_IMPORT_REF void Close();

    private:
        OsclRegistryClient* iClient;

};



#endif //PVMF_CPMPLUGIN_INTERFACE_H_INCLUDED

