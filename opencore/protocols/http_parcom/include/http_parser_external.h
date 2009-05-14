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
#ifndef HTTP_PARSER_EXTERNAL_H_
#define HTTP_PARSER_EXTERNAL_H_

#include "oscl_refcounter_memfrag.h"
#include "oscl_vector.h"

// content info exposed to user
struct HTTPContentInfo
{
    uint32 iContentLength;		// for "Content-Length"
    uint32 iContentRangeLeft;	// for "Content-Range"
    uint32 iContentRangeRight;

    // constructor
    HTTPContentInfo()
    {
        clear();
    }
    void clear()
    {
        oscl_memset(this, 0, sizeof(HTTPContentInfo));
    }
};

///////////////////////////////////////////////////////////////////////////////////////
// The following structure is an array of ref-counted memory fragments
typedef Oscl_TAlloc<OsclRefCounterMemFrag, OsclMemAllocator> HTTPEntityUnit_Alloc;

class HTTPEntityUnit
{
    public:
        Oscl_Vector<OsclRefCounterMemFrag, HTTPEntityUnit_Alloc> iFragments;

    public:
        // default constructor
        HTTPEntityUnit()
        {
            clear();
        }

        // copy constructor
        HTTPEntityUnit(const HTTPEntityUnit &x) : iFragments(x.iFragments)
        {
            ;
        }

        // destructor
        ~HTTPEntityUnit()
        {
            clear();
        }

        // operator "="
        HTTPEntityUnit& operator=(const HTTPEntityUnit &x)
        {
            iFragments = x.iFragments;
            return *this;
        }

        // get number of fragments
        uint32 getNumFragments()
        {
            return iFragments.size();
        }

        // get memory fragment
        bool getMemFrag(uint32 index, OsclRefCounterMemFrag& memfrag)
        {
            if (index >= iFragments.size()) return false;

            memfrag = OsclRefCounterMemFrag(iFragments[index]);
            return true;
        }

        // add a memory fragment
        bool addMemFrag(const OsclRefCounterMemFrag& memfrag)
        {
            int err = 0;
            OSCL_TRY(err, iFragments.push_back(memfrag);)
            return (err == 0);
        }

        // get total size of all the available memory fragments
        uint32 getTotalFragSize()
        {
            uint32 total_size = 0, i;
            for (i = 0; i < iFragments.size(); i++)
            {
                total_size += (iFragments[i].getMemFrag()).len;
            }
            return total_size;
        }

        // clear memory fragments
        void clear()
        {
            iFragments.clear();
        }

        // empty
        bool empty()
        {
            return iFragments.empty();
        }
};

///////////////////////////////////////////////////////////////////////////////////////
// The following structure is a ref-counted array of ref-counted memory fragments.
class RefCountHTTPEntityUnit
{
        HTTPEntityUnit iEntityUnit;
        OsclRefCounter *iRefcnt;

    public:
        // default constructor
        RefCountHTTPEntityUnit() : iRefcnt(NULL)
        {
            ;
        }

        // another constructor
        RefCountHTTPEntityUnit(HTTPEntityUnit &aEntityUnit, OsclRefCounter *aRefcnt) :
                iEntityUnit(aEntityUnit), iRefcnt(aRefcnt)
        {
            if (iRefcnt) iRefcnt->addRef();
        }

        // copy constructor
        RefCountHTTPEntityUnit(const RefCountHTTPEntityUnit &rhs) :
                iEntityUnit(rhs.iEntityUnit), iRefcnt(rhs.iRefcnt)
        {
            if (iRefcnt) iRefcnt->addRef();
        }

        // destructor
        ~RefCountHTTPEntityUnit()
        {
            if (iRefcnt) iRefcnt->removeRef();
        }

        // operator "="
        RefCountHTTPEntityUnit& operator=(const RefCountHTTPEntityUnit& rhs)
        {
            if (this == &rhs)
            {
                return *this;    // protect against self-assignment
            }

            bind(rhs.iEntityUnit, rhs.iRefcnt);
            return *this;
        }

        // bind
        void bind(const HTTPEntityUnit &aEntityUnit, OsclRefCounter *aRefcnt)
        {
            // remove ref for current entity unit
            if (iRefcnt) iRefcnt->removeRef();

            // copy assigned object
            iEntityUnit = aEntityUnit;
            iRefcnt = aRefcnt;

            // add ref for new entity unit
            if (iRefcnt) iRefcnt->addRef();
        }

        // Returns a pointer to the contained reference counter object
        OsclRefCounter* getRefCounter() const
        {
            return iRefcnt;
        }

        // return the entity unit
        HTTPEntityUnit& getEntityUnit()
        {
            return iEntityUnit;
        }

        // empty
        bool empty()
        {
            return (iEntityUnit.empty() && iRefcnt == NULL);
        }
        // clear
        void clear()
        {
            iEntityUnit.clear();
            iRefcnt = NULL;
        }

};

#endif // HTTP_PARSER_EXTERNAL_H_

