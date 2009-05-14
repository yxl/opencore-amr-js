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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                              MPEG-4 ESID_Ref Class                            */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This ESID_Ref Class contains information on the Elementary Stream that will
	be replaced when streaming to a client.  The file format replaces the actual
	ES_Descriptor with this object that references the actual descriptor.  This
	is used in the OD Stream!
*/

#define IMPLEMENT_ES_ID_Ref

#include "es_id_ref.h"
#include "atomutils.h"
#include "atomdefs.h"


// Stream-in constructor
ES_ID_Ref::ES_ID_Ref(MP4_FF_FILE *fp)
        : BaseDescriptor(fp)
{
    AtomUtils::read16(fp, _trackIndex);
}

// Destructor
ES_ID_Ref::~ES_ID_Ref()
{
    // Empty
}




