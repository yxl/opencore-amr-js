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
/*                              MPEG-4 ESID_Inc Class                            */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This ESID_Inc Class contains information on the Elementary Stream that will
	be replaced when streaming to a client.  The file format replaces the actual
	ES_Descriptor with this object that references the actual descriptor.
*/

#define IMPLEMENT_ES_ID_Inc

#include "es_id_inc.h"
#include "atomutils.h"
#include "atomdefs.h"


// Stream-in constructor
ES_ID_Inc::ES_ID_Inc(MP4_FF_FILE *fp)
        : BaseDescriptor(fp)
{
    AtomUtils::read32(fp, _trackID);
}


// Destructor
ES_ID_Inc::~ES_ID_Inc()
{
    // Empty
}


