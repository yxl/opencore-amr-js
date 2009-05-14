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
#ifndef PVMF_DATA_SOURCE_INIT_EXTENSION_H_INCLUDED
#define PVMF_DATA_SOURCE_INIT_EXTENSION_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif
#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif

#define PVMF_DATA_SOURCE_INIT_INTERFACE_MIMETYPE "pvxxx/pvmf/pvmfdatasourceinitalizationinterface"
#define PVMF_DATA_SOURCE_INIT_INTERFACE_UUID PVUuid(0x161ef65f,0x8cd1,0x424d,0x9f,0x7f,0xe3,0xf6,0x35,0x2a,0x6c,0x1e)

class PVMFDataSourceInitializationExtensionInterface : public PVInterface
{
    public:
        virtual void addRef() = 0;
        virtual void removeRef() = 0;
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;

        /**
         * Sets the data necessary to initialize the data source
         *
         * @param aSourceURL Wide character string reference to the URL of the source (e.g. path/filename, HTTP URL, RTSP URL)
         * @param aSourceFormat Reference to a format type describing aSourceURL
         * @param aSourceData Pointer to opaque data which the receiving component would understand (e.g. authentication information)
         * @return PVMFSuccess if successfully set. PVMFErrNoMemory if no memory is available to copy the data. PVMFFailure for all other errors.
         */
        virtual PVMFStatus SetSourceInitializationData(OSCL_wString& aSourceURL,
                PVMFFormatType& aSourceFormat,
                OsclAny* aSourceData) = 0;
        /**
         * Sets the client playback clock for the use of the data source.
         *
         * @param aClientClock Pointer to PVMFMediaClock
         * @return PVMFSuccess if successfully set. PVMFFailure for all other errors.
         */
        virtual PVMFStatus SetClientPlayBackClock(PVMFMediaClock* aClientClock) = 0;

        /**
         * Sets the estimated server clock for the use of the data source.
         *
         * @param aClientClock Pointer to PVMFMediaClock
         * @return PVMFSuccess if successfully set. PVMFFailure for all other errors.
         */
        virtual PVMFStatus SetEstimatedServerClock(PVMFMediaClock* aClientClock) = 0;

};


#define PVMF_DATA_SOURCE_NODE_REGISRTY_INIT_INTERFACE_MIMETYPE "pvxxx/pvmf/pvmfdatasourcenoderegistryinitinterface"
#define PVMF_DATA_SOURCE_NODE_REGISRTY_INIT_INTERFACE_UUID PVUuid(0x4f2d9799,0x7621,0x4712,0x85,0x07,0x61,0xd0,0xae,0xa0,0xcf,0x47)

class PVPlayerNodeRegistryInterface;

class PVMFDataSourceNodeRegistryInitInterface : public PVInterface
{
    public:
        /**
         * Sets the node registry for the use of the data source
         *
         * @param aRegistry Pointer to PVPlayerNodeRegistryInterface
         * @return PVMFSuccess if successfully set.
         * PVMFFailure for all other errors.
         */
        virtual PVMFStatus SetPlayerNodeRegistry(PVPlayerNodeRegistryInterface* aRegistry) = 0;
};

#endif //PVMF_DATA_SOURCE_INIT_EXTENSION_H_INCLUDED

