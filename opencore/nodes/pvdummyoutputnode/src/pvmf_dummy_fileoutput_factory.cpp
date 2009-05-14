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
 * @file pvmf_fileoutput_factory.cpp
 * @brief Singleton factory for PVMFDummyFileOutputNode
 */

#include "oscl_base.h"

#include "pvmf_dummy_fileoutput_node.h"

#include "pvmf_dummy_fileoutput_factory.h"

#include "pvmf_dummy_fileoutput_inport.h"

#include "oscl_error_codes.h"

#include "oscl_exception.h"

#include "oscl_dll.h"

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFNodeInterface* PVMFDummyFileOutputNodeFactory::CreateDummyFileOutput(int32 aPriority)
{
    PVMFNodeInterface* node = NULL;
    node = OSCL_NEW(PVMFDummyFileOutputNode, (aPriority));
    if (node == NULL)
    {
        OSCL_LEAVE(OsclErrNoMemory);
    }
    return node;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFNodeInterface* PVMFDummyFileOutputNodeFactory::CreateDummyFileOutput(OSCL_wString &aFileName, PVMFFormatType aFormat, int32 aPriority)
{
    PVMFNodeInterface* node = NULL;
    node = OSCL_NEW(PVMFDummyFileOutputNode, (aPriority));
    if (node == NULL)
    {
        OSCL_LEAVE(OsclErrNoMemory);
    }
    ((PVMFDummyFileOutputNode *)node)->iFormat = aFormat;
    ((PVMFDummyFileOutputNode *)node)->iOutputFileName = aFileName.get_cstr();
    return node;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFDummyFileOutputNodeFactory::DeleteDummyFileOutput(PVMFNodeInterface*& aNode)
{
    if (aNode)
    {
        OSCL_DELETE(aNode);
        return true;
    }

    return false;
}

