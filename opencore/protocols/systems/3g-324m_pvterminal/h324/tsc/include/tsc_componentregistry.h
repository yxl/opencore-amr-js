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
#ifndef TSC_COMPONENTREGISTRY_H
#define TSC_COMPONENTREGISTRY_H

#define TSC_COMPONENT_MIME_TYPE "x-pvmf/component"

#include "tsc_node_interface.h"
#include "tsc_component.h"


class TSC_statemanager;
class TSC_capability;
class TSC_lc;
class TSC_blc;
class TSC_clc;
class TSC_mt;

class TSCComponentRegistry
{
    public:
        TSCComponentRegistry(TSC_statemanager& aTSCStateManager,
                             TSC_capability& aTSCcapability,
                             TSC_lc& aTSClc,
                             TSC_blc& aTSCblc,
                             TSC_clc& aTSCclc,
                             TSC_mt& aTSCmt);

        void QueryRegistry(Tsc324mNodeCommand& cmd);

        TSC_component* Create(PVMFSessionId aSession,
                              const PVUuid& aUuid,
                              PVInterface*& aInterfacePtr,
                              const OsclAny* aContext);
    private:
        TSC_statemanager& iTSCstatemanager;
        TSC_capability& iTSCcapability;
        TSC_lc& iTSClc;
        TSC_blc& iTSCblc;
        TSC_clc& iTSCclc;
        TSC_mt& iTSCmt;
};

#endif

