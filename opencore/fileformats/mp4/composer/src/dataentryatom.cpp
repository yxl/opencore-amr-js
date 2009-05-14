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
/*
    This PVA_FF_DataEntryAtom Class is the base class for the PVA_FF_DataEntryUrlAtom and
    PVA_FF_DataEntryUrnAtom classes.
*/
#define IMPLEMENT_DataEntryAtom_H__

#include "dataentryatom.h"

// Constructor
PVA_FF_DataEntryAtom::PVA_FF_DataEntryAtom(uint32 type, uint8 version, uint32 flags)
        : PVA_FF_FullAtom(type, version, flags)
{
    // Empty
}

// Destructor
PVA_FF_DataEntryAtom::~PVA_FF_DataEntryAtom()
{
    // Empty
}
