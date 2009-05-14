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
#include "oscl_defalloc.h"
#include "oscl_dll.h"
#include "oscl_tls.h"
#include "oscl_singleton.h"
#include "oscl_base_alloc.h"



OSCL_EXPORT_REF int32 OsclBase::Init()
{
    {
        _OsclBasicAllocator alloc;
        int32 error;
        OsclTLSRegistry::initialize(alloc, error);
        //exit on error
        if (error)
            return error;
    }
#if (OSCL_HAS_SINGLETON_SUPPORT)
    {
        _OsclBasicAllocator alloc;
        int32 error;
        OsclSingletonRegistry::initialize(alloc, error);
        //exit on error
        if (error)
            return error;
    }
#endif
    return 0;
}

OSCL_EXPORT_REF int32 OsclBase::Cleanup()
{
    int32 result = 0;
#if (OSCL_HAS_SINGLETON_SUPPORT)
    {
        _OsclBasicAllocator alloc;
        int32 error;
        OsclSingletonRegistry::cleanup(alloc, error);
        //continue if error
        if (error)
            result = error;
    }
#endif
    {
        _OsclBasicAllocator alloc;
        int32 error;
        OsclTLSRegistry::cleanup(alloc, error);
        //continue if error
        if (error)
            result = error;
    }
    //return the last error encountered.
    return result;
}

void PVOsclBase_Init()
{
    OsclBase::Init();
}

void PVOsclBase_Cleanup()
{
    OsclBase::Cleanup();
}

OSCL_DLL_ENTRY_POINT_DEFAULT()


#if(OSCL_HAS_BASIC_LOCK)
//
// _OsclBasicLock
//
#include "oscl_lock_base.h"

OSCL_EXPORT_REF _OsclBasicLock::_OsclBasicLock()
{
    iError = 0;


    int result = pthread_mutex_init(&ObjLock, NULL);
    if (result != 0)
        iError = result;

}

OSCL_EXPORT_REF _OsclBasicLock::~_OsclBasicLock()
{

    int result = pthread_mutex_destroy(&ObjLock);
    if (result != 0)
        iError = result;

}


OSCL_EXPORT_REF void _OsclBasicLock::Lock()
{

    int result = pthread_mutex_lock(&ObjLock);
    if (result != 0)
        iError = result;

}


OSCL_EXPORT_REF void _OsclBasicLock::Unlock()
{


    int result = pthread_mutex_unlock(&ObjLock);
    if (result != 0)
        iError = result;

}
#endif //OSCL_HAS_BASIC_LOCK



