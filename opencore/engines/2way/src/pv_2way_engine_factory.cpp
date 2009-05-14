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
#include "oscl_base.h"
#include "oscl_scheduler.h"
#include "pv_2way_engine_factory.h"
#include "pv_2way_engine.h"

OSCL_EXPORT_REF void CPV2WayEngineFactory::Init()
{
    // initialize Oscl and PVlogger ASAP in each thread
    OsclBase::Init();
    OsclMem::Init();
    PVLogger::Init();
}

OSCL_EXPORT_REF void CPV2WayEngineFactory::Cleanup()
{
    PVLogger::Cleanup();
    OsclMem::Cleanup();
    OsclBase::Cleanup();
}

OSCL_EXPORT_REF CPV2WayInterface *CPV2WayEngineFactory::CreateTerminal(PV2WayTerminalType aTerminalType,
        PVCommandStatusObserver* aCmdStatusObserver,
        PVInformationalEventObserver *aInfoEventObserver,
        PVErrorEventObserver *aErrorEventObserver)
{

    if (aTerminalType == PV_324M ||
            aTerminalType == PV_SIP)
    {
        return CPV324m2Way::NewL(NULL,
                                 aTerminalType,
                                 aCmdStatusObserver,
                                 aInfoEventObserver,
                                 aErrorEventObserver);
    }
    else
    {
        OSCL_LEAVE(PVMFErrNotSupported);
    }
    return NULL;
}

OSCL_EXPORT_REF void CPV2WayEngineFactory::DeleteTerminal(CPV2WayInterface* terminal)
{
    CPV324m2Way::Delete((CPV324m2Way*) terminal);
}
