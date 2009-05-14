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

#include "oscl_error.h"
#include "oscl_defalloc.h"
#include "pvmf_format_type.h"
#include "pvmi_config_and_capability_utils.h"
#include "pv_mime_string_utils.h"


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmiCapabilityAndConfigPortFormatImpl::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters, int& num_parameter_elements,
        PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aSession);

    num_parameter_elements = 0;

    if (pv_mime_strcmp(aIdentifier, iFormatTypeString.get_str()) != 0)
    {
        if (pv_mime_strstr(aIdentifier, iFormatTypeString.get_str()))
        {
            // if we have a 'cap' or 'cur' parameter we can return
            // ok
            char *param;
            if (pv_mime_string_extract_param(0, aIdentifier, param))
            {
                if (oscl_strncmp(param, "attr=cap", oscl_strlen("attr=cap")) &&
                        oscl_strncmp(param, "attr=cur", oscl_strlen("attr=cur")))
                    return PVMFErrNotSupported;
            }
        }
    }

    uint32 strLen = iFormatValTypeString.get_size() + 1;
    OsclMemAllocator alloc;
    uint8* ptr = (uint8*)alloc.ALLOCATE(sizeof(PvmiKvp) + strLen);
    if (!ptr)
    {
        return PVMFErrNoMemory;
    }

    num_parameter_elements = 1;

    aParameters = new(ptr) PvmiKvp;
    ptr += sizeof(PvmiKvp);
    aParameters->key = (PvmiKeyType)ptr;
    oscl_strncpy(aParameters->key, iFormatValTypeString.get_cstr(), strLen);
    aParameters->value.pChar_value = (char*)iFormat.getMIMEStrPtr();
    aParameters->length = aParameters->capacity = strLen;

    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmiCapabilityAndConfigPortFormatImpl::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    if ((num_elements != 1) ||
            (pv_mime_strcmp(aParameters->key, iFormatValTypeString.get_str()) != 0))
    {
        return PVMFFailure;
    }
    OsclMemAllocator alloc;
    alloc.deallocate((OsclAny*)aParameters);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmiCapabilityAndConfigPortFormatImpl::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
        int num_elements, PvmiKvp * & aRet_kvp)
{
    OSCL_UNUSED_ARG(aSession);
    if (!aParameters || (num_elements != 1) ||
            (pv_mime_strcmp(aParameters->key, iFormatValTypeString.get_str()) != 0))
    {
        aRet_kvp = aParameters;
        OSCL_LEAVE(OsclErrArgument);
    }
    else if (IsFormatSupported(aParameters->value.pChar_value))
    {
        aRet_kvp = NULL;
        iFormat = (PVMFFormatType)aParameters->value.pChar_value;
        //notify derived class of format update.
        //This function may leave.
        FormatUpdated();
    }
    else
    {
        aRet_kvp = aParameters;
        OSCL_LEAVE(OsclErrArgument);
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmiCapabilityAndConfigPortFormatImpl::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    OSCL_UNUSED_ARG(aSession);

    if ((num_elements != 1) ||
            (pv_mime_strcmp(aParameters->key, iFormatValTypeString.get_str()) != 0))
    {
        return PVMFErrNotSupported;
    }

    if (IsFormatSupported(aParameters->value.pChar_value))
    {
        return PVMFSuccess;
    }
    return PVMFErrNotSupported;
}


OSCL_EXPORT_REF void pvmiSetPortFormatSync(PvmiCapabilityAndConfig *aPort, const char* aFormatValType, PVMFFormatType aFormat)
{
    // Create PvmiKvp for capability settings
    OsclMemAllocator alloc;
    PvmiKvp kvp;
    kvp.key = NULL;
    kvp.length = oscl_strlen(aFormatValType) + 1; // +1 for \0
    kvp.capacity = kvp.length;
    kvp.key = (PvmiKeyType)alloc.ALLOCATE(kvp.length);
    OsclError::LeaveIfNull(kvp.key);
    oscl_strncpy(kvp.key, aFormatValType, kvp.length);
    kvp.value.pChar_value = (char*)aFormat.getMIMEStrPtr();

    PvmiKvp* retKvp = NULL; // for return value
    aPort->setParametersSync(NULL, &kvp, 1, retKvp);

    alloc.deallocate(kvp.key);
}

OSCL_EXPORT_REF PVMFStatus AllocateKvp(OsclMemAllocator& aAlloc, PvmiKvp*& aKvp, PvmiKeyType aKey, int32 aNumParams)
{
    uint8* buf = NULL;
    uint32 keyLen = oscl_strlen(aKey) + 1;
    int32 err = 0;

    OSCL_TRY(err,
             buf = (uint8*)aAlloc.ALLOCATE(aNumParams * (sizeof(PvmiKvp) + keyLen));
             if (!buf)
             OSCL_LEAVE(OsclErrNoMemory);
            );
    OSCL_FIRST_CATCH_ANY(err,
                         return PVMFErrNoMemory;
                        );

    int32 i = 0;
    PvmiKvp* curKvp = aKvp = new(buf) PvmiKvp;
    buf += sizeof(PvmiKvp);
    for (i = 1; i < aNumParams; i++)
    {
        curKvp += i;
        curKvp = new(buf) PvmiKvp;
        buf += sizeof(PvmiKvp);
    }

    for (i = 0; i < aNumParams; i++)
    {
        aKvp[i].key = (char*)buf;
        oscl_strncpy(aKvp[i].key, aKey, keyLen);
        buf += keyLen;
    }

    return PVMFSuccess;
}

