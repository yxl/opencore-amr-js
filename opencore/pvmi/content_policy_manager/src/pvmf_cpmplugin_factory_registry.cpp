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

#include "pvmf_cpmplugin_factory_registry.h"
#include "oscl_registry_client.h"
#include "oscl_mem.h"
#include "pvmf_return_codes.h"

OSCL_EXPORT_REF PVMFCPMPluginFactoryRegistryClient::PVMFCPMPluginFactoryRegistryClient()
{
    iClient = NULL;
}

OSCL_EXPORT_REF PVMFCPMPluginFactoryRegistryClient::~PVMFCPMPluginFactoryRegistryClient()
{
    if (iClient)
        OSCL_DELETE(iClient);
}

OSCL_EXPORT_REF PVMFStatus PVMFCPMPluginFactoryRegistryClient::Connect(bool aPerThread)
{
    if (!iClient)
    {
        int32 err;
        OSCL_TRY(err, iClient = OSCL_NEW(OsclRegistryClient, ()););
        if (err != OsclErrNone || !iClient)
            return PVMFErrNoMemory;
    }
    switch (iClient->Connect(aPerThread))
    {
        case OsclErrNone:
            return PVMFSuccess;
        default:
            return PVMFFailure;
    }
}

OSCL_EXPORT_REF PVMFStatus PVMFCPMPluginFactoryRegistryClient::RegisterPlugin(OSCL_String& aMimeType,
        PVMFCPMPluginFactory& aFactory)
{
    if (!iClient)
        return PVMFErrInvalidState;

    //Make sure it's a valid mime-string for a CPM-plugin.  the access
    //interface side will lookup plugins using "X-CPM-PLUGIN" as the
    //header, so all plugins have to match.
    OSCL_HeapString<OsclMemAllocator> cpmregid(PVMF_MIME_CPM_PLUGIN);
    //hierarchical string match.
    //tells whether "id" is a prefix of the component ID (or an exact match).

    if (aMimeType.get_size() >= cpmregid.get_size()
            && oscl_CIstrncmp(cpmregid.get_cstr(), aMimeType.get_cstr(), cpmregid.get_size()) == 0
            && (aMimeType.get_cstr()[cpmregid.get_size()] == '/'
                || aMimeType.get_cstr()[cpmregid.get_size()] == '\0'))
    {
        switch (iClient->Register(aMimeType, (OsclComponentFactory)&aFactory))
        {
            case OsclErrNone:
                return PVMFSuccess;
            case OsclErrAlreadyExists:
                return PVMFErrAlreadyExists;
            case OsclErrNoMemory:
                return PVMFErrNoMemory;
            default:
                return PVMFFailure;
        }
    }
    else
    {
        return PVMFErrArgument;
    }
}

OSCL_EXPORT_REF PVMFStatus PVMFCPMPluginFactoryRegistryClient::UnRegisterPlugin(OSCL_String& aMimeType)
{
    if (!iClient)
        return PVMFErrInvalidState;
    switch (iClient->UnRegister(aMimeType))
    {
        case OsclErrNone:
            return PVMFSuccess;
        default:
            return PVMFFailure;
    }
}

OSCL_EXPORT_REF void PVMFCPMPluginFactoryRegistryClient::Close()
{
    if (iClient)
        iClient->Close();
}



