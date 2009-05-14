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
#define IMPLEMENT_TrackFragmentRunAtom

#include "trackfragmentrunatom.h"
#include "atomutils.h"
#include "atomdefs.h"

typedef Oscl_Vector<TFrunSampleTable*, OsclMemAllocator> _pTFrunSampleTableVecType;

TFrunSampleTable::TFrunSampleTable(MP4_FF_FILE *fp , uint32 tr_flag, uint32 base_data_offset,
                                   uint64 sampleTS)
{
    _sample_duration = 0;
    _sample_size = 0;
    _sample_flags = 0;
    _sample_composition_time_offset = 0;
    _sample_offset = 0;
    _sample_timestamp = sampleTS;
    _sample_offset = base_data_offset;

    if (tr_flag & 0x000100)
    {
        if (!AtomUtils::read32(fp, _sample_duration))
        {
            return;
        }
    }

    if (tr_flag & 0x000200)
    {
        if (!AtomUtils::read32(fp, _sample_size))
        {
            return;
        }
    }

    if (tr_flag & 0x000400)
    {
        if (!AtomUtils::read32(fp, _sample_flags))
        {
            return;
        }
    }

    if (tr_flag & 0x000800)
    {
        if (!AtomUtils::read32(fp, _sample_composition_time_offset))
        {
            return;
        }
    }
}

TrackFragmentRunAtom ::TrackFragmentRunAtom(MP4_FF_FILE *fp, uint32 size, uint32 type,
        uint32 baseDataOffset,
        uint32 &currentTrunOffset,
        uint32 &offset,
        uint64 trackDuration,
        bool bdo_present,
        bool &trunParsingCompleted,
        uint32 &countOfTrunsParsed)
        : FullAtom(fp, size, type)
{
    OSCL_UNUSED_ARG(type);
    _data_offset = 0;
    _sample_count = 0;
    _first_sample_flags = 0;
    _sampleTimeStamp = trackDuration;
    tr_flag = getFlags();
    _trun_start_offset  = currentTrunOffset;
    _samplesToBeParsed = 0;
    _partialTrunOffset = 0;

    iLogger = PVLogger::GetLoggerObject("mp4ffparser");
    iStateVarLogger = PVLogger::GetLoggerObject("mp4ffparser_mediasamplestats");
    iParsedDataLogger = PVLogger::GetLoggerObject("mp4ffparser_parseddata");

    if (_success)
    {
        if (!AtomUtils::read32(fp, _sample_count))
        {
            _success = false;
            _mp4ErrorCode = READ_TRACK_FRAGMENT_RUN_ATOM_FAILED;
            return;
        }

        if (tr_flag & 0x000001)
        {
            if (!AtomUtils::read32(fp, _data_offset))
            {
                _success = false;
                _mp4ErrorCode = READ_TRACK_FRAGMENT_RUN_ATOM_FAILED;
                return;
            }
            _trun_start_offset = baseDataOffset;
            _trun_start_offset += _data_offset;
        }
        else if (bdo_present)
        {
        }
        else
        {
            _trun_start_offset += offset;
        }

        if (tr_flag & 0x000004)
        {
            if (!AtomUtils::read32(fp, _first_sample_flags))
            {
                _success = false;
                _mp4ErrorCode = READ_TRACK_FRAGMENT_RUN_ATOM_FAILED;
                return;
            }
        }

        if (_sample_count > 0)
        {
            PV_MP4_FF_NEW(fp->auditCB, _pTFrunSampleTableVecType, (), _pTFrunSampleTable);
            _pTFrunSampleTable->reserve(_sample_count);
            _samplesToBeParsed = _sample_count;
        }

        if ((countOfTrunsParsed > COUNT_OF_TRUNS_PARSED_THRESHOLD) && (_sample_count > 25))
        {
            if (trunParsingCompleted)
            {
                trunParsingCompleted = false;
                _samplesToBeParsed = _sample_count / 2;
                if (_sample_count % 2 != 0)
                {
                    _samplesToBeParsed += 1;
                }
            }
            else
            {
                trunParsingCompleted = true;
                _samplesToBeParsed = _sample_count - _samplesToBeParsed;
            }
        }

        uint32 sigmaSampleSize = 0;
        uint32 sample_offset = _trun_start_offset;
        for (uint32 idx = 0; idx < _samplesToBeParsed ;idx++)
        {
            TFrunSampleTable *pTFrunSampleTable = NULL;
            PV_MP4_FF_NEW(fp->auditCB, TFrunSampleTable, (fp, tr_flag, sample_offset, _sampleTimeStamp), pTFrunSampleTable);
            _pTFrunSampleTable->push_back(pTFrunSampleTable);
            if (pTFrunSampleTable != NULL)
            {
                PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "Track Fragment Run=>Sample Number			=%d", idx));
                PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "Track Fragment Run=>Sample Offset			=%d", sample_offset));
                PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "Track Fragment Run=>Sample Sample Size		=%d", pTFrunSampleTable->_sample_size));
                sample_offset += pTFrunSampleTable->_sample_size;
                sigmaSampleSize += pTFrunSampleTable->_sample_size;
                _sampleTimeStamp += pTFrunSampleTable->_sample_duration;

            }
        }
        offset = sigmaSampleSize;
        currentTrunOffset = _trun_start_offset;
        _partialTrunOffset = sample_offset;

        if (trunParsingCompleted)
        {
            countOfTrunsParsed++;
        }

        PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "###################################"));

    }
}
// Destructor
TrackFragmentRunAtom::~TrackFragmentRunAtom()
{
    if (_pTFrunSampleTable != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pTFrunSampleTable->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, TFrunSampleTable, (*_pTFrunSampleTable)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, _pTFrunSampleTableVecType, Oscl_Vector, _pTFrunSampleTable);
    }

}

void TrackFragmentRunAtom::ParseTrunAtom(MP4_FF_FILE *fp,
        uint32 &offset,
        bool &trunParsingCompleted,
        uint32 &countOfTrunsParsed)
{
    if (countOfTrunsParsed > COUNT_OF_TRUNS_PARSED_THRESHOLD)
    {
        if (trunParsingCompleted)
        {
            trunParsingCompleted = false;
            _samplesToBeParsed = _sample_count / 2;
            if (_sample_count % 2 != 0)
            {
                _samplesToBeParsed += 1;
            }
        }
        else
        {
            trunParsingCompleted = true;
            _samplesToBeParsed = _sample_count - _samplesToBeParsed;
        }
    }

    uint32 sigmaSampleSize = 0;
    uint32 sample_offset = _partialTrunOffset;
    for (uint32 idx = 0; idx < _samplesToBeParsed ;idx++)
    {
        TFrunSampleTable *pTFrunSampleTable = NULL;
        PV_MP4_FF_NEW(fp->auditCB, TFrunSampleTable, (fp, tr_flag, sample_offset, _sampleTimeStamp), pTFrunSampleTable);
        _pTFrunSampleTable->push_back(pTFrunSampleTable);
        if (pTFrunSampleTable != NULL)
        {
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "Track Fragment Run=>Sample Number			=%d", idx));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "Track Fragment Run=>Sample Offset			=%d", sample_offset));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "Track Fragment Run=>Sample Sample Size		=%d", pTFrunSampleTable->_sample_size));
            sample_offset += pTFrunSampleTable->_sample_size;
            sigmaSampleSize += pTFrunSampleTable->_sample_size;
            _sampleTimeStamp += pTFrunSampleTable->_sample_duration;

        }
    }
    offset += sigmaSampleSize;

    if (trunParsingCompleted)
    {
        countOfTrunsParsed++;
    }
}

void TrackFragmentRunAtom::setDefaultDuration(uint32 default_duration)
{
    for (uint32 idx = 0; idx < _sample_count ;idx++)
    {
        TFrunSampleTable *pTFrunSampleTable = NULL;
        if (_pTFrunSampleTable != NULL)
        {
            pTFrunSampleTable = (*_pTFrunSampleTable)[idx];
            pTFrunSampleTable->setDefaultDuration(Oscl_Int64_Utils::get_uint64_lower32(_sampleTimeStamp), default_duration);
            _sampleTimeStamp += default_duration;
        }
    }

}
void TrackFragmentRunAtom::setSampleDurationAndTimeStampFromSampleNum(uint32 startSampleNum, uint32 startSampleTS, uint32 default_duration)
{
    _sampleTimeStamp = startSampleTS;
    for (uint32 idx = startSampleNum; idx < _sample_count ;idx++)
    {
        TFrunSampleTable *pTFrunSampleTable = NULL;
        if (_pTFrunSampleTable != NULL)
        {
            pTFrunSampleTable = (*_pTFrunSampleTable)[idx];
            if (pTFrunSampleTable->_sample_duration != 0)
            {
                default_duration = pTFrunSampleTable->_sample_duration;
            }
            pTFrunSampleTable->setDefaultDuration(Oscl_Int64_Utils::get_uint64_lower32(_sampleTimeStamp), default_duration);
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "Track Fragment Run=>Set Sample TS	>>>>>>>>>>>>=%d", pTFrunSampleTable->_sample_timestamp));
            _sampleTimeStamp += default_duration;
        }
    }

}
void TrackFragmentRunAtom::setDefaultSampleSize(uint32 default_samplesize, uint32 &sigmaSampleSize)
{
    uint32 sumSampleSize = 0;
    uint32 sample_offset = _trun_start_offset;
    for (uint32 idx = 0; idx < _sample_count ;idx++)
    {
        TFrunSampleTable *pTFrunSampleTable = NULL;
        if (_pTFrunSampleTable != NULL)
        {
            pTFrunSampleTable = (*_pTFrunSampleTable)[idx];
            pTFrunSampleTable->setDefaultSampleSize(sample_offset, default_samplesize);
            sample_offset += default_samplesize;
            sumSampleSize += default_samplesize;
        }

    }
    sigmaSampleSize = sumSampleSize;
}

