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
#ifndef PVMF_CPMPLUGIN_PASSTHRU_OMA1_CONFIG_H_INCLUDED
#define PVMF_CPMPLUGIN_PASSTHRU_OMA1_CONFIG_H_INCLUDED

#ifndef CPM_TYPES_H
#include "cpm_types.h"
#endif
#ifndef CPM_PLUGIN_REGISTRY_H
#include "cpm_plugin_registry.h"
#endif
#ifndef PVMF_CPMPLUGIN_PASSTHRU_OMA1_H_INCLUDED
#include "pvmf_cpmplugin_passthru_oma1.h"
#endif
#ifndef PVMF_CPMPLUGIN_PASSTHRU_OMA1_TYPES_H_INCLUDED
#include "pvmf_cpmplugin_passthru_oma1_types.h"
#endif
class PVMFCPMOma1PassthruUtils
{
    public:
        static CPMPluginRegistry* PopulateCPMPluginRegistry()
        {
            //Create registry
            CPMPluginRegistry* _pRegistry = CPMPluginRegistryFactory::CreateCPMPluginRegistry();

            //Create a passthru OMA1 plugin.
            PVMFCPMPluginInterface* _pPVMFCPMPluginPassthruOMA1 =
                PVMFCPMPassThruPlugInOMA1::CreatePlugIn();

            //Package the plugin with its user authentication data.
            CPMPluginContainer container(*_pPVMFCPMPluginPassthruOMA1, NULL);

            //Define the plugin mime type
            OSCL_StackString<100> mimestr(PVMF_CPM_MIME_PASSTHRU_OMA1);

            //Add the plugin to the registry
            _pRegistry->addPluginToRegistry(mimestr, container);

            return (_pRegistry);
        };

        static void DePopulateCPMPluginRegistry(CPMPluginRegistry* aRegistry)
        {
            if (aRegistry)
            {
                //Lookup the Passthru OM1 plugin
                OSCL_StackString<100> mimestr(PVMF_CPM_MIME_PASSTHRU_OMA1);

                CPMPluginContainer* _pContainer =
                    aRegistry->lookupPlugin(mimestr);

                if (_pContainer != NULL)
                {
                    //delete it & remove from registry
                    PVMFCPMPassThruPlugInOMA1::DestroyPlugIn(&_pContainer->PlugIn());
                    aRegistry->removePluginFromRegistry(mimestr);
                }

                //Destroy the plugin registry
                CPMPluginRegistryFactory::DestroyCPMPluginRegistry(aRegistry);

            }
        }
};


#endif //PVMF_CPMPLUGIN_PASSTHRU_OMA1_CONFIG_H_INCLUDED



