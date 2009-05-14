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
 * @file pvmf_socket_factory.cpp
 * @brief Singleton factory for PVRTSPEngineNode
 */

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif

#ifndef OSCL_EXCEPTION_H_INCLUDED
#include "oscl_exception.h"
#endif

#ifndef PVRTSP_CLIENT_ENGINE_NODE_H
#include "pvrtsp_client_engine_node.h"
#endif
#include "pvrtsp_client_engine_factory.h"

// Use default DLL entry point for Symbian
#include "oscl_dll.h"

OSCL_DLL_ENTRY_POINT_DEFAULT()

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFNodeInterface* PVMFRrtspEngineNodeFactory::CreatePVMFRtspEngineNode(int32 aPriority)
{
    PVMFNodeInterface* node = NULL;
    node = OSCL_NEW(PVRTSPEngineNode, (aPriority));
    if (node == NULL)
    {
        OSCL_LEAVE(OsclErrNoMemory);
    }
    return node;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFRrtspEngineNodeFactory::DeletePVMFRtspEngineNode(PVMFNodeInterface* aNode)
{
    if (aNode)
    {
        OSCL_DELETE(((PVRTSPEngineNode*)aNode));
        return true;
    }

    return false;
}

