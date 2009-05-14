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

#define IMPLEMENT_AMRSampleEntry

#include "amrsampleentry.h"
#include "amrdecoderspecificinfo.h"
#include "atomutils.h"
#include "atomdefs.h"

typedef Oscl_Vector<DecoderSpecificInfo*, OsclMemAllocator> decoderSpecificInfoVecType;
// Stream-in ctor
AMRSampleEntry::AMRSampleEntry(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    _pAMRSpecificAtom = NULL;
    _pAMRDecSpecInfoArray = NULL;

    if (_success)
    {
        // Read reserved values
        if (!AtomUtils::read8read8(fp, _reserved[0], _reserved[1]))
            _success = false;
        if (!AtomUtils::read8read8(fp, _reserved[2], _reserved[3]))
            _success = false;
        if (!AtomUtils::read8read8(fp, _reserved[4], _reserved[5]))
            _success = false;

        if (!AtomUtils::read16(fp, _dataReferenceIndex))
            _success = false;

        if (!AtomUtils::read32read32(fp, _reserved1[0], _reserved1[1]))
            _success = false;
        if (!AtomUtils::read16read16(fp, _reserved2, _reserved3))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved4))
            _success = false;

        if (!AtomUtils::read16read16(fp, _timeScale, _reserved5))
            _success = false;

        if (_success)
        {
            uint32 atom_type = UNKNOWN_ATOM;
            uint32 atom_size = 0;

            AtomUtils::getNextAtomType(fp, atom_size, atom_type);

            if (atom_type != AMR_SPECIFIC_ATOM)
            {
                _success = false;
                _mp4ErrorCode = READ_AMR_SAMPLE_ENTRY_FAILED;
            }
            else
            {
                PV_MP4_FF_NEW(fp->auditCB, AMRSpecificAtom, (fp, atom_size, atom_type), _pAMRSpecificAtom);

                if (!_pAMRSpecificAtom->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = READ_AMR_SAMPLE_ENTRY_FAILED;
                }

                PV_MP4_FF_NEW(fp->auditCB, decoderSpecificInfoVecType, (), _pAMRDecSpecInfoArray);

                for (uint32 i = 0; i < 16; i++)
                {
                    AMRDecoderSpecificInfo *pinfo = NULL;
                    PV_MP4_FF_NEW(fp->auditCB, AMRDecoderSpecificInfo, (fp, true), pinfo);

                    pinfo->_frame_type = (uint8)i;
                    pinfo->_codec_version = _pAMRSpecificAtom->getDecoderVersion();
                    pinfo->_mode_change_period = _pAMRSpecificAtom->getModeChangePeriod();
                    pinfo->_mode_set = _pAMRSpecificAtom->getModeSet();
                    pinfo->_mode_change_neighbour = false;

                    (*_pAMRDecSpecInfoArray).push_back(pinfo);
                }
            }
        }
        else
        {
            _mp4ErrorCode = READ_AMR_SAMPLE_ENTRY_FAILED;
        }
    }
    else
    {
        _mp4ErrorCode = READ_AMR_SAMPLE_ENTRY_FAILED;
    }

}

// Destructor
AMRSampleEntry::~AMRSampleEntry()
{
    if (_pAMRSpecificAtom != NULL)
    {
        // Cleanup ESDAtom
        PV_MP4_FF_DELETE(NULL, AMRSpecificAtom, _pAMRSpecificAtom);
    }
    if (_pAMRDecSpecInfoArray != NULL)
    {
        for (uint32 i = 0; i < _pAMRDecSpecInfoArray->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, DecoderSpecificInfo, (*_pAMRDecSpecInfoArray)[i]);
        }
    }

    if (_pAMRDecSpecInfoArray != NULL)
    {
        PV_MP4_FF_TEMPLATED_DELETE(NULL, decoderSpecificInfoVecType, Oscl_Vector, _pAMRDecSpecInfoArray);
        _pAMRDecSpecInfoArray = NULL;
    }

}

DecoderSpecificInfo *
AMRSampleEntry::getDecoderSpecificInfo(uint32 index)
{
    if ((_pAMRDecSpecInfoArray->size() == 0) ||
            (index >= (uint32)(_pAMRDecSpecInfoArray->size())))
    {
        return NULL;
    }
    else
    {
        return ((*_pAMRDecSpecInfoArray)[index]);
    }
}
