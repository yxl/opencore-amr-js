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
/*                        MPEG-4 AudioSampleEntry Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This AudioSampleEntry Class is used for visual streams.
*/

#define IMPLEMENT_AudioSampleEntry

#include "audiosampleentry.h"
#include "atomutils.h"
#include "atomdefs.h"


// Stream-in ctor
AudioSampleEntry::AudioSampleEntry(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : SampleEntry(fp, size, type)
{
    _pes = NULL;
    _pparent = NULL;

    if (_success)
    {
        // Read reserved values
        if (!AtomUtils::read32read32(fp, _reserved1[0], _reserved1[1]))
            _success = false;
        if (!AtomUtils::read16read16(fp, _channelCount, _sampleSize))
            _success = false;
        if (!AtomUtils::read16read16(fp, _preDefined, _reserved))
            _success = false;

        if (!AtomUtils::read16read16(fp, _timeScale, _sampleRateLo)) //_timeScale and _sampleRateHi are same
            _success = false;

        _sampleRateHi = _timeScale;


        if (_success)
        {
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
                _mp4ErrorCode = READ_AUDIO_SAMPLE_ENTRY_FAILED;
            }
        }
        else
        {
            _mp4ErrorCode = READ_AUDIO_SAMPLE_ENTRY_FAILED;
        }
    }
    else
    {
        _mp4ErrorCode = READ_AUDIO_SAMPLE_ENTRY_FAILED;
    }

}

// Destructor
AudioSampleEntry::~AudioSampleEntry()
{
    if (_pes != NULL)
    {
        // Cleanup ESDAtom
        PV_MP4_FF_DELETE(NULL, ESDAtom, _pes);
    }
}

