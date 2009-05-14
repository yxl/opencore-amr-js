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
#include "pvmi_datastreamsyncinterface_ref_factory.h"
#include "pvmi_datastreamsyncinterface_ref_impl.h"


OSCL_EXPORT_REF PVMIDataStreamSyncInterfaceRefFactory::PVMIDataStreamSyncInterfaceRefFactory(OSCL_wString& aFileName)
{
    iFileName = aFileName;
    iFileHandle = NULL;
}


OSCL_EXPORT_REF PVMIDataStreamSyncInterfaceRefFactory::PVMIDataStreamSyncInterfaceRefFactory(OsclFileHandle* aFileHandle)
{
    iFileHandle = aFileHandle;
    iFileName = _STRLIT_WCHAR("");
}


OSCL_EXPORT_REF PVMIDataStreamSyncInterfaceRefFactory::~PVMIDataStreamSyncInterfaceRefFactory()
{
    // Nothing needed
}


PVMFStatus PVMIDataStreamSyncInterfaceRefFactory::QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids)
{
    aUuids.push_back(PVMIDataStreamSyncInterfaceUuid);
    return PVMFSuccess;
}


PVInterface* PVMIDataStreamSyncInterfaceRefFactory::CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid)
{
    if (aUuid == PVMIDataStreamSyncInterfaceUuid)
    {
        // Create the ref data stream sync interface object using ref impl's static create method
        PVMIDataStreamSyncInterface* datastreamptr = NULL;
        if (iFileHandle)
        {
            datastreamptr = PVMIDataStreamSyncInterfaceRefImpl::CreateDataStreamSyncInterfaceRefImpl(iFileHandle);
        }
        else
        {
            datastreamptr = PVMIDataStreamSyncInterfaceRefImpl::CreateDataStreamSyncInterfaceRefImpl(iFileName);
        }

        return (OSCL_STATIC_CAST(PVInterface*, datastreamptr));
    }
    return NULL;
}


void PVMIDataStreamSyncInterfaceRefFactory::DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid, PVInterface* aPtr)
{
    if (aUuid == PVMIDataStreamSyncInterfaceUuid)
    {
        // Release the data stream sync interface object using ref impl's static destroy method
        PVMIDataStreamSyncInterface* interimPtr = (PVMIDataStreamSyncInterface*)aPtr;
        PVMIDataStreamSyncInterfaceRefImpl::DestroyDataStreamSyncInterfaceRefImpl(interimPtr);
    }
}


void PVMIDataStreamSyncInterfaceRefFactory::addRef()
{
    // Nothing needed
}


void PVMIDataStreamSyncInterfaceRefFactory::removeRef()
{
    // Nothing needed
}


bool PVMIDataStreamSyncInterfaceRefFactory::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    iface = NULL;
    if (uuid == PVMFCPMPluginAccessInterfaceFactoryUuid)
    {
        // Return the current object as data stream factory pointer
        PVMFDataStreamFactory* myInterface = OSCL_STATIC_CAST(PVMFDataStreamFactory*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return true;
    }
    return false;
}

