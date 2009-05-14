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
    This PVA_FF_MediaInformationHeaderAtom Class is an abstract base class for the
    atoms VideoMediaHeaderAton, AudioMediaHeaderAtom, PVA_FF_HintMediaHeaderAtom,
    and PVA_FF_Mpeg4MediaHeaderAtom.  This class onbly contains one pure virtual method
    getMediaHeaderType() that will return what type of mediaInformationHeader
    this atom is.
*/


#define IMPLEMENT_MediaInformationAtomHeader

#include "mediainformationheaderatom.h"
#include "a_atomdefs.h"

// Constructor
PVA_FF_MediaInformationHeaderAtom::PVA_FF_MediaInformationHeaderAtom(uint32 atomType, uint8 version, uint32 flags)
        : PVA_FF_FullAtom(atomType, version, flags)
{
    // Empty
}

// Destructor
PVA_FF_MediaInformationHeaderAtom::~PVA_FF_MediaInformationHeaderAtom()
{
    //
}

