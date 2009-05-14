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
#ifndef PVMI_DATASTREAMUSER_INTERFACE_H_INCLUDED
#define PVMI_DATASTREAMUSER_INTERFACE_H_INCLUDED

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
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PVMF_CPMPLUGIN_ACCESS_INTERFACE_FACTORY_H_INCLUDED
#include "pvmf_cpmplugin_access_interface_factory.h"
#endif
#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h" // for PvmfMimeString = OSCL_String
#endif

#define PVMI_DATASTREAMUSER_INTERFACE_MIMETYPE "pvxxx/pvmf/streamuser"
#define PVMIDatastreamuserInterfaceUuid PVUuid(0x8f5ccbd8,0x3331,0x46ec,0xad,0x6f,0x39,0xec,0x22,0x2b,0x44,0xb9)

// A PVMFDataStreamFactory is really a PVMFCPMPluginAccessInterfaceFactory.
typedef PVMFCPMPluginAccessInterfaceFactory PVMFDataStreamFactory;

/* This interface is implemented by objects that support having data stream factories passed to them. */

class PVMFDataStreamReadCapacityObserver;

class PVMIDatastreamuserInterface : public PVInterface
{
    public:
        /**
         *  Provides a datastream factory along with supplemental information about how to use it.
         *  @param aFactory A reference to the factory. Provides methods to create sync or async data stream objects.
         *  @param aFactoryTag Provides information about the purpose for which this factory is being provided.
         *  @param aFactoryConfig Provides configuration information for use with the factory.
         */
        virtual void PassDatastreamFactory(PVMFDataStreamFactory& aFactory, int32 aFactoryTag, const PvmfMimeString* aFactoryConfig = NULL) = 0;
        virtual void PassDatastreamReadCapacityObserver(PVMFDataStreamReadCapacityObserver* aObserver)
        {
            OSCL_UNUSED_ARG(aObserver);
            OSCL_LEAVE(OsclErrNotSupported);
        }
};



#endif //PVMI_DATASTREAMUSER_INTERFACE_H_INCLUDED


