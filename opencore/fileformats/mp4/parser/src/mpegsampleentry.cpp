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
/*                         MPEG-4 MpegSampleEntry Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MpegSampleEntry is used for hint tracks.
*/


#define IMPLEMENT_MpegSampleEntry

#include "mpegsampleentry.h"
#include "atomdefs.h"
#include "atomutils.h"


// Stream-in constructor
MpegSampleEntry::MpegSampleEntry(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : SampleEntry(fp, size, type)
{
    _pes = NULL;

    if (_success)
    {
        _pparent = NULL;

        uint32 atomType = UNKNOWN_ATOM;
        uint32 atomSize = 0;

        AtomUtils::getNextAtomType(fp, atomSize, atomType);

        if (atomType == ESD_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ESDAtom, (fp, atomSize, atomType), _pes);

            if (!_pes->MP4Success())
            {
                _success = false;
                _mp4ErrorCode = _pes->GetMP4Error();
            }
            else
            {
                _pes->setParent(this);
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_MPEG_SAMPLE_ENTRY_FAILED;
        }
    }
    else
    {
        _mp4ErrorCode = READ_MPEG_SAMPLE_ENTRY_FAILED;
    }
}



// Destructor
MpegSampleEntry::~MpegSampleEntry()
{
    if (_pes != NULL)
    {
        // Cleanup ESDAtom
        PV_MP4_FF_DELETE(NULL, ESDAtom, _pes);
    }
}
