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
/*! \file oscl_mem_imp.cpp
    \brief This file contains the implementation of advanced memory APIs
*/

#include "oscl_mem.h"


//////////////////////
// OsclMem
//////////////////////


OSCL_EXPORT_REF void OsclMem::Init()
{
#if(!OSCL_BYPASS_MEMMGT)
    OsclMemGlobalAuditObject::createGlobalMemAuditObject();
#endif
}

OSCL_EXPORT_REF void OsclMem::Cleanup()
{
#if(!OSCL_BYPASS_MEMMGT)
    OsclMemGlobalAuditObject::deleteGlobalMemAuditObject();
#endif
}



//////////////////////
// OsclMemInit
//////////////////////

OSCL_EXPORT_REF void OsclMemInit(OsclAuditCB & auditCB)
{
#if(!OSCL_BYPASS_MEMMGT)
    auditCB.pStatsNode = NULL;
    auditCB.pAudit = (OsclMemGlobalAuditObject::getGlobalMemAuditObject());
#else
    OSCL_UNUSED_ARG(auditCB);
#endif
}

#if(!OSCL_BYPASS_MEMMGT)

#include "oscl_mem_audit.h"

#include "oscl_singleton.h"

//////////////////////
// OsclMemGlobalAuditObject
//////////////////////

OSCL_EXPORT_REF OsclMemGlobalAuditObject::audit_type* OsclMemGlobalAuditObject::getGlobalMemAuditObject()
{
    return (audit_type*)OsclSingletonRegistryEx::getInstance(OSCL_SINGLETON_ID_OSCLMEM);
}

void OsclMemGlobalAuditObject::createGlobalMemAuditObject()
{
    audit_type* audit = (audit_type*)OsclSingletonRegistryEx::lockAndGetInstance(OSCL_SINGLETON_ID_OSCLMEM);
    if (audit)
    {
        audit->iRefCount++;
    }
    else
    {
        OsclAny *ptr = _oscl_malloc(sizeof(audit_type));
        if (!ptr)
        {
            OsclSingletonRegistryEx::registerInstanceAndUnlock(audit, OSCL_SINGLETON_ID_OSCLMEM);
            OsclError::Leave(OsclErrNoMemory);
            return;
        }
        audit = OSCL_PLACEMENT_NEW(ptr, audit_type());
    }
    OsclSingletonRegistryEx::registerInstanceAndUnlock(audit, OSCL_SINGLETON_ID_OSCLMEM);
}

void OsclMemGlobalAuditObject::deleteGlobalMemAuditObject()
{
    audit_type* audit = (audit_type*)OsclSingletonRegistryEx::lockAndGetInstance(OSCL_SINGLETON_ID_OSCLMEM);
    if (!audit)
    {
        OsclSingletonRegistryEx::registerInstanceAndUnlock(audit, OSCL_SINGLETON_ID_OSCLMEM);
        OsclError::Leave(OsclErrNotInstalled);
        return;
    }
    audit->iRefCount--;
    if (audit->iRefCount == 0)
    {
        audit->~OsclMemAudit();
        _oscl_free(audit);
        audit = NULL;
    }
    OsclSingletonRegistryEx::registerInstanceAndUnlock(audit, OSCL_SINGLETON_ID_OSCLMEM);
}

//////////////////////
// Default new/malloc
//////////////////////
OSCL_EXPORT_REF void* _oscl_default_audit_new(size_t nBytes, const char * file_name, const int line_num)
{
    //get global mem audit object.
    OsclAuditCB audit;
    OsclMemInit(audit);
    return _oscl_audit_new(nBytes, audit, file_name, line_num);
}

OSCL_EXPORT_REF void* _oscl_default_audit_malloc(size_t nBytes, const char * file_name, const int line_num)
{
    //get global mem audit object.
    OsclAuditCB audit;
    OsclMemInit(audit);
    return _oscl_audit_malloc(nBytes, audit, file_name, line_num);
}


OSCL_EXPORT_REF void* _oscl_default_audit_calloc(size_t num, size_t nBytes, const char * file_name, const int line_num)
{
    //get global mem audit object.
    OsclAuditCB audit;
    OsclMemInit(audit);
    return _oscl_audit_calloc(num, nBytes, audit, file_name, line_num);
}

OSCL_EXPORT_REF void* _oscl_default_audit_realloc(void* in_ptr, size_t nBytes, const char * file_name, const int line_num)
{
    //get global mem audit object.
    OsclAuditCB audit;
    OsclMemInit(audit);
    return _oscl_audit_realloc(in_ptr, nBytes, audit, file_name, line_num);
}

//////////////////////
// Audit new/malloc
//////////////////////

OSCL_EXPORT_REF void* _oscl_audit_new(size_t nBytes, OsclAuditCB & auditCB, const char * file_name, const int line_num)
{
    void * pTmp = auditCB.pAudit->MM_allocate(auditCB.pStatsNode, nBytes, file_name, line_num);
    if (!pTmp)
        OsclError::LeaveIfNull(pTmp);
    return pTmp;
}

OSCL_EXPORT_REF void* _oscl_audit_malloc(size_t nBytes, OsclAuditCB & auditCB, const char * file_name, const int line_num)
{
    return auditCB.pAudit->MM_allocate(auditCB.pStatsNode, nBytes, file_name, line_num);
}

OSCL_EXPORT_REF void* _oscl_audit_calloc(size_t num, size_t nBytes, OsclAuditCB & auditCB, const char * file_name, const int line_num)
{
    size_t size = num * nBytes;
    void* ptr = _oscl_audit_malloc(size, auditCB, file_name, line_num);
    if (ptr)
        oscl_memset(ptr, 0, size);
    //note: return pointer can be null.
    return ptr;
}

OSCL_EXPORT_REF void* _oscl_audit_realloc(void* in_ptr, size_t nBytes, OsclAuditCB & auditCB, const char * file_name, const int line_num)
{
    if (nBytes > 0)
    {
        //allocate new space
        void* ptr = _oscl_audit_malloc(nBytes, auditCB, file_name, line_num);
        if (in_ptr && ptr)
        {
            //copy current contents to new space.
            //note: the getSize call leaves if it's a bad pointer.
            //just propagate the leave to the caller.
            if (MM_Audit_Imp::getSize(in_ptr) > MM_Audit_Imp::getSize(ptr))
            {
                oscl_memcpy(ptr, in_ptr, MM_Audit_Imp::getSize(ptr));
            }
            else
            {
                oscl_memcpy(ptr, in_ptr, MM_Audit_Imp::getSize(in_ptr));
            }
            //free original space.
            _oscl_audit_free(in_ptr);
        }
        else if (in_ptr)
        {
            return in_ptr;
        }
        //note: return pointer can be null.
        return ptr;
    }
    else
    {
        _oscl_audit_free(in_ptr);
        return NULL;
    }
}

//////////////
// Free
//////////////
OSCL_EXPORT_REF void _oscl_audit_free(void *p)
{
    //always free from the root node-- global audit object.
    //Get the audit root from within the alloc block rather
    //than from TLS, since we can't be sure this call is
    //in the same thread as the allocation.
    OsclMemAudit *pAuditRoot = MM_Audit_Imp::getAuditRoot(p) ;
    if (pAuditRoot)
    {
        pAuditRoot->MM_deallocate(p);
    }
    else
    {//bad or corrupt block?
        OSCL_LEAVE(OsclErrArgument);
    }
}

#else

OSCL_EXPORT_REF void* _oscl_default_new(size_t nBytes)
{
    void * pTmp = _oscl_malloc(nBytes);
    if (!pTmp)
        OsclError::LeaveIfNull(pTmp);
    return pTmp;
}

#endif //OSCL_BYPASS_MEMMGT


