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
#ifndef PVMI_KVP_UTIL_H_INCLUDED
#define PVMI_KVP_UTIL_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif

#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif

/**
 * This function returns the type parameter ("type=") index for the input key string
 *
 * @param aKeyString      The input key string to determine the type parameter
 * @returns               One of PvmiKvpType enum. If the type cannot be determined PVMI_KVPTYPE_UNKNOWN will be returned
 */
OSCL_IMPORT_REF PvmiKvpType GetTypeFromKeyString(PvmiKeyType aKeyString);

/**
 * This function returns the attribute parameter ("attr=") index for the input key string
 *
 * @param aKeyString      The input key string to determine the attribute parameter
 * @returns               One of PvmiKvpAttr enum. If the type cannot be determined PVMI_KVPATTR_UNKNOWN will be returned
 */
OSCL_IMPORT_REF PvmiKvpAttr GetAttrTypeFromKeyString(PvmiKeyType aKeyString);

/**
 * This function returns the value type parameter ("valtype=") index for the input key string
 *
 * @param aKeyString      The input key string to determine the value type parameter
 * @returns               One of PvmiKvpValueType enum. If the type cannot be determined PVMI_KVPVALTYPE_UNKNOWN will be returned
 */
OSCL_IMPORT_REF PvmiKvpValueType GetValTypeFromKeyString(PvmiKeyType aKeyString);


class PVMFCreateKVPUtils
{
    public:
        OSCL_IMPORT_REF static PVMFStatus CreateKVPForWStringValue(PvmiKvp& aKeyVal,
                const char* aKeyTypeString,
                OSCL_wString& aValString,
                char* aMiscKeyParam = NULL,
                uint32 aMaxSize = 0xFFFFFFFF,
                uint32 aTruncateFlag = 1);

        OSCL_IMPORT_REF static PVMFStatus CreateKVPForCharStringValue(PvmiKvp& aKeyVal,
                const char* aKeyTypeString,
                const char* aValString,
                char* aMiscKeyParam = NULL,
                uint32 aMaxSize = 0xFFFFFFFF,
                uint32 aTruncateFlag = 1);

        OSCL_IMPORT_REF static PVMFStatus CreateKVPForByteArrayValue(PvmiKvp& aKeyVal,
                const char* aKeyTypeString,
                uint8* aValue,
                uint32 aValueLen,
                char* aMiscKeyParam = NULL,
                uint32 aMaxSize = 0xFFFFFFFF);

        OSCL_IMPORT_REF static PVMFStatus CreateKVPForUInt32Value(PvmiKvp& aKeyVal,
                const char* aKeyTypeString,
                uint32& aValueUInt32,
                char* aMiscKeyParam = NULL);

        OSCL_IMPORT_REF static PVMFStatus CreateKVPForFloatValue(PvmiKvp& aKeyVal,
                const char* aKeyTypeString,
                float& aValueFloat,
                char* aMiscKeyParam = NULL);

        OSCL_IMPORT_REF static PVMFStatus CreateKVPForBoolValue(PvmiKvp& aKeyVal,
                const char* aKeyTypeString,
                bool& aValueBool,
                char* aMiscKeyParam = NULL);

        OSCL_IMPORT_REF static PVMFStatus CreateKVPForKSVValue(PvmiKvp& aKeyVal,
                const char* aKeyTypeString,
                OsclAny* aValue,
                char* aMiscKeyParam = NULL);

};

#endif //PVMI_KVP_UTIL_H_INCLUDED

