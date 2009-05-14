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
/**
 * @file pvmf_meta_data_extension.h
 * @brief Extension interface to retrieve metadata
 */

#ifndef PVMF_META_DATA_EXTENSION_H_INCLUDED
#define PVMF_META_DATA_EXTENSION_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_event_handling.h"
#endif
#ifndef PVMF_META_DATA_TYPES_H_INCLUDED
#include "pvmf_meta_data_types.h"
#endif
#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif

#define PVMF_META_DATA_EXTENSION_INTERFACE_MIMETYPE "pvxxx/pvmf/pvmfmetadataextensioninterface"

// UUID for the extension interface
#define KPVMFMetadataExtensionUuid  PVUuid(0x7DBD6D8D,0xB4CC,0x4887,0xB1,0x0B,0x7E,0xE6,0x48,0x47,0xB2,0x05)

/**
 * PVMFMetadataExtensionInterface specifies the extension interface for metadata retrieval
 */
class PVMFMetadataExtensionInterface : public PVInterface
{
    public:
        /**
         * Synchronous method to return the number of metadata keys for the specified query key string
         *
         * @param aQueryKeyString A NULL terminated character string specifying a subset of metadata keys to count.
         *                        If the string is NULL, total number of all keys will be returned
         *
         * @returns The number of metadata keys
         **/
        virtual uint32 GetNumMetadataKeys(char* aQueryKeyString = NULL) = 0;

        /**
         * Synchronous method to return the number of metadata values for the specified list of key strings
         *
         * @param aKeyList A reference to a metadata key list to specifying the values of interest to count
         *
         * @returns The number of metadata values based on the provided key list
         **/
        virtual uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList) = 0;

        /**
         * Asynchronous method to retrieve a list of metadata keys. The subset of all available keys in the node can
         * be specified by providing a combination of query key string, starting index, and maximum number of keys to retrieve
         *
         * @param aSessionId The assigned node session ID to use for this request
         * @param aKeyList A reference to a metadata key list to add the metadata keys
         * @param aStartingKeyIndex Index into the node's total key list that corresponds to the first key entry to retrieve
         * @param aMaxKeyEntries The maximum number of key entries to add to aKeyList. If there is no maximum, set to -1.
         * @param aQueryKeyString Optional NULL terminated character string to select a subset of keys
         * @param aContext Optional opaque data to be passed back to user with the command response
         *
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVMFCommandId GetNodeMetadataKeys(PVMFSessionId aSessionId,
                PVMFMetadataList& aKeyList,
                uint32 aStartingKeyIndex,
                int32 aMaxKeyEntries = -1,
                char* aQueryKeyString = NULL,
                const OsclAny* aContextData = NULL) = 0;

        /**
         * Asynchronous method to retrieve a list of metadata values. The subset of all available values in the node can
         * be specified by providing a combination of key list, starting index, and maximum number of values to retrieve
         *
         * @param aSessionId The assigned node session ID to use for this request
         * @param aKeyList A reference to a metadata key list specifying the metadata values to retrieve
         * @param aValueList A reference to a metadata value list to add the metadata values
         * @param aStartingValueIndex Index into the node's value list specified by the key list that corresponds to the first value entry to retrieve
         * @param aMaxValueEntries The maximum number of value entries to add to aValueList. If there is no maximum, set to -1.
         * @param aContext Optional opaque data to be passed back to user with the command response
         *
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVMFCommandId GetNodeMetadataValues(PVMFSessionId aSessionId,
                PVMFMetadataList& aKeyList,
                Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                uint32 aStartingValueIndex,
                int32 aMaxValueEntries = -1,
                const OsclAny* aContextData = NULL) = 0;

        /**
         * Synchronous method to free up the specified range of metadata keys in the list. It is assumed that caller of this function
         * knows that start and end indices should correspond to metadata keys returned by this particular instance of the
         * metadata extension interface using GetNodeMetadataKeys().
         *
         * @param aKeyList A reference to a metadata key list to free the key entries
         * @param aStartingKeyIndex Index into aKeyList that corresponds to the first key entry to release
         * @param aEndKeyIndex Index into aKeyList that corresponds to the last key entry to release
         *
         * @returns PVMFSuccess if the release of specified keys succeeded. PVMFErrArgument if indices are invalid or the list is empty.
         *          PVMFFailure otherwise.
         **/
        virtual PVMFStatus ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList,
                uint32 aStartingKeyIndex,
                uint32 aEndKeyIndex) = 0;

        /**
         * Synchronous method to free up the specified range of metadata values in the list. It is assumed that caller of this function
         * knows that start and end indices should correspond to metadata values returned by this particular instance of the
         * metadata extension interface using GetNodeMetadataValues().
         *
         * @param aValueList A reference to a metadata value list to free the value entries
         * @param aStartingValueIndex Index into aValueList that corresponds to the first value entry to release
         * @param aEndValueIndex Index into aValueList that corresponds to the last value entry to release
         *
         * @returns PVMFSuccess if the release of specified values succeeded. PVMFErrArgument if indices are invalid or the list is empty.
         *          PVMFFailure otherwise.
         **/
        virtual PVMFStatus ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                uint32 aStartingValueIndex,
                uint32 aEndValueIndex) = 0;

        // From PVInterface
        virtual void addRef() = 0;

        virtual void removeRef() = 0;

        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;
};

#endif // PVMF_META_DATA_EXTENSION_H_INCLUDED
