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
// -*- c++ -*-
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//               O S C L _ S T R I N G   C L A S S

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "oscl_string_containers.h"
#include "oscl_stdstring.h"
#include "oscl_error.h"

// **************************************************************
//					 CHeapRep Implementation
// **************************************************************

OSCL_EXPORT_REF void CHeapRep::set_rep(CHeapRep*& aRep, Oscl_DefAlloc& aAlloc, const char* cp, uint32 len)
//set heap rep to new string.
{
    CHeapRep*newrep = New(aAlloc);
    if (newrep
            && newrep->set(len, cp, aAlloc))
    {
        CHeapRep::assign(aRep, newrep, aAlloc);
    }
    else
    {
        //could not allocate newrep or could not allocate buffer.
        if (newrep)
            aAlloc.deallocate(newrep);
        OsclError::Leave(OsclErrNoMemory);//no memory
    }
}

// **************************************************************
OSCL_EXPORT_REF void CHeapRep::set_rep(CHeapRep*& aRep, Oscl_DefAlloc& aAlloc, const oscl_wchar* cp, uint32 len)
//set heap rep to new string.
{
    CHeapRep*newrep = New(aAlloc);
    if (newrep
            && newrep->set(len, cp, aAlloc))
    {
        CHeapRep::assign(aRep, newrep, aAlloc);
    }
    else
    {
        //could not allocate newrep or could not allocate buffer.
        if (newrep)
            aAlloc.deallocate(newrep);
        OsclError::Leave(OsclErrNoMemory);//no memory
    }
}

// **************************************************************
OSCL_EXPORT_REF void CHeapRep::append_rep(CHeapRep*& aRep, Oscl_DefAlloc& aAlloc, const char* cp, uint32 len)
//set heap rep to current string plus new string.
{
    CHeapRep*newrep = New(aAlloc);
    if (newrep
            && newrep->append((aRep) ? aRep->size : 0, (char*)((aRep) ? aRep->buffer : NULL), len, cp, aAlloc))
    {
        CHeapRep::assign(aRep, newrep, aAlloc);
    }
    else
    {
        //could not allocate newrep or could not allocate buffer.
        if (newrep)
            aAlloc.deallocate(newrep);
        OsclError::Leave(OsclErrNoMemory);//no memory
    }
}

// **************************************************************
OSCL_EXPORT_REF void CHeapRep::append_rep(CHeapRep*& aRep, Oscl_DefAlloc& aAlloc, const oscl_wchar* cp, uint32 len)
//set heap rep to current string plus new string.
{
    CHeapRep*newrep = New(aAlloc);
    if (newrep
            && newrep->append((aRep) ? aRep->size : 0, (oscl_wchar*)((aRep) ? aRep->buffer : NULL), len, cp, aAlloc))
    {
        CHeapRep::assign(aRep, newrep, aAlloc);
    }
    else
    {
        //could not allocate newrep or could not allocate buffer.
        if (newrep)
            aAlloc.deallocate(newrep);
        OsclError::Leave(OsclErrNoMemory);//no memory
    }
}

// **************************************************************
OSCL_EXPORT_REF void CHeapRep::assign(CHeapRep*& dest, CHeapRep* src, Oscl_DefAlloc &alloc)
{
    if (src)
        src->add_ref();
    if (dest)
        dest->remove_ref(alloc);
    dest = src;
}

// **************************************************************
CHeapRep* CHeapRep::New(Oscl_DefAlloc &alloc)
{
    OsclAny *ptr = alloc.ALLOCATE(sizeof(CHeapRep));
    if (ptr)
    {
        CHeapRep*newrep = new(ptr) CHeapRep;
        return newrep;
    }
    return NULL;
}

// **************************************************************
OSCL_EXPORT_REF bool CHeapRep::set(uint32 nsz, const char*cp, Oscl_DefAlloc &alloc)
{
    // allocate enough space including terminator
    OsclAny* ptr = alloc.ALLOCATE(sizeof(char) * (nsz + 1));
    if (!ptr)
    {
        refcount = 0;
        size = 0;
        maxsize = 0;
        buffer = NULL;
        return false;//can't allocate buffer.
    }
    refcount = 0;
    size = nsz;
    maxsize = nsz;
    buffer = ptr;
    if (cp)
        oscl_strncpy((char*)buffer, cp, size);
    ((char*)buffer)[size] = '\0';
    return true;
}
OSCL_EXPORT_REF bool CHeapRep::set(uint32 nsz, const oscl_wchar*cp, Oscl_DefAlloc &alloc)
{
    // allocate enough space including terminator
    OsclAny* ptr = alloc.ALLOCATE(sizeof(oscl_wchar) * (nsz + 1));
    if (!ptr)
    {
        refcount = 0;
        size = 0;
        maxsize = 0;
        buffer = NULL;
        return false;//can't allocate buffer.
    }
    refcount = 0;
    size = nsz;
    maxsize = nsz;
    buffer = ptr;
    if (cp)
        oscl_strncpy((oscl_wchar*)buffer, cp, size);
    ((oscl_wchar*)buffer)[size] = '\0';
    return true;
}

// **************************************************************
OSCL_EXPORT_REF bool CHeapRep::append(uint32 s1, const char*cp1,
                                      uint32 s2, const char*cp2,
                                      Oscl_DefAlloc &alloc)
{
    maxsize = s1 + s2;
    size = maxsize;
    // allocate enough space including terminator
    OsclAny* ptr = alloc.ALLOCATE(sizeof(char) * (maxsize + 1));
    if (!ptr)
    {
        refcount = 0;
        size = 0;
        maxsize = 0;
        buffer = NULL;
        return false;//can't allocate buffer.
    }
    refcount = 0;
    buffer = ptr;
    if (cp1)
        oscl_strncpy((char*)buffer, cp1, s1);
    ((char*)buffer)[s1] = '\0';
    if (cp2)
        oscl_strncat((char*)buffer, cp2, s2);
    ((char*)buffer)[size] = '\0';
    return true;
}

OSCL_EXPORT_REF bool CHeapRep::append(uint32 s1, const oscl_wchar*cp1,
                                      uint32 s2, const oscl_wchar*cp2,
                                      Oscl_DefAlloc &alloc)
{
    maxsize = s1 + s2;
    size = maxsize;
    // allocate enough space including terminator
    OsclAny* ptr = alloc.ALLOCATE(sizeof(oscl_wchar) * (maxsize + 1));
    if (!ptr)
    {
        refcount = 0;
        size = 0;
        maxsize = 0;
        buffer = NULL;
        return false;//can't allocate buffer.
    }
    refcount = 0;
    buffer = ptr;
    if (cp1)
        oscl_strncpy((oscl_wchar*)buffer, cp1, s1);
    ((oscl_wchar*)buffer)[s1] = '\0';
    if (cp2)
        oscl_strncat((oscl_wchar*)buffer, cp2, s2);
    ((oscl_wchar*)buffer)[size] = '\0';
    return true;
}

// **************************************************************
OSCL_EXPORT_REF void CHeapRep::add_ref()
{
    refcount++;
}

// **************************************************************
OSCL_EXPORT_REF void CHeapRep::remove_ref(Oscl_DefAlloc &alloc)
{
    --refcount;
    if (refcount == 0)
    {
        alloc.deallocate(buffer);
        alloc.deallocate(this);
    }
}

// **************************************************************
//					 OSCL_HeapStringA Implementation
//					 OSCL_wHeapStringA Implementation
// **************************************************************

void OSCL_HeapStringA::set_rep(const chartype* cp, uint32 len)
//set heap rep to new string.
{
    CHeapRep::set_rep(iRep, *iAlloc, cp, len);
}

void OSCL_wHeapStringA::set_rep(const chartype* cp, uint32 len)
//set heap rep to new string.
{
    CHeapRep::set_rep(iRep, *iAlloc, cp, len);
}

// **************************************************************
void OSCL_HeapStringA::append_rep(const chartype* cp, uint32 len)
//set heap rep to current string plus new string.
{
    CHeapRep::append_rep(iRep, *iAlloc, cp, len);
}

void OSCL_wHeapStringA::append_rep(const chartype* cp, uint32 len)
//set heap rep to current string plus new string.
{
    CHeapRep::append_rep(iRep, *iAlloc, cp, len);
}

// **************************************************************
void OSCL_HeapStringA::set_rep(const chartype* cp)
{
    uint32 len = (cp) ? oscl_strlen(cp) : 0;
    set_rep(cp, len);
}

void OSCL_wHeapStringA::set_rep(const chartype* cp)
{
    uint32 len = (cp) ? oscl_strlen(cp) : 0;
    set_rep(cp, len);
}


// **************************************************************
void OSCL_HeapStringA::append_rep(const chartype* cp)
{
    uint32 len = (cp) ? oscl_strlen(cp) : 0;
    append_rep(cp, len);
}

void OSCL_wHeapStringA::append_rep(const chartype* cp)
{
    uint32 len = (cp) ? oscl_strlen(cp) : 0;
    append_rep(cp, len);
}


// **************************************************************
void OSCL_HeapStringA::set_rep(const OSCL_String& src)
{
    set_rep(src.get_cstr(), src.get_size());
}

void OSCL_wHeapStringA::set_rep(const OSCL_wString& src)
{
    set_rep(src.get_cstr(), src.get_size());
}


// **************************************************************
void OSCL_HeapStringA::append_rep(const OSCL_String& src)
{
    append_rep(src.get_cstr(), src.get_size());
}

void OSCL_wHeapStringA::append_rep(const OSCL_wString& src)
{
    append_rep(src.get_cstr(), src.get_size());
}


// **************************************************************
OSCL_EXPORT_REF uint32 OSCL_HeapStringA::get_size() const
{
    if (iRep)
        return iRep->size;
    return 0;
}

OSCL_EXPORT_REF uint32 OSCL_wHeapStringA::get_size() const
{
    if (iRep)
        return iRep->size;
    return 0;
}

// **************************************************************
void OSCL_HeapStringA::set_len(uint32 len)
{
    iRep->size = len;
}

void OSCL_wHeapStringA::set_len(uint32 len)
{
    iRep->size = len;
}


// **************************************************************
OSCL_EXPORT_REF uint32 OSCL_HeapStringA::get_maxsize() const
{
    if (iRep)
        return iRep->maxsize;
    return 0;
}

OSCL_EXPORT_REF uint32 OSCL_wHeapStringA::get_maxsize() const
{
    if (iRep)
        return iRep->maxsize;
    return 0;
}


// **************************************************************
OSCL_EXPORT_REF const OSCL_HeapStringA::chartype* OSCL_HeapStringA::get_cstr() const
{
    if (iRep)
        return (chartype*)iRep->buffer;
    return NULL;
}

OSCL_EXPORT_REF const OSCL_wHeapStringA::chartype* OSCL_wHeapStringA::get_cstr() const
{
    if (iRep)
        return (chartype*)iRep->buffer;
    return NULL;
}


// **************************************************************
OSCL_EXPORT_REF OSCL_HeapStringA::chartype* OSCL_HeapStringA::get_str() const
{
    if (iRep)
        return (chartype*)iRep->buffer;
    return NULL;
}

OSCL_EXPORT_REF OSCL_wHeapStringA::chartype* OSCL_wHeapStringA::get_str() const
{
    if (iRep)
        return (chartype*)iRep->buffer;
    return NULL;
}


// **************************************************************
void OSCL_HeapStringA::create(Oscl_DefAlloc *alloc, OsclRefCounter *ref)
{
    iRep = NULL;
    if (alloc)
    {//use an allocator owned by the caller that may also have a ref counter.
        iAllocRef = ref;
        if (iAllocRef)
            iAllocRef->addRef();
        iAlloc = alloc;
    }
    else
    {//use a basic allocator that resides in this object.
        iAlloc = &iDefAlloc;
        iAllocRef = NULL;
    }
}

void OSCL_wHeapStringA::create(Oscl_DefAlloc *alloc, OsclRefCounter *ref)
{
    iRep = NULL;
    if (alloc)
    {//use an allocator owned by the caller that may also have a ref counter.
        iAllocRef = ref;
        if (iAllocRef)
            iAllocRef->addRef();
        iAlloc = alloc;
    }
    else
    {//use a basic allocator that resides in this object.
        iAlloc = &iDefAlloc;
        iAllocRef = NULL;
    }
}


// **************************************************************
OSCL_EXPORT_REF OSCL_HeapStringA::OSCL_HeapStringA()
{
    create(NULL, NULL);
    set_rep(NULL);
}
OSCL_EXPORT_REF OSCL_HeapStringA::OSCL_HeapStringA(Oscl_DefAlloc *alloc, OsclRefCounter *ref)
{
    create(alloc, ref);
    set_rep(NULL);
}

OSCL_EXPORT_REF OSCL_wHeapStringA::OSCL_wHeapStringA()
{
    create(NULL, NULL);
    set_rep(NULL);
}
OSCL_EXPORT_REF OSCL_wHeapStringA::OSCL_wHeapStringA(Oscl_DefAlloc *alloc, OsclRefCounter *ref)
{
    create(alloc, ref);
    set_rep(NULL);
}

// **************************************************************
OSCL_EXPORT_REF OSCL_HeapStringA::OSCL_HeapStringA(const chartype* cp, Oscl_DefAlloc *alloc, OsclRefCounter *ref)
{
    create(alloc, ref);
    set_rep(cp);
}

OSCL_EXPORT_REF OSCL_wHeapStringA::OSCL_wHeapStringA(const chartype* cp, Oscl_DefAlloc *alloc, OsclRefCounter *ref)
{
    create(alloc, ref);
    set_rep(cp);
}

// **************************************************************
OSCL_EXPORT_REF void OSCL_HeapStringA::set(const chartype* cp, uint32 length)
{
    set_rep(cp, length);
    //just in case input string is shorter than 'length'
    iRep->size = oscl_strlen(get_cstr());
}

OSCL_EXPORT_REF void OSCL_wHeapStringA::set(const chartype* cp, uint32 length)
{
    set_rep(cp, length);
    //just in case input string is shorter than 'length'
    iRep->size = oscl_strlen(get_cstr());
}


// **************************************************************
OSCL_EXPORT_REF OSCL_HeapStringA::OSCL_HeapStringA(const chartype* cp, uint32 length, Oscl_DefAlloc *alloc, OsclRefCounter *ref)
{
    create(alloc, ref);
    set(cp, length);
}

OSCL_EXPORT_REF OSCL_wHeapStringA::OSCL_wHeapStringA(const chartype* cp, uint32 length, Oscl_DefAlloc *alloc, OsclRefCounter *ref)
{
    create(alloc, ref);
    set(cp, length);
}

// **************************************************************
OSCL_EXPORT_REF OSCL_HeapStringA::OSCL_HeapStringA(const OSCL_HeapStringA& src) : OSCL_String(src)
{
    create(NULL, NULL);
    *this = src;//uses heapstring=heapstring overload.
}
OSCL_EXPORT_REF OSCL_HeapStringA::OSCL_HeapStringA(const OSCL_HeapStringA& src, Oscl_DefAlloc *alloc, OsclRefCounter *ref)
{
    create(alloc, ref);
    *this = src;//uses heapstring=heapstring overload.
}

OSCL_EXPORT_REF OSCL_wHeapStringA::OSCL_wHeapStringA(const OSCL_wHeapStringA& src) : OSCL_wString(src)
{
    create(NULL, NULL);
    *this = src;//uses heapstring=heapstring overload.
}
OSCL_EXPORT_REF OSCL_wHeapStringA::OSCL_wHeapStringA(const OSCL_wHeapStringA& src , Oscl_DefAlloc *alloc, OsclRefCounter *ref)
{
    create(alloc, ref);
    *this = src;//uses heapstring=heapstring overload.
}

// **************************************************************
OSCL_EXPORT_REF OSCL_HeapStringA::OSCL_HeapStringA(const OSCL_String& src, Oscl_DefAlloc *alloc, OsclRefCounter *ref)
{
    create(alloc, ref);
    set_rep(src);
}

OSCL_EXPORT_REF OSCL_wHeapStringA::OSCL_wHeapStringA(const OSCL_wString& src , Oscl_DefAlloc *alloc, OsclRefCounter *ref)
{
    create(alloc, ref);
    set_rep(src);
}

// **************************************************************
OSCL_EXPORT_REF OSCL_HeapStringA::~OSCL_HeapStringA()
{
    //remove ref to string rep
    if (iRep)
        iRep->remove_ref(*iAlloc);
    //remove ref to allocator.
    if (iAllocRef)
        iAllocRef->removeRef();
}

OSCL_EXPORT_REF OSCL_wHeapStringA::~OSCL_wHeapStringA()
{
    //remove ref to string rep
    if (iRep)
        iRep->remove_ref(*iAlloc);
    //remove ref to allocator.
    if (iAllocRef)
        iAllocRef->removeRef();
}

// **************************************************************
OSCL_EXPORT_REF OSCL_HeapStringA& OSCL_HeapStringA::operator=(const OSCL_HeapStringA & src)
{
    //the allocators must match in order to re-use
    //the heap string representation.
    //Allocators match if they're the same object,
    //or they're both the default allocators.
    if (src.iRep
            && ((src.iAlloc == (Oscl_DefAlloc*)&src.iDefAlloc
                 && iAlloc == &iDefAlloc)
                || src.iAlloc == iAlloc))
    {
        CHeapRep::assign(iRep, src.iRep, *iAlloc);
    }
    else
    {//otherwise, create a new representation
        set_rep(src);
    }
    return (*this);
}

OSCL_EXPORT_REF OSCL_wHeapStringA& OSCL_wHeapStringA::operator=(const OSCL_wHeapStringA & src)
{
    //the allocators must match in order to re-use
    //the heap string representation.
    //Allocators match if they're the same object,
    //or they're both the default allocators.
    if (src.iRep
            && ((src.iAlloc == (Oscl_DefAlloc*)&src.iDefAlloc
                 && iAlloc == &iDefAlloc)
                || src.iAlloc == iAlloc))
    {
        CHeapRep::assign(iRep, src.iRep, *iAlloc);
    }
    else
    {//otherwise, create a new representation
        set_rep(src);
    }
    return (*this);
}

// **************************************************************
OSCL_EXPORT_REF OSCL_HeapStringA& OSCL_HeapStringA::operator=(const chartype * cp)
{
    set_rep(cp);
    return (*this);
}

OSCL_EXPORT_REF OSCL_wHeapStringA& OSCL_wHeapStringA::operator=(const chartype * cp)
{
    set_rep(cp);
    return (*this);
}

// **************************************************************
//				   CStackRep Implementation
// **************************************************************

OSCL_EXPORT_REF void CStackRep::set(const char* cp, uint32 len)
{
    if (len > maxsize)
        size = maxsize;//truncate
    else
        size = len;
    if (cp)
        oscl_strncpy((char*)buffer, cp, size);
    ((char*)buffer)[size] = '\0';
}
OSCL_EXPORT_REF void CStackRep::set(const oscl_wchar* cp, uint32 len)
{
    if (len > maxsize)
        size = maxsize;//truncate
    else
        size = len;
    if (cp)
        oscl_strncpy((oscl_wchar*)buffer, cp, size);
    ((oscl_wchar*)buffer)[size] = '\0';
}

// **************************************************************
OSCL_EXPORT_REF void CStackRep::append(const char* cp, uint32 alen)
{
    uint32 len = alen;
    if (size + len > maxsize)
        len = maxsize - size;//truncate
    if (len > 0)
    {
        size = size + len;
        oscl_strncat((char*)buffer, cp, len);
        ((char*)buffer)[size] = '\0';
    }
}
OSCL_EXPORT_REF void CStackRep::append(const oscl_wchar* cp, uint32 alen)
{
    uint32 len = alen;
    if (size + len > maxsize)
        len = maxsize - size;//truncate
    if (len > 0)
    {
        size = size + len;
        oscl_strncat((oscl_wchar*)buffer, cp, len);
        ((oscl_wchar*)buffer)[size] = '\0';
    }
}

// **************************************************************
//				  CFastRep Implementation
// **************************************************************

OSCL_EXPORT_REF void CFastRep::set_r(const char* cp, uint32 len)
{
    size = len;
    maxsize = len;
    buffer = (OsclAny*)cp;
    writable = false;
}
OSCL_EXPORT_REF void CFastRep::set_r(const oscl_wchar* cp, uint32 len)
{
    size = len;
    maxsize = len;
    buffer = (OsclAny*)cp;
    writable = false;
}

// **************************************************************
OSCL_EXPORT_REF void CFastRep::set_w(char* cp, uint32 len, uint32 maxlen)
{
    size = len;
    maxsize = maxlen;
    buffer = (OsclAny*)cp;
    writable = true;
}
OSCL_EXPORT_REF void CFastRep::set_w(oscl_wchar* cp, uint32 len, uint32 maxlen)
{
    size = len;
    maxsize = maxlen;
    buffer = (OsclAny*)cp;
    writable = true;
}

// **************************************************************
OSCL_EXPORT_REF void CFastRep::append(const char* cp, uint32 len)
{
    uint32 ncopy = len;
    if (size + len > maxsize)
        ncopy = maxsize - size;//truncate
    if (ncopy > 0)
    {
        oscl_strncat((char*)buffer, cp, ncopy);
        size += ncopy;
        if (!writable)
            maxsize = size;
    }
}
OSCL_EXPORT_REF void CFastRep::append(const oscl_wchar* cp, uint32 len)
{
    uint32 ncopy = len;
    if (size + len > maxsize)
        ncopy = maxsize - size;//truncate
    if (ncopy > 0)
    {
        oscl_strncat((oscl_wchar*)buffer, cp, ncopy);
        size += ncopy;
        if (!writable)
            maxsize = size;
    }
}

// **************************************************************
//					 OSCL_FastString Implementation
//					 OSCL_wFastString Implementation
// **************************************************************
void OSCL_FastString::set_rep(const chartype* cp)
{
    uint32 len = (cp) ? oscl_strlen(cp) : 0;
    rep.set_r(cp, len);
}

void OSCL_wFastString::set_rep(const chartype* cp)
{
    uint32 len = (cp) ? oscl_strlen(cp) : 0;
    rep.set_r(cp, len);
}

// **************************************************************
void OSCL_FastString::append_rep(const chartype* cp)
{
    uint32 len = (cp) ? oscl_strlen(cp) : 0;
    rep.append(cp, len);
}

void OSCL_wFastString::append_rep(const chartype* cp)
{
    uint32 len = (cp) ? oscl_strlen(cp) : 0;
    rep.append(cp, len);
}


// **************************************************************
void OSCL_FastString::set_rep(const OSCL_String& src)
{
    //inherit the writable-ness of the source string.
    if (src.get_str())
        rep.set_w(src.get_str(), src.get_size(), src.get_maxsize());
    else
        rep.set_r(src.get_cstr(), src.get_size());
}

void OSCL_wFastString::set_rep(const OSCL_wString& src)
{
    //inherit the writable-ness of the source string.
    if (src.get_str())
        rep.set_w(src.get_str(), src.get_size(), src.get_maxsize());
    else
        rep.set_r(src.get_cstr(), src.get_size());
}

// **************************************************************
void OSCL_FastString::append_rep(const OSCL_String& src)
{
    rep.append(src.get_cstr(), src.get_size());
}

void OSCL_wFastString::append_rep(const OSCL_wString& src)
{
    rep.append(src.get_cstr(), src.get_size());
}

// **************************************************************
void OSCL_FastString::set_len(uint32 len)
{
    rep.size = len;
}
void OSCL_wFastString::set_len(uint32 len)
{
    rep.size = len;
}

// **************************************************************
OSCL_EXPORT_REF OSCL_FastString::OSCL_FastString()
{
    set_rep(NULL);
}

OSCL_EXPORT_REF OSCL_wFastString::OSCL_wFastString()
{
    set_rep(NULL);
}

// **************************************************************
OSCL_EXPORT_REF OSCL_FastString::OSCL_FastString(const chartype* cp)
{
    set_rep(cp);
}

OSCL_EXPORT_REF OSCL_wFastString::OSCL_wFastString(const chartype* cp)
{
    set_rep(cp);
}

// **************************************************************
OSCL_EXPORT_REF void OSCL_FastString::set(chartype* cp, uint32 maxlen)
{
    //set string to new writable buffer.
    //make sure buffer is null-terminated
    for (uint32 i = 0;i <= maxlen;i++)
    {
        if (cp[i] == '\0')
        {
            rep.set_w(cp, i, maxlen);
            return;
        }
    }
    OsclError::Leave(OsclErrGeneral);//not null-terminated
}

OSCL_EXPORT_REF void OSCL_wFastString::set(chartype* cp, uint32 maxlen)
{
    //set string to new writable buffer.
    //make sure buffer is null-terminated
    for (uint32 i = 0;i <= maxlen;i++)
    {
        if (cp[i] == '\0')
        {
            rep.set_w(cp, i, maxlen);
            return;
        }
    }
    OsclError::Leave(OsclErrGeneral);//not null-terminated
}

// **************************************************************
OSCL_EXPORT_REF OSCL_FastString::OSCL_FastString(chartype* cp, uint32 maxlen)
{
    set(cp, maxlen);
}

OSCL_EXPORT_REF OSCL_wFastString::OSCL_wFastString(chartype* cp, uint32 maxlen)
{
    set(cp, maxlen);
}

// **************************************************************
OSCL_EXPORT_REF OSCL_FastString::OSCL_FastString(const OSCL_FastString& src) : OSCL_String(src)
{
    set_rep(src);
}

OSCL_EXPORT_REF OSCL_wFastString::OSCL_wFastString(const OSCL_wFastString& src) : OSCL_wString(src)
{
    set_rep(src);
}

// **************************************************************
OSCL_EXPORT_REF OSCL_FastString::~OSCL_FastString()
{
}

OSCL_EXPORT_REF OSCL_wFastString::~OSCL_wFastString()
{
}

// **************************************************************
OSCL_EXPORT_REF void OSCL_FastString::set_length()
{
    rep.size = oscl_strlen(get_cstr());
    //for read-only string, the maxsize tracks the size.
    if (!rep.writable)
        rep.maxsize = rep.size;
}

OSCL_EXPORT_REF void OSCL_wFastString::set_length()
{
    rep.size = oscl_strlen(get_cstr());
    //for read-only string, the maxsize tracks the size.
    if (!rep.writable)
        rep.maxsize = rep.size;
}

// **************************************************************
OSCL_EXPORT_REF OSCL_FastString& OSCL_FastString::operator=(const OSCL_FastString & src)
{
    set_rep(src);
    return (*this);
}

OSCL_EXPORT_REF OSCL_wFastString& OSCL_wFastString::operator=(const OSCL_wFastString & src)
{
    set_rep(src);
    return (*this);
}


// **************************************************************
OSCL_EXPORT_REF OSCL_FastString& OSCL_FastString::operator=(const chartype * cp)
{
    set_rep(cp);
    return (*this);
}

OSCL_EXPORT_REF OSCL_wFastString& OSCL_wFastString::operator=(const chartype * cp)
{
    set_rep(cp);
    return (*this);
}


// **************************************************************
OSCL_EXPORT_REF uint32 OSCL_FastString::get_size() const
{
    return rep.size;
}

OSCL_EXPORT_REF uint32 OSCL_wFastString::get_size() const
{
    return rep.size;
}

// **************************************************************
OSCL_EXPORT_REF uint32 OSCL_FastString::get_maxsize() const
{
    return rep.maxsize;
}

OSCL_EXPORT_REF uint32 OSCL_wFastString::get_maxsize() const
{
    return rep.maxsize;
}

// **************************************************************
OSCL_EXPORT_REF const OSCL_FastString::chartype* OSCL_FastString::get_cstr() const
{
    return (chartype*)rep.buffer;
}

OSCL_EXPORT_REF const OSCL_wFastString::chartype* OSCL_wFastString::get_cstr() const
{
    return (chartype*)rep.buffer;
}

// **************************************************************
OSCL_EXPORT_REF OSCL_FastString::chartype* OSCL_FastString::get_str() const
{
    if (rep.writable)
        return (chartype*)rep.buffer;
    return NULL;
}

OSCL_EXPORT_REF OSCL_wFastString::chartype* OSCL_wFastString::get_str() const
{
    if (rep.writable)
        return (chartype*)rep.buffer;
    return NULL;
}





