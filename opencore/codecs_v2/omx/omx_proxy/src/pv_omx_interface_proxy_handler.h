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


#ifndef PV_OMX_INTERFACE_PROXY_HANDLER_H_INCLUDED
#define PV_OMX_INTERFACE_PROXY_HANDLER_H_INCLUDED

#ifndef PV_OMXDEFS_H_INCLUDED
#include "pv_omxdefs.h"
#endif

#ifndef PV_OMX_INTERFACE_PROXY_H_INCLUDED
#include "pv_omx_interface_proxy.h"
#endif

#ifndef PV_INTERFACE_CMD_MESSAGE_H_INCLUDED
#include "pv_interface_cmd_message.h"
#endif

#include "oscl_scheduler_ao.h"

#if PROXY_INTERFACE

class CPVInterfaceProxyHandler_OMX :
            public OsclActiveObject
{
    public:
        CPVInterfaceProxyHandler_OMX(CPVInterfaceProxy_OMX *aProxy, int32 pri):
                OsclActiveObject(pri, "proxyHandler")
                , iProxy(aProxy)
        {}

        virtual ~CPVInterfaceProxyHandler_OMX()
        {}

    private:
        void Run();

        CPVInterfaceProxy_OMX *iProxy;
};

#endif // PROXY_INTERFACE
#endif //


