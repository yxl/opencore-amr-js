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
#include "atom.h"
#include "atomutils.h"
#include "oscl_file_io.h"
#include"avcconfigurationatom.h"
#include"a_atomdefs.h"

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
typedef Oscl_Vector<PVA_FF_AVCConfigurationAtom::PVA_FF_ParameterSet*, OsclMemAllocator> parameterSetVecType;



PVA_FF_AVCConfigurationAtom::PVA_FF_AVCConfigurationAtom(uint8 profile, uint8 profileComp, uint8 level):
        PVA_FF_Atom(AVC_CONFIGURATION_BOX)
{
    _configurationVersion = 1;
    _avcProfileIndication = 1;
    _avcLevelIndication = 1;
    _profileCompatibility = 1;
    _reserved_zero_5bits = 1;
    _lengthSize = 0;
    _numSequenceParameterSets = 0;
    _seqParameterSetLen = 0;
    _numPictureParameterSets = 0;
    _avcProfileIndication = profile;
    _profileCompatibility = profileComp;
    _avcLevelIndication = level;

    PV_MP4_FF_NEW(fp->auditCB, parameterSetVecType, (), _sequenceParameterSetVec);
    PV_MP4_FF_NEW(fp->auditCB, parameterSetVecType, (), _pictureParameterSetVec);
    recomputeSize();
}

void PVA_FF_AVCConfigurationAtom::setSample(void* samp, uint32 size)
{
    OSCL_UNUSED_ARG(size);
    if (_numSequenceParameterSets == 0 && _numPictureParameterSets == 0)
    {
        _sample = (uint8*)samp;

        if (_lengthSize == 0)
        {
            _lengthSize = NAL_UNIT_LENGTH;		// set length size of NAL units as 16 bits
        }
        if (_numSequenceParameterSets == 0)
        {
            _numSequenceParameterSets = *(uint8*)_sample;
            _sample += sizeof(uint8);
        }

        bool avcProfileIndicationErr = false;
        bool set_li = false;
        bool set_pi = false;
        bool set_pc = false;
        uint8 profile_comp = 0x00;

        for (int i = 0;i < _numSequenceParameterSets;i++)
        {

            oscl_memcpy(&_seqParameterSetLen, _sample, 2);
            _sample += sizeof(uint16);
            if (_seqParameterSetLen != 0)
            {
                if (_avcProfileIndication == 1)
                {
                    _avcProfileIndication = *(_sample + 1);
                    set_pi = true;
                }
                if (set_pi)
                {
                    if (_avcProfileIndication != *(_sample + 1))
                        avcProfileIndicationErr = true;
                }

                uint8 li = *(_sample + 3);
                uint8 pc = *(_sample + 2);

                if (_avcLevelIndication == 0xFF)
                {
                    _avcLevelIndication = li;
                    set_li = true;
                }
                if (set_li)
                {
                    if (li > _avcLevelIndication)
                        _avcLevelIndication = li;
                }

                if (_profileCompatibility == 0xFF)
                {
                    _avcLevelIndication = li;
                    set_pc = true;
                }
                if (set_pc)
                {
                    if (pc > profile_comp)
                        profile_comp = pc;
                }
                _totalSeqParameterSetLength += _seqParameterSetLen;

                _pSequenceParameterSet = _sample;

                PVA_FF_ParameterSet *paramSet = OSCL_NEW(PVA_FF_ParameterSet, (_seqParameterSetLen, _pSequenceParameterSet));


                _sample += _seqParameterSetLen;
                (*_sequenceParameterSetVec).push_back(paramSet);
            }
        }
        if (avcProfileIndicationErr)
            _avcProfileIndication = 1;

        if (_profileCompatibility == 0xFF)
        {
            if (((_avcProfileIndication == 0x66) ||
                    (_avcProfileIndication == 0x77) ||
                    (_avcProfileIndication == 0x88)) &&
                    (_avcLevelIndication == 0x11))
            {
                _profileCompatibility = 0x08;
            }

            if (_avcLevelIndication != 0x11)
            {
                if (_avcProfileIndication == 0x66)
                {
                    _profileCompatibility = 0x01;
                }
                else if (_avcProfileIndication == 0x77)
                {
                    _profileCompatibility = 0x02;
                }
                else if (_avcProfileIndication == 0x88)
                {
                    _profileCompatibility = 0x04;
                }
                else
                {
                    _profileCompatibility = profile_comp;
                }
            }
        }

        if (_numPictureParameterSets == 0)
        {
            _numPictureParameterSets = *(uint8*)_sample;
            _sample += sizeof(uint8);
        }
        for (int j = 0;j < _numPictureParameterSets;j++)
        {


            oscl_memcpy(&_picParameterSetLen, _sample, 2);
            _sample += sizeof(uint16);

            _totalPicutureParameterSetLength += _picParameterSetLen;

            _pPictureParameterSet = _sample;

            PVA_FF_ParameterSet *paramSet = OSCL_NEW(PVA_FF_ParameterSet, (_picParameterSetLen, _pPictureParameterSet));

            _sample += _picParameterSetLen;
            (*_pictureParameterSetVec).push_back(paramSet);
        }

    }
}
// Destructor
PVA_FF_AVCConfigurationAtom::~PVA_FF_AVCConfigurationAtom()
{

    if (_sequenceParameterSetVec != NULL)
    {
        for (uint32 i = 0; i < _sequenceParameterSetVec->size(); i++)
        {
            OSCL_DELETE((*_sequenceParameterSetVec)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, parameterSetVecType, Oscl_Vector, _sequenceParameterSetVec);
    }
    if (_pictureParameterSetVec != NULL)
    {
        for (uint32 i = 0; i < _pictureParameterSetVec->size(); i++)
        {
            OSCL_DELETE((*_pictureParameterSetVec)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, parameterSetVecType, Oscl_Vector, _pictureParameterSetVec);
    }

}

bool
PVA_FF_AVCConfigurationAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{

    int32 rendered = 0;
    uint16 length;
    uint8* paramSet;
    uint8  numSps = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render8(fp, _configurationVersion))
    {
        return false;
    }
    rendered += 1;

    if (!PVA_FF_AtomUtils::render8(fp, _avcProfileIndication))
    {
        return false;
    }
    rendered += 1;

    if (!PVA_FF_AtomUtils::render8(fp, _profileCompatibility))
    {
        return false;
    }
    rendered += 1;
    if (!PVA_FF_AtomUtils::render8(fp, _avcLevelIndication))
    {
        return false;
    }
    rendered += 1;


    if (!PVA_FF_AtomUtils::render8(fp, _lengthSize))
    {
        return false;
    }
    rendered += 1;
    numSps = _numSequenceParameterSets;
    numSps |= 0xE0;				// first 3 bits are reserved
    if (!PVA_FF_AtomUtils::render8(fp, numSps))
    {
        return false;
    }
    rendered += 1;
    for (int i = 0;i < _numSequenceParameterSets;i++)
    {

        PVA_FF_ParameterSet *pSet = (*_sequenceParameterSetVec)[i];

        length = pSet->getParameterSetLength();
        paramSet = pSet->getParameterSet();

        if (!PVA_FF_AtomUtils::render16(fp, length))
        {
            return false;
        }
        rendered += 2;

        if (!PVA_FF_AtomUtils::renderByteData(fp, length, paramSet))
        {
            return false;
        }
        rendered  += length;

    }

    if (!PVA_FF_AtomUtils::render8(fp, _numPictureParameterSets))
    {
        return false;
    }
    rendered += 1;
    if (_pictureParameterSetVec->size() < _numPictureParameterSets)
    {
        return false;
    }
    for (int j = 0;j < _numPictureParameterSets;j++)
    {

        PVA_FF_ParameterSet *pSet = (*_pictureParameterSetVec)[j];

        length = pSet->getParameterSetLength();
        paramSet = pSet->getParameterSet();

        if (!PVA_FF_AtomUtils::render16(fp, length))
        {
            return false;
        }
        rendered += 2;

        if (!PVA_FF_AtomUtils::renderByteData(fp, length, paramSet))
        {
            return false;
        }
        rendered  += length;

    }

    return true;
}

void
PVA_FF_AVCConfigurationAtom::recomputeSize()
{
    int32 size = getDefaultSize();


    size += 7; // wrote six one byte data

    if (_sequenceParameterSetVec->size() >= _numSequenceParameterSets)
    {
        for (int i = 0;i < _numSequenceParameterSets;i++)
        {
            PVA_FF_ParameterSet *pSet = (*_sequenceParameterSetVec)[i];

            uint16	length = pSet->getParameterSetLength();
            size += 2;
            size += length;
        }
    }
    if (_pictureParameterSetVec->size() >= _numPictureParameterSets)
    {
        for (int j = 0;j < _numPictureParameterSets;j++)
        {
            PVA_FF_ParameterSet *pSet = (*_pictureParameterSetVec)[j];

            uint16	length = pSet->getParameterSetLength();
            size += 2;
            size += length;
        }
    }

    _size = size;

    // Update size of parent
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }

}

uint32
PVA_FF_AVCConfigurationAtom::getSize()
{
    recomputeSize();
    return (_size);
}
