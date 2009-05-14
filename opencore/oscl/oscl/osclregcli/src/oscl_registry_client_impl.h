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

//				 Oscl Registry Client Impl

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \addtogroup osclutil OSCL Util
 *
 * @{
 */

/*!
 * \file oscl_registry_client_impl.h
 * \brief Client-side implementation of OsclRegistryInterface
 *
 */

#ifndef OSCL_REGISTRY_CLIENT_IMPL_H_INCLUDED
#define OSCL_REGISTRY_CLIENT_IMPL_H_INCLUDED

#include "oscl_base.h"
#include "osclconfig_proc.h"
#if (OSCL_HAS_SINGLETON_SUPPORT)
//1st choice implementation-- uses Oscl singleton

#include "oscl_registry_serv_impl_global.h"

//client is the same as server
class OsclRegistryClientImpl: public OsclRegistryServImpl
{
};

//access client is also the same as server
class OsclRegistryAccessClientImpl: public OsclRegistryServImpl
{
};

#else //OSCL_HAS_ ...
//3rd choice implementation -- Non-functional stubs.

#include "oscl_vector.h"
#include "oscl_string.h"
#include "oscl_registry_types.h"

class OsclRegistryClientImpl
{
    protected:

        int32 Connect()
        {
            return OsclErrNotSupported;
        }
        void Close() {}

        int32 Register(OSCL_String& , OsclComponentFactory)
        {
            return OsclErrNotSupported;
        }
        int32 UnRegister(OSCL_String&)
        {
            return OsclErrNotSupported;
        }

        //for access client.
        OsclComponentFactory GetFactory(OSCL_String&)
        {
            return NULL;
        }
        void GetFactories(OSCL_String& , Oscl_Vector<OsclRegistryAccessElement, OsclMemAllocator>&) {}

        friend class OsclRegistryClient;
        friend class OsclRegistryAccessClient;
};

class OsclRegistryAccessClientImpl: public OsclRegistryClientImpl
{
};

#endif //OSCL_HAS_ ...


//TLS-based registry
#include "oscl_registry_serv_impl_tls.h"

//client is the same as server
class OsclRegistryClientTlsImpl: public OsclRegistryServTlsImpl
{
};

//access client is also the same as server
class OsclRegistryAccessClientTlsImpl: public OsclRegistryServTlsImpl
{
};

#endif //OSCL_REGISTRY_IMPL_H_INCLUDED
/*! @} */


