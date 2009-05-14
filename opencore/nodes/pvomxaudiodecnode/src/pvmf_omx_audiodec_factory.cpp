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

#include "pvmf_omx_audiodec_node.h"
#include "pvmf_omx_audiodec_factory.h"

#include "oscl_error_codes.h"
#include "oscl_exception.h"

// Use default DLL entry point
#include "oscl_dll.h"

OSCL_DLL_ENTRY_POINT_DEFAULT()

/* Audio decoder node factory methods */


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFNodeInterface* PVMFOMXAudioDecNodeFactory::CreatePVMFOMXAudioDecNode(int32 aPriority)
{
    PVMFNodeInterface* node = NULL;
    node = new PVMFOMXAudioDecNode(aPriority);
    if (node == NULL)
    {
        OSCL_LEAVE(OsclErrNoMemory);
    }
    return node;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFOMXAudioDecNodeFactory::DeletePVMFOMXAudioDecNode(PVMFNodeInterface* aNode)
{
    if (aNode)
    {
        delete aNode;
        return true;
    }

    return false;
}

