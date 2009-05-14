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
 * @file pvsbcencoder_factory.cpp
 * @brief Singleton factory for CPVSbcEncoder
 */

#include "oscl_base.h"

#include "pvsbcencoder.h"
#include "pvsbcencoder_factory.h"

#include "oscl_error_codes.h"
#include "oscl_exception.h"

// Use default DLL entry point
#include "oscl_dll.h"

OSCL_DLL_ENTRY_POINT_DEFAULT()


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVSbcEncoderInterface* PVSbcEncoderFactory::CreatePVSbcEncoder()
{
    PVSbcEncoderInterface* sbcenc = NULL;
    sbcenc = CPVSbcEncoder::New();
    if (sbcenc == NULL)
    {
        OSCL_LEAVE(OsclErrNoMemory);
    }
    return sbcenc;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVSbcEncoderFactory::DeletePVSbcEncoder(PVSbcEncoderInterface* aSbcEnc)
{
    if (aSbcEnc)
    {
        OSCL_DELETE(aSbcEnc);
        return true;
    }

    return false;
}

