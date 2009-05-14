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

#include "oscl_base.h"

#include "oscl_tls.h"
#include "oscl_assert.h"

//Use a magic number to help detect un-initialized TLS.
#define OSCL_TLS_MAGIC_NUMBER 0x8765abcd
#define OSCL_TLS_REGISTRY_VALID(reg) (reg!=0 && reg[OSCL_TLS_ID_MAGICNUM]==(OsclAny*)OSCL_TLS_MAGIC_NUMBER)

OSCL_EXPORT_REF void TLSStorageOps::save_registry(TOsclTlsKey* key, OsclAny* ptr, int32 &aError)
{
    aError = 0;
#if (OSCL_TLS_IS_KEYED)
    OSCL_ASSERT(key);
    if (!OSCL_TLS_STORE_FUNC(*key, ptr))
    {
        aError = EPVErrorBaseSystemCallFailed;
        return;
    }
#else
    OSCL_UNUSED_ARG(key);
    if (!OSCL_TLS_STORE_FUNC(ptr))
    {
        aError = EPVErrorBaseSystemCallFailed;
        return;
    }
#endif
}

OSCL_EXPORT_REF OsclAny* TLSStorageOps::get_registry(TOsclTlsKey* key)
{
#if (OSCL_TLS_IS_KEYED)
    OSCL_ASSERT(key);
    return OSCL_TLS_GET_FUNC(*key);
#else
    OSCL_UNUSED_ARG(key);
    return OSCL_TLS_GET_FUNC();
#endif
}

#if (OSCL_TLS_IS_KEYED)

//Global var for Tls Key.
OsclTLSRegistry::TlsKey* OsclTLSRegistry::iTlsKey = NULL;


#endif //OSCL_TLS_IS_KEYED



OSCL_EXPORT_REF void OsclTLSRegistry::initialize(Oscl_DefAlloc &alloc, int32 &aError)
{
    TOsclTlsKey* pkey = NULL;
    aError = 0;

#if ( OSCL_TLS_IS_KEYED)
    //Allocate the table on the first init call.
    //Note there's some risk of thread contention here, since
    //the thread lock is not available until after this step.
    if (!iTlsKey)
    {
        OsclAny* table = alloc.allocate(sizeof(TlsKey));
        if (!table)
        {
            aError = EPVErrorBaseOutOfMemory;
            return;
        }

        //allocate space for key
        pkey = (TOsclTlsKey*)alloc.allocate(sizeof(TOsclTlsKey));
        if (!pkey)
        {
            aError = EPVErrorBaseOutOfMemory;
            alloc.deallocate(table);
            return;
        }

        //create key for this thread.
        if (!OSCL_TLS_KEY_CREATE_FUNC(*pkey))
        {
            aError = EPVErrorBaseSystemCallFailed;
            alloc.deallocate(pkey);
            alloc.deallocate(table);
            return;
        }

        iTlsKey = new(table) TlsKey();
        iTlsKey->iLock.Lock();
        iTlsKey->iRefCnt++;
        iTlsKey->iOsclTlsKey = pkey;
        iTlsKey->iLock.Unlock();
    }
    else
    {
        iTlsKey->iLock.Lock();
        iTlsKey->iRefCnt++;
        pkey = iTlsKey->iOsclTlsKey;
        iTlsKey->iLock.Unlock();
    }

#endif

    // allocate the space and save the pointer
    registry_pointer_type registry = OSCL_STATIC_CAST(registry_pointer_type,
                                     alloc.allocate(sizeof(registry_type) * OSCL_TLS_MAX_SLOTS));
    if (registry == 0)
    {
        aError = EPVErrorBaseOutOfMemory;
        return;
    }

    // initialize all TLSs to 0
    for (uint32 ii = 0; ii < OSCL_TLS_MAX_SLOTS; ii++)
        registry[ii] = 0;
    // initialize the magic number
    registry[OSCL_TLS_ID_MAGICNUM] = (OsclAny*)OSCL_TLS_MAGIC_NUMBER;

    // save it away
    TLSStorageOps::save_registry(pkey, registry, aError);
}

OSCL_EXPORT_REF void OsclTLSRegistry::cleanup(Oscl_DefAlloc &alloc, int32 &aError)
{
    TOsclTlsKey* pkey = NULL;
    aError = 0;

#if (OSCL_TLS_IS_KEYED)
    if (!iTlsKey)
    {
        aError = EPVErrorBaseNotInstalled;//No key!
        return;
    }
    pkey = iTlsKey->iOsclTlsKey;
#endif

    //Cleanup this thread's registry
    registry_pointer_type registry = OSCL_STATIC_CAST(registry_pointer_type , TLSStorageOps::get_registry(pkey));
    if (!OSCL_TLS_REGISTRY_VALID(registry))
    {
        aError = EPVErrorBaseNotInstalled;//No registry!
        return;
    }
    alloc.deallocate(registry);

    TLSStorageOps::save_registry(pkey, NULL, aError);
    if (aError)
        return;

#if (OSCL_TLS_IS_KEYED)

    //Remove Tls key

    iTlsKey->iLock.Lock();
    iTlsKey->iRefCnt--;
    if (iTlsKey->iRefCnt == 0)
    {
        //Deallocate key.
        OSCL_TLS_KEY_DELETE_FUNC(*pkey);
        alloc.deallocate(pkey);
        iTlsKey->iLock.Unlock();
        iTlsKey->~TlsKey();
        alloc.deallocate(iTlsKey);
        iTlsKey = NULL;
    }
    else
    {
        iTlsKey->iLock.Unlock();
    }
#endif
}

OSCL_EXPORT_REF OsclAny* OsclTLSRegistry::getInstance(uint32 ID, int32 &aError)
{
    OSCL_ASSERT(ID < OSCL_TLS_MAX_SLOTS);

    aError = 0;

    TOsclTlsKey* pkey = NULL;

#if (OSCL_TLS_IS_KEYED)
    if (!iTlsKey)
    {
        aError = EPVErrorBaseNotInstalled;//No table!
        return NULL;
    }
    pkey = iTlsKey->iOsclTlsKey;
#endif

    registry_pointer_type registry = OSCL_STATIC_CAST(registry_pointer_type , TLSStorageOps::get_registry(pkey));
    if (!OSCL_TLS_REGISTRY_VALID(registry))
    {
        aError = EPVErrorBaseNotInstalled;//No registry!
        return NULL;
    }

    return registry[ID];
}

OSCL_EXPORT_REF void OsclTLSRegistry::registerInstance(OsclAny* ptr, uint32 ID, int32 &aError)
{
    OSCL_ASSERT(ID < OSCL_TLS_MAX_SLOTS);

    aError = 0;
    TOsclTlsKey *pkey = NULL;

#if (OSCL_TLS_IS_KEYED)
    if (!iTlsKey)
    {
        aError = EPVErrorBaseNotInstalled;//No table!
        return ;
    }
    pkey = iTlsKey->iOsclTlsKey;
#endif

    registry_pointer_type registry = OSCL_STATIC_CAST(registry_pointer_type , TLSStorageOps::get_registry(pkey));
    if (!OSCL_TLS_REGISTRY_VALID(registry))
    {
        aError = EPVErrorBaseNotInstalled;//no registry!
        return;
    }

    registry[ID] = ptr;

}



