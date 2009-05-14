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
#include "pvmp4ffrec_plugin.h"
#include "pvmp4ffrec_factory.h"

#include "oscl_error_codes.h"
#include "oscl_exception.h"

// Use default DLL entry point for Symbian
#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFRecognizerPluginInterface* PVMP4FFRecognizerFactory::CreateRecognizerPlugin()
{
    PVMFRecognizerPluginInterface* plugin = NULL;
    plugin = OSCL_STATIC_CAST(PVMFRecognizerPluginInterface*, OSCL_NEW(PVMP4FFRecognizerPlugin, ()));
    if (plugin == NULL)
    {
        OSCL_LEAVE(OsclErrNoMemory);
    }
    return plugin;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMP4FFRecognizerFactory::DestroyRecognizerPlugin(PVMFRecognizerPluginInterface* aPlugin)
{
    if (aPlugin)
    {
        OSCL_DELETE(((PVMP4FFRecognizerPlugin*)aPlugin));
    }
}

