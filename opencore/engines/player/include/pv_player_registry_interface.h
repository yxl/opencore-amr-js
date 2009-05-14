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
#ifndef PV_PLAYER_REGISTRY_INTERFACE_H_INCLUDED
#define PV_PLAYER_REGISTRY_INTERFACE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif
#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif

#define PV_PLAYER_REGISTRY_INTERFACE_UUID OsclUuid(0xa054569c,0x24c5,0x452e,0x99,0x77,0x87,0x4b,0xca,0x79,0xd3,0xaf)

/**
 * Dynamic registry interface for pvPlayer
 */

class PVPlayerRegistryInterface : public PVInterface
{
    public:
        virtual ~PVPlayerRegistryInterface() {};

        /**
         * Method to populate all registries with dynamic components as specified
         * in the config file
         * @param aPath [in] This is a path to a shared library config file that
         * specifies the UUIDs and the corresponding dynamic libraries
         */
        virtual void PopulateAllRegistries(const OSCL_String& aPath) = 0;

        /**
         * Method to populate the node registry
         */
        virtual void PopulateNodeRegistry(const OSCL_String& aPath) = 0;

        /**
         * Method to populate the recognizer registry
         */
        virtual void PopulateRecognizerRegistry(const OSCL_String& aPath) = 0;

        /**
         * Method to remove dynamic components from all the registers as sepcified in the config file
         * @param aPath [in] This is a path to a shared library config file that
             * specifies the UUIDs and the corresponding dynamic libraries
             */
        virtual void DepopulateAllRegistries(const OSCL_String& aPath) = 0;

        /**
         * Method to depopulate the dynamic components in node registry
         */
        virtual void DepopulateNodeRegistry(const OSCL_String& aPath) = 0;

        /**
         * Method to depopulate the dynamic components in recognizer registry
         */
        virtual void DepopulateRecognizerRegistry(const OSCL_String& aPath) = 0;
};


#endif

