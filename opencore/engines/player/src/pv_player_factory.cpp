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
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PV_PLAYER_INTERFACE_H_INCLUDED
#include "pv_player_interface.h"
#endif

#ifndef PV_ENGINE_OBSERVER_H_INCLUDED
#include "pv_engine_observer.h"
#endif

#ifndef PV_PLAYER_ENGINE_H_INCLUDED
#include "pv_player_engine.h"
#endif

#ifndef PV_PLAYER_FACTORY_H_INCLUDED
#include "pv_player_factory.h"
#endif

#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

OSCL_EXPORT_REF PVPlayerInterface *PVPlayerFactory::CreatePlayer(PVCommandStatusObserver* aCmdStatusObserver,
        PVErrorEventObserver *aErrorEventObserver,
        PVInformationalEventObserver *aInfoEventObserver)
{
    return PVPlayerEngine::New(aCmdStatusObserver, aErrorEventObserver, aInfoEventObserver);
}

OSCL_EXPORT_REF bool PVPlayerFactory::DeletePlayer(PVPlayerInterface* aPlayer)
{
    PVPlayerEngine* engptr = (PVPlayerEngine*)aPlayer;
    OSCL_DELETE(engptr);

    return true;
}


