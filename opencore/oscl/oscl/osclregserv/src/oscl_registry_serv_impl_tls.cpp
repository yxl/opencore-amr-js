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

#include "oscl_registry_serv_impl_tls.h"

#include "osclconfig_proc.h"

#include "oscl_error.h"
#include "oscl_registry_serv_impl.h"

OsclComponentRegistry* OsclRegistryServTlsImpl::GetOsclComponentRegistry()
{
    return (OsclComponentRegistry*)OsclTLSRegistryEx::getInstance(OSCL_TLS_ID_OSCLREGISTRY);
}

OsclRegistryServTlsImpl::OsclRegistryServTlsImpl()
{
    iIsOpen = false;
}

OsclRegistryServTlsImpl::~OsclRegistryServTlsImpl()
{
    if (iIsOpen)
        Close();
}

int32 OsclRegistryServTlsImpl::Connect()
{
    if (iIsOpen)
        return OsclErrInvalidState;
    //Create the registry on the first session.
    if (!GetOsclComponentRegistry())
    {
        OsclComponentRegistry* reg = NULL;
        int32 err;
        OSCL_TRY(err, reg = OSCL_NEW(OsclComponentRegistry, ()));
        if (err != OsclErrNone)
            return err;

        OsclTLSRegistryEx::registerInstance(reg, OSCL_TLS_ID_OSCLREGISTRY);
    }
    iIsOpen = true;
    GetOsclComponentRegistry()->OpenSession();
    return OsclErrNone;
}

void OsclRegistryServTlsImpl::Close()
{
    //session cleanup
    if (!iIdVec.empty())
    {
        //unregister all comps that were registered by this session
        for (uint32 i = 0;i < iIdVec.size();i++)
            GetOsclComponentRegistry()->Unregister(iIdVec[i]);
        //clear our comp list.
        iIdVec.destroy();
    }

    if (iIsOpen)
    {
        GetOsclComponentRegistry()->CloseSession();
        iIsOpen = false;
        //Delete the registry when the session count goes to zero.
        if (GetOsclComponentRegistry()->iNumSessions == 0)
        {
            OSCL_DELETE(GetOsclComponentRegistry());
            OsclTLSRegistryEx::registerInstance(NULL, OSCL_TLS_ID_OSCLREGISTRY);
        }
    }
}

int32 OsclRegistryServTlsImpl::Register(OSCL_String& aComp, OsclComponentFactory aFac)
{
    if (!IsOpen())
        return OsclErrInvalidState;

    int32 err;
    OSCL_TRY(err, iIdVec.reserve(iIdVec.size() + 1););
    if (err != OsclErrNone)
        return err;

    uint32 id;
    int32 result = GetOsclComponentRegistry()->Register(id, aComp, aFac);

    //save all comp IDs in our session data
    if (result == OsclErrNone)
        iIdVec.push_back(id);//can't leave, already reserved space.

    return result;
}

int32 OsclRegistryServTlsImpl::UnRegister(OSCL_String& aComp)
{
    if (!IsOpen())
        return OsclErrInvalidState;

    return GetOsclComponentRegistry()->Unregister(aComp);
}

OsclComponentFactory OsclRegistryServTlsImpl::GetFactory(OSCL_String& aComp)
{
    if (!IsOpen())
    {
        return NULL;
    }

    return GetOsclComponentRegistry()->FindExact(aComp);
}

void OsclRegistryServTlsImpl::GetFactories(OSCL_String& aReg, Oscl_Vector<OsclRegistryAccessElement, OsclMemAllocator>& aVec)
{
    if (!IsOpen())
        return;

    GetOsclComponentRegistry()->FindHierarchical(aReg, aVec);
}










