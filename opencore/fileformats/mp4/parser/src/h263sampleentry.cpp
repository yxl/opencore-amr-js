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
/*                        MPEG-4 H263SampleEntry Class                           */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    H263SampleEntry
*/


#define IMPLEMENT_H263SampleEntry

#include "h263sampleentry.h"
#include "h263decoderspecificinfo.h"
#include "atomutils.h"
#include "atomdefs.h"


H263SampleEntry::H263SampleEntry(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    _pH263SpecificAtom = NULL;
    _pH263decSpecificInfo = NULL;

    if (_success)
    {
        _pparent = NULL;

        // Read reserved values
        if (!AtomUtils::read8read8(fp, _reserved[0], _reserved[1]))
            _success = false;
        if (!AtomUtils::read8read8(fp, _reserved[2], _reserved[3]))
            _success = false;
        if (!AtomUtils::read8read8(fp, _reserved[4], _reserved[5]))
            _success = false;

        if (!AtomUtils::read16(fp, _dataReferenceIndex))
            _success = false;

        // Read in all reserved members
        for (int32 i = 0; i < 4; i++)
        {
            if (!AtomUtils::read32(fp, _reserved1[i]))
            {
                _success = false;
                break;
            }
        }

        if (_success)
        {
            if (!AtomUtils::read16(fp, _width))
                _success = false;
            if (!AtomUtils::read16(fp, _height))
                _success = false;
            if (!AtomUtils::read32(fp, _reserved3))
                _success = false;
            if (!AtomUtils::read32read32(fp, _reserved4, _reserved5))
                _success = false;
            if (!AtomUtils::read16(fp, _reserved6))
                _success = false;

            for (int32 i = 0; i < 32; i++)
            {
                if (!AtomUtils::read8(fp, _reserved7[i]))
                {
                    _success = false;
                    break;
                }
            }

            if (!AtomUtils::read16read16(fp, _reserved8, _reserved9))
                _success = false;
        }

        if (_success)
        {
            uint32 atom_type = UNKNOWN_ATOM;
            uint32 atom_size = 0;

            AtomUtils::getNextAtomType(fp, atom_size, atom_type);

            if (atom_type != H263_SPECIFIC_ATOM)
            {
                _success = false;
                _mp4ErrorCode = READ_H263_SAMPLE_ENTRY_FAILED;
            }
            else
            {
                PV_MP4_FF_NEW(fp->auditCB, H263SpecficAtom, (fp, atom_size, atom_type), _pH263SpecificAtom);

                if (_pH263SpecificAtom->MP4Success())
                {
                    PV_MP4_FF_NEW(fp->auditCB, H263DecoderSpecificInfo, (fp, true), _pH263decSpecificInfo);

                    _pH263decSpecificInfo->_VendorCode = _pH263SpecificAtom->getVendorCode();
                    _pH263decSpecificInfo->_codec_version = _pH263SpecificAtom->getDecoderVersion();
                    _pH263decSpecificInfo->_codec_level = _pH263SpecificAtom->getCodecLevel();
                    _pH263decSpecificInfo->_codec_profile = _pH263SpecificAtom->getCodecProfile();
                    _pH263decSpecificInfo->_max_width = _width;
                    _pH263decSpecificInfo->_max_height = _height;
                }
            }
        }
        else
        {
            _mp4ErrorCode = READ_H263_SAMPLE_ENTRY_FAILED;
        }
    }
    else
    {
        _mp4ErrorCode = READ_H263_SAMPLE_ENTRY_FAILED;
    }

}

// Destructor
H263SampleEntry::~H263SampleEntry()
{
    if (_pH263SpecificAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, H263SpecficAtom, _pH263SpecificAtom);
    }

    if (_pH263decSpecificInfo != NULL)
    {
        PV_MP4_FF_DELETE(NULL, H263DecoderSpecificInfo, _pH263decSpecificInfo);
    }
}
