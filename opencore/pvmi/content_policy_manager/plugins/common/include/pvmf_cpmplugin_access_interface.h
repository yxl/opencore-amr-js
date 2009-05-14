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
#ifndef PVMF_CPMPLUGIN_ACCESS_INTERFACE_H_INCLUDED
#define PVMF_CPMPLUGIN_ACCESS_INTERFACE_H_INCLUDED

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
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PVMI_DATA_STREAM_INTERFACE_H_INCLUDED
#include "pvmi_data_stream_interface.h"
#endif

#define PVMF_CPMPLUGIN_ACCESS_INTERFACE_MIMETYPE "pvxxx/pvmf/cpm/plugin/access_interface"
#define PVMFCPMPluginAccessInterfaceUuid PVUuid(0xc5f05532,0x7bcb,0x4186,0x85,0xa6,0xfa,0x9e,0x42,0x2d,0xb7,0x6d)

/**
 * Base Content Access Interface for all Content Policy Manager Plugins
 */
class PVMFCPMPluginAccessInterface : public PVInterface
{
    public:
        virtual void Init(void) = 0;
        virtual void Reset(void) = 0;
};

#define PVMF_CPMPLUGIN_DECRYPTION_INTERFACE_MIMETYPE "pvxxx/pvmf/cpm/plugin/remote_sync_access_interface"
#define PVMFCPMPluginDecryptionInterfaceUuid PVUuid(0x1e14b2fe,0x947b,0x49c8,0x8b,0x11,0xe9,0xec,0x4c,0x11,0xa2,0x8f)

/**
 * Access Unit Decryption Interface for Content Policy Manager Plugins
 */
class PVMFCPMPluginAccessUnitDecryptionInterface : public PVMFCPMPluginAccessInterface
{
    public:
        /**
         * DecryptAccessUnit operation
         * Decrypts input buffer of size 'aInputSizeInBytes'
         *
         * @param aInputBuffer pointer to input buffer containing
         *                     encrypted data of type uint8*
         *
         * @param aInputBufferSizeInBytes  input buffer size in bytes,
         *
         * @param aOutputBuffer pointer to output buffer containing
         *                      decrypted data of type uint8*,
            *                      memory for the buffer allocated by the caller
         *
         * @param aOutputBufferSizeInBytes   size of allocated output buffer
         *                                   in bytes
         *
         * @return returns true on successful decryption, false otherwise
         */
        virtual bool DecryptAccessUnit(uint8*& aInputBuffer,
                                       uint32  aInputBufferSizeInBytes,
                                       uint8*& aOutputBuffer,
                                       uint32& aOutputBufferSizeInBytes,
                                       uint32  aTrackID = 0,
                                       uint32  aAccesUnitTimeStamp = 0) = 0;
        virtual int32 GetDecryptError() = 0;

        /**
        * For in-place decryption
        */
        virtual bool CanDecryptInPlace() = 0;
        virtual bool DecryptAccessUnit(uint8*& aInputBuffer,
                                       uint32  aInputBufferSizeInBytes,
                                       uint32  aTrackID = 0,
                                       uint32  aAccesUnitTimeStamp = 0) = 0;

        /**
        * Optional version of DecryptAccessUnit with a decrypt context
        */
        virtual bool DecryptAccessUnit(uint8*& aInputBuffer,
                                       uint32  aInputBufferSizeInBytes,
                                       uint32  aTrackID ,
                                       uint32  aAccesUnitTimeStamp,
                                       PVInterface* aContext = NULL)
        {
            OSCL_UNUSED_ARG(aInputBuffer);
            OSCL_UNUSED_ARG(aInputBufferSizeInBytes);
            OSCL_UNUSED_ARG(aTrackID);
            OSCL_UNUSED_ARG(aAccesUnitTimeStamp);
            OSCL_UNUSED_ARG(aContext);
            return false;
        }

};


#endif //PVMF_CPMPLUGIN_ACCESS_INTERFACE_H_INCLUDED

