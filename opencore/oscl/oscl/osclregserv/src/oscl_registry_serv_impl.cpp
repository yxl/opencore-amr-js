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

#include "oscl_registry_serv_impl.h"

#include "osclconfig_proc.h"

#include "oscl_registry_types.h"
/*!
** OS-independent server-side implementation.  This part works
** for either global var or symbian server implementation.
*/

OsclComponentRegistryElement::OsclComponentRegistryElement(OSCL_String& compid, OsclComponentFactory fac)
        : iId(NULL)
        , iFactory(fac)
{
    iId = OSCL_NEW(OSCL_HeapString<OsclMemAllocator>, (compid));
}

OsclComponentRegistryElement::~OsclComponentRegistryElement()
{
    if (iId)
    {
        OSCL_HeapString<OsclMemAllocator>* id = (OSCL_HeapString<OsclMemAllocator>*)iId;
        OSCL_DELETE(id);
    }
}

OsclComponentRegistryElement::OsclComponentRegistryElement(const OsclComponentRegistryElement&v)
        : iId(NULL)
        , iFactory(v.iFactory)
        , iComponentId(v.iComponentId)
{
    iId = OSCL_NEW(OSCL_HeapString<OsclMemAllocator>, (*v.iId));
}

OsclComponentRegistryElement& OsclComponentRegistryElement::operator=(const OsclComponentRegistryElement & src)
{
    if (iId)
    {
        OSCL_HeapString<OsclMemAllocator>* id = (OSCL_HeapString<OsclMemAllocator>*)iId;
        OSCL_DELETE(id);
        iId = NULL;
    }
    iId = OSCL_NEW(OSCL_HeapString<OsclMemAllocator>, (*src.iId));
    iFactory = src.iFactory;
    iComponentId = src.iComponentId;
    return *this;
}

bool OsclComponentRegistryElement::Match(OSCL_String& id, bool aExact)
{
    if (aExact)
    {
        if (id == *iId)
            return true;
    }
    else
    {
        //hierarchical string match.
        //tells whether "id" is a prefix of the component ID (or an exact match).
        if (iId->get_size() >= id.get_size()
                && oscl_CIstrncmp(id.get_cstr(), iId->get_cstr(), id.get_size()) == 0
                && (iId->get_cstr()[id.get_size()] == '/'
                    || iId->get_cstr()[id.get_size()] == '\0'))
        {
            return true;
        }
    }
    return false;
}


OsclComponentRegistryElement* OsclComponentRegistryData::Find(OSCL_String& id, bool aExact)
{
    for (uint32 i = 0;i < iVec.size();i++)
    {
        if (iVec[i].Match(id, aExact))
            return &iVec[i];
    }
    return NULL;
}

OsclComponentRegistry::OsclComponentRegistry()
{
    iMutex.Create();
    iComponentIdCounter = 0;
    iNumSessions = 0;
}

OsclComponentRegistry::~OsclComponentRegistry()
{
    //need to be careful not to do any cleanup that isn't valid
    //for a global variable destructor here, such as anything that
    //relies on memory manager.
    iMutex.Close();
}

void OsclComponentRegistry::OpenSession()
{
    iMutex.Lock();
    iNumSessions++;
    iMutex.Unlock();
}

void OsclComponentRegistry::CloseSession()
{
    //when the number of sessions is zero, cleanup all dynamically allocated
    //data.
    //This is necessary to avoid trying to use memory manager after it's
    //cleaned up, in the global variable implementation.
    iMutex.Lock();
    iNumSessions--;
    if (iNumSessions <= 0)
    {
        iData.iVec.clear();
        iData.iVec.destroy();
    }
    iMutex.Unlock();
}

int32 OsclComponentRegistry::Register(uint32&aId, OSCL_String&compId, OsclComponentFactory fac)
{
    int32 err = OsclErrNone;;
    iMutex.Lock();
    //try to find existing component (use exact match).
    if (iData.Find(compId, true))
    {
        err = OsclErrAlreadyExists;
    }
    else
    {
        //create new component.
        OsclComponentRegistryElement elem(compId, fac);
        elem.iComponentId = ++iComponentIdCounter;
        aId = elem.iComponentId;
        OSCL_TRY(err, iData.iVec.push_back(elem););
    }
    iMutex.Unlock();
    return err;
}

OsclComponentFactory OsclComponentRegistry::FindExact(OSCL_String& compId)
{
    OsclComponentRegistryElement*comp = NULL;
    OsclComponentFactory fac = NULL;

    iMutex.Lock();
    comp = iData.Find(compId, true);//exact
    if (comp)
        fac = comp->iFactory;
    iMutex.Unlock();

    return fac;
}

void OsclComponentRegistry::FindHierarchical(OSCL_String& aReg, Oscl_Vector<OsclRegistryAccessElement, OsclMemAllocator>& aVec)
{
    iMutex.Lock();

    //compare target string against all components, using
    //heirarchical match.

    //note: the push_back call may leave.  if so, just exit
    //gracefully.
    int32 err;
    OSCL_TRY(err,
             for (uint32 i = 0;i < iData.iVec.size();i++)
{
    if (iData.iVec[i].Match(aReg, false))//hierarchical match.
        {
            OsclRegistryAccessElement element;
            element.iFactory = iData.iVec[i].iFactory;
            element.iMimeString = *(iData.iVec[i].iId);
            aVec.push_back(element);
        }
    }
            );

    iMutex.Unlock();
}

int32 OsclComponentRegistry::Unregister(OSCL_String& compId)
{
    OsclComponentRegistryElement*comp = NULL;

    iMutex.Lock();
    comp = iData.Find(compId, true);
    if (!comp)
    {
        iMutex.Unlock();
        return OsclErrArgument;//non existent.
    }
    iData.iVec.erase(comp);
    //also cleanup reserve
    if (iData.iVec.empty())
        iData.iVec.destroy();
    iMutex.Unlock();
    return 0;
}

int32 OsclComponentRegistry::Unregister(uint32 aId)
{
    //no choice but to search all registries...

    iMutex.Lock();
    for (uint32 i = 0;i < iData.iVec.size();i++)
    {
        if (iData.iVec[i].iComponentId == aId)
        {
            iData.iVec.erase(&iData.iVec[i]);
            //also cleanup reserve
            if (iData.iVec.empty())
                iData.iVec.destroy();
            break;
        }
    }
    iMutex.Unlock();
    return 0;
}










