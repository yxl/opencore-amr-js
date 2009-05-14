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

#include "oscl_registry_access_client.h"
#include "oscl_registry_client_impl.h"
#include "oscl_error.h"


OSCL_EXPORT_REF OsclRegistryAccessClient::OsclRegistryAccessClient()
{
    iGlobalImpl = NULL;
    iTlsImpl = NULL;
}


OSCL_EXPORT_REF OsclRegistryAccessClient::~OsclRegistryAccessClient()
{
    Close();
}

OSCL_EXPORT_REF int32 OsclRegistryAccessClient::Connect()
{
    //Decide whether to use TLS-based or global registry
    //If there is anything in the TLS-based registry then use
    //TLS-based.
    if (OsclTLSRegistryEx::getInstance(OSCL_TLS_ID_OSCLREGISTRY) != NULL)
    {
        //avoid mixing session types
        if (iGlobalImpl)
            return OsclErrInvalidState;

        if (!iTlsImpl)
        {
            int32 err;
            OSCL_TRY(err, iTlsImpl = OSCL_NEW(OsclRegistryAccessClientTlsImpl, ()););
            if (err != OsclErrNone)
                return err;
        }
        return iTlsImpl->Connect();
    }
    else
    {
        if (!iGlobalImpl)
        {
            int32 err;
            OSCL_TRY(err, iGlobalImpl = OSCL_NEW(OsclRegistryAccessClientImpl, ()););
            if (err != OsclErrNone)
                return err;
        }
        return iGlobalImpl->Connect();
    }
}

OSCL_EXPORT_REF void OsclRegistryAccessClient::Close()
{
    if (iTlsImpl)
    {
        iTlsImpl->Close();
        OSCL_DELETE(iTlsImpl);
        iTlsImpl = NULL;
    }
    else
        if (iGlobalImpl)
        {
            iGlobalImpl->Close();
            OSCL_DELETE(iGlobalImpl);
            iGlobalImpl = NULL;
        }
}

OSCL_EXPORT_REF OsclComponentFactory OsclRegistryAccessClient::GetFactory(OSCL_String& aReg)
{
    if (iTlsImpl)
        return iTlsImpl->GetFactory(aReg);

    else
        if (iGlobalImpl)
            return iGlobalImpl->GetFactory(aReg);

        else
            return NULL;
}

OSCL_EXPORT_REF void OsclRegistryAccessClient::GetFactories(OSCL_String& aReg, Oscl_Vector<OsclRegistryAccessElement, OsclMemAllocator>& aVec)
{
    if (iTlsImpl)
        iTlsImpl->GetFactories(aReg, aVec);

    else
        if (iGlobalImpl)
            iGlobalImpl->GetFactories(aReg, aVec);

}





