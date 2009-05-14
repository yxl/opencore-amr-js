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
#ifndef PVMF_CPMPLUGIN_PASSTHRU_OMA1_FACTORY_H_INCLUDED
#define PVMF_CPMPLUGIN_PASSTHRU_OMA1_FACTORY_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVMF_CPMPLUGIN_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_interface.h"
#endif

//Oma1 Passthru plugin factory.
class PVMFOma1PassthruPluginFactory : public PVMFCPMPluginFactory
{
    public:
        OSCL_IMPORT_REF PVMFOma1PassthruPluginFactory();
        OSCL_IMPORT_REF PVMFOma1PassthruPluginFactory(bool aAuthorizeUsage, bool aCancelAcquireLicense,
                bool aSourceInitDataNotSupported,
                PVMFCPMContentType aCPMContentType);
        //from PVMFCPMPluginFactory
        OSCL_IMPORT_REF PVMFCPMPluginInterface* CreateCPMPlugin();
        OSCL_IMPORT_REF void DestroyCPMPlugin(PVMFCPMPluginInterface* aPlugIn);
        bool iFailAuthorizeUsage;
        bool iCancelAcquireLicense;
        bool iSourceInitDataNotSupported;
        PVMFCPMContentType iCPMContentType;
    private:

};

#endif //PVMF_CPMPLUGIN_PASSTHRU_OMA1_FACTORY_H_INCLUDED



