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
#ifndef HTTP_PARCOM_INTERNAL_H_
#define HTTP_PARCOM_INTERNAL_H_

#ifndef OSCL_REFCOUNTER_MEMFRAG_H_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif


///////////////////////////////////////////////////////////////////////////////////////
const char HTTP_CHAR_CR 	 = 13;
const char HTTP_CHAR_LF 	 = 10;
const char HTTP_CHAR_NULL 	 = 0;
const char HTTP_CHAR_COLON	 = ':';
const char HTTP_CHAR_DOLLAR  = '$';
const char HTTP_CHAR_SPACE   = ' ';
const char HTTP_CHAR_SLASH   = '/';
const char HTTP_CHAR_SEMICOLON = ';';
const char HTTP_CHAR_STAR    = '*';
const char HTTP_CHAR_PLUS    = '+';
const char HTTP_CHAR_MINUS   = '-';
const char HTTP_CHAR_DOT     = '.';
const char HTTP_CHAR_TAB     = '\t';
const char HTTP_CHAR_COMMA   = ',';
const char HTTP_CHAR_EQUAL   = '=';


//An array of strings corresponding to HTTPMethod enum
static const char* const HTTPMethodString[] =
{
    "GET",		//METHOD_GET
    "HEAD",		//METHOD_HEAD
    "POST",		//METHOD_POST
    "DELETE",	//METHOD_DELETE
    "LINK",		//METHOD_LINK
    "UNLINK",	//METHOD_UNLINK
    "OPTIONS",	//METHOD_OPTIONS
    "PUT",		//METHOD_PUT
    "TRACE",	//METHOD_TRACE
    "CONNECT",	//METHOD_CONNECT
};

// The following structure is an extension of OsclMemoryFragment with a differentation between memory usage and memory capacity
struct HTTPMemoryFragment
{
    void *iPtr;
    uint32 iLen; // refer to the memory usage, initially (no memory use), len=0
    uint32 iCapacity;

    // constructor
    HTTPMemoryFragment()
    {
        clear();
    }

    // copy constructor
    HTTPMemoryFragment(const void *aPtr, const uint32 aCapacity) : iPtr((void *)aPtr), iLen(0), iCapacity(aCapacity)
    {
        ;
    }

    // copy constructor
    HTTPMemoryFragment(const OsclMemoryFragment &x)
    {
        iPtr = x.ptr;
        iLen = 0;
        iCapacity = x.len;
    }

    // operator "="
    HTTPMemoryFragment& operator=(const HTTPMemoryFragment& rhs)
    {
        iPtr = rhs.iPtr;
        iLen = rhs.iLen;
        iCapacity = rhs.iCapacity;
        return *this;
    }

    // operator "="
    HTTPMemoryFragment& operator=(const OsclMemoryFragment& rhs)
    {
        iPtr = rhs.ptr;
        iLen = 0;
        iCapacity = rhs.len;
        return *this;
    }

    // destructor
    ~HTTPMemoryFragment()
    {
        clear();
    }

    // clear
    void clear()
    {
        oscl_memset(this, 0, sizeof(HTTPMemoryFragment));
    }

    // bind
    void bind(const void *aPtr, const uint32 aCapacity)
    {
        iPtr = (void *)aPtr;
        iLen = 0;
        iCapacity = aCapacity;
    }

    void bind(const OsclMemoryFragment &x)
    {
        iPtr = x.ptr;
        iLen = 0;
        iCapacity = x.len;
    }

    // empty
    bool empty()
    {
        return ((uint8 *)iPtr == NULL || (iCapacity == 0));
    }

    // get functions
    void *getPtr()
    {
        return (void *)((uint8 *)iPtr + iLen);
    }
    uint32 getLen() const
    {
        return iLen;
    }
    uint32 getCapacity()
    {
        return iCapacity;
    }
    uint32 getAvailableSpace()
    {
        return iCapacity -iLen;
    }

    // update length
    bool update(const uint32 aLength)
    {
        if (isSpaceEnough(aLength))
        {
            iLen += aLength;
            return true;
        }
        return false;
    }

    bool update(const void *aPtr)
    {
        int32 aLen = (uint8*)aPtr - (uint8*)iPtr;
        if (aLen < 0 || (uint32)aLen > iCapacity) return false;
        iLen = aLen;
        return true;
    }

    // is remaining space enough for the new memory usage
    bool isSpaceEnough(uint32 aNewUsage)
    {
        return (iLen + aNewUsage <= iCapacity);
    }
};

// wrap OsclMemoryFragment and add more functionalities
struct OsclMemoryFragWrapper
{
    OsclMemoryFragment iMemFrag;

    // constructor
    OsclMemoryFragWrapper()
    {
        clear();
    }

    // copy constructers
    OsclMemoryFragWrapper(const OsclMemoryFragWrapper &rhs)
    {
        iMemFrag.ptr = rhs.iMemFrag.ptr;
        iMemFrag.len = rhs.iMemFrag.len;
    }
    OsclMemoryFragWrapper(const OsclMemoryFragment &rhs)
    {
        iMemFrag.ptr = rhs.ptr;
        iMemFrag.len = rhs.len;
    }
    OsclMemoryFragWrapper(const void *aPtr, const uint32 aLen)
    {
        bind(aPtr, aLen);
    }

    // assignment Operator
    OsclMemoryFragWrapper& operator= (const OsclMemoryFragment &x)
    {
        bind(x);
        return *this;
    }

    OsclMemoryFragWrapper& operator= (OsclMemoryFragWrapper &x)
    {
        bind((const OsclMemoryFragment &)(x.get()));
        return *this;
    }

    // clear
    void clear()
    {
        iMemFrag.ptr = NULL;
        iMemFrag.len = 0;
    }

    // empty
    bool empty()
    {
        return (((uint8*)iMemFrag.ptr == NULL) || (iMemFrag.len == 0));
    }

    // bind
    void bind(const void *aPtr, const uint32 aLen)
    {
        iMemFrag.ptr = (void *)aPtr;
        iMemFrag.len = aLen;
    }
    void bind(const OsclMemoryFragment &aFrag)
    {
        iMemFrag.ptr = aFrag.ptr;
        iMemFrag.len = aFrag.len;
    }

    // update
    void update(const uint32 aLen)
    {
        iMemFrag.len = aLen;
    }
    void update(const void *aPtr)
    {
        iMemFrag.ptr = (void *)aPtr;
    }

    // get
    OsclMemoryFragment &get()
    {
        return iMemFrag;
    }
    void *getPtr()
    {
        return iMemFrag.ptr;
    }
    uint32 getLen()
    {
        return iMemFrag.len;
    }
};

// This class is based on OsclRefCounterMemFrag, but with the following change,
// memory fragment and its associated with reference counter can be updated any time
class RefCounterMemoryFragment
{
    public:

        // constructor
        RefCounterMemoryFragment(OsclMemoryFragment &m, OsclRefCounter *r) :
                iMemfrag(m), iRefcnt(r)
        {
            if (iRefcnt) iRefcnt->addRef();
        }

        RefCounterMemoryFragment(const void *aPtr, const uint32 aLen, OsclRefCounter *r) :
                iMemfrag(aPtr, aLen), iRefcnt(r)
        {
            if (iRefcnt) iRefcnt->addRef();
        }

        // Copy constructor.
        RefCounterMemoryFragment(const OsclRefCounterMemFrag &x) :
                iMemfrag(((OsclRefCounterMemFrag &)x).getMemFrag()),
                iRefcnt(((OsclRefCounterMemFrag &)x).getRefCounter())
        {
            if (iRefcnt) iRefcnt->addRef();
        }

        RefCounterMemoryFragment(const RefCounterMemoryFragment &x) :
                iMemfrag(x.iMemfrag), iRefcnt(x.iRefcnt)
        {
            if (iRefcnt) iRefcnt->addRef();
        }

        // Default constructor.
        RefCounterMemoryFragment()
        {
            iMemfrag.clear();
            iRefcnt = 0;
        }

        // Assignment Operator
        RefCounterMemoryFragment& operator= (const RefCounterMemoryFragment &x)
        {
            if (this == &x) return *this; // protect against self-assignment
            bind((RefCounterMemoryFragment &)x);
            return *this;
        }

        // Destructor
        ~RefCounterMemoryFragment()
        {
            if (iRefcnt) iRefcnt->removeRef();
        }

        bool empty()
        {
            return (iMemfrag.empty() && iRefcnt == NULL);
        }

        // get functions
        // Returns a pointer to the contained reference counter object
        OsclRefCounter* getRefCounter()
        {
            return iRefcnt;
        }
        // Returns a reference to the contained memory fragment
        OsclMemoryFragment& getMemFrag()
        {
            return iMemfrag.get();
        }
        // Returns a pointer to the memory fragment data.
        OsclAny* getMemFragPtr()
        {
            return iMemfrag.get().ptr;
        }
        // Returns the size of the memory fragment data which equals its filled size.
        uint32 getMemFragSize()
        {
            return iMemfrag.get().len;
        }
        // Returns the reference counter's current count.
        uint32 getCount()
        {
            return (iRefcnt) ? iRefcnt->getCount() : 0;
        }
        void getRefCountMemFrag(OsclRefCounterMemFrag &aFrag)
        {
            aFrag = OsclRefCounterMemFrag(iMemfrag.get(), iRefcnt, iMemfrag.getLen());
            iRefcnt->addRef(); // have to add ref-count manually
        }

        // set functions
        void bind(OsclMemoryFragment &m, OsclRefCounter *r)
        {
            if (iRefcnt) iRefcnt->removeRef(); // remove ref for current memfrag
            iMemfrag = m;
            iRefcnt = r;
            if (iRefcnt) iRefcnt->addRef(); // add ref for new memfrag
        }
        void bind(OsclRefCounterMemFrag &x)
        {
            if (iRefcnt) iRefcnt->removeRef(); // remove ref for current memfrag
            // copy assigned object
            iMemfrag = x.getMemFrag();
            iRefcnt = x.getRefCounter();
            if (iRefcnt) iRefcnt->addRef(); // add ref for new memfrag
        }
        void bind(RefCounterMemoryFragment &x)
        {
            if (iRefcnt) iRefcnt->removeRef(); // remove ref for current memfrag
            // copy assigned object
            iMemfrag = x.iMemfrag;
            iRefcnt = x.iRefcnt;
            if (iRefcnt) iRefcnt->addRef(); // add ref for new memfrag
        }

        // update memory fragment with the same reference counter
        void update(OsclMemoryFragment &m)
        {
            iMemfrag.bind(m);
        }
        void update(void *aPtr)
        {
            iMemfrag.update(aPtr);
        }
        void update(uint32 aLen)
        {
            iMemfrag.update(aLen);
        }

    private:
        OsclMemoryFragWrapper iMemfrag;
        OsclRefCounter *iRefcnt;
};


#endif // HTTP_PARCOM_INTERNAL_H_

