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
#include "pv_2way_proxy_factory.h"
#include "pv_2way_engine_factory.h"
#include "pv_2way_proxy_adapter.h"
#include "pv_2way_engine.h"

OSCL_EXPORT_REF void CPV2WayProxyFactory::Init()
{
    CPV2WayEngineFactory::Init();
}

OSCL_EXPORT_REF void CPV2WayProxyFactory::Cleanup()
{
    CPV2WayEngineFactory::Cleanup();
}

OSCL_EXPORT_REF CPV2WayInterface *CPV2WayProxyFactory::CreateTerminal(TPVTerminalType aTerminalType,
        PVCommandStatusObserver* aCmdStatusObserver,
        PVInformationalEventObserver *aInfoEventObserver,
        PVErrorEventObserver *aErrorEventObserver)
{
    if (aTerminalType == PV_324M)
    {
        return CPV2WayProxyAdapter::New(aTerminalType,
                                        aCmdStatusObserver,
                                        aInfoEventObserver,
                                        aErrorEventObserver);
    }
    else
    {
        return NULL;
    }
}

OSCL_EXPORT_REF void CPV2WayProxyFactory::DeleteTerminal(CPV2WayInterface* terminal)
{
    OSCL_DELETE((CPV2WayProxyAdapter*)terminal);
}
