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
/**
 *  @file pvmf_interface.h
 *  @brief This file defines the PV interface class.
 *  This class serves as the base for interfaces and provides methods
 *  to query for other interfaces, and to increment and decrement reference counts.
 *
 */

#ifndef PV_INTERFACE_H
#define PV_INTERFACE_H


#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif

class PVInterface
{

    public:
        virtual void addRef() = 0;
        virtual void removeRef() = 0;
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;

        virtual ~PVInterface() {}
};

#endif
