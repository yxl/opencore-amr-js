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
#define IMPLEMENT_TrackFragmentRandomAccessAtom

#include "trackfragmentrandomaccess.h"
#include "atomutils.h"
#include "atomdefs.h"

typedef Oscl_Vector<TFRAEntries*, OsclMemAllocator> TFRAEntriesVecType;

TFRAEntries::TFRAEntries(MP4_FF_FILE *fp , uint32 version,
                         uint8 length_size_of_traf_num,
                         uint8 length_size_of_trun_num,
                         uint8 length_size_of_sample_num)
{
    _version = version;
    _time64 = 0;
    _moof_offset64 = 0;
    _time32 = 0;
    _moof_offset32 = 0;
    _traf_number = 0;
    _trun_number = 0;
    _sample_number = 0;

    if (_version == 1)
    {
        if (!AtomUtils::read64(fp, _time64))
        {
            return;
        }
        if (!AtomUtils::read64(fp, _moof_offset64))
        {
            return;
        }
    }
    else
    {
        if (!AtomUtils::read32(fp, _time32))
        {
            return;
        }
        if (!AtomUtils::read32(fp, _moof_offset32))
        {
            return;
        }

    }
    switch (length_size_of_traf_num)
    {
        case 0:
        {
            uint8 tmp = 0;
            if (!AtomUtils::read8(fp, tmp))
            {
                return;
            }
            _traf_number = tmp;
        }
        break;
        case 1:
        {
            uint16 tmp = 0;
            if (!AtomUtils::read16(fp, tmp))
            {
                return;
            }
            _traf_number = tmp;
        }
        break;
        case 2:
            if (!AtomUtils::read24(fp, _traf_number))
            {
                return;
            }
            break;
        case 3:
            if (!AtomUtils::read32(fp, _traf_number))
            {
                return;
            }
            break;
    }

    switch (length_size_of_trun_num)
    {
        case 0:
        {
            uint8 tmp = 0;
            if (!AtomUtils::read8(fp, tmp))
            {
                return;
            }
            _trun_number = tmp;
        }
        break;
        case 1:
        {
            uint16 tmp = 0;
            if (!AtomUtils::read16(fp, tmp))
            {
                return;
            }
            _trun_number = tmp;
        }
        break;
        case 2:
            if (!AtomUtils::read24(fp, _trun_number))
            {
                return;
            }
            break;
        case 3:
            if (!AtomUtils::read32(fp, _trun_number))
            {
                return;
            }
            break;
    }

    switch (length_size_of_sample_num)
    {
        case 0:
        {
            uint8 tmp = 0;
            if (!AtomUtils::read8(fp, tmp))
            {
                return;
            }
            _sample_number = tmp;
        }
        break;
        case 1:
        {
            uint16 tmp = 0;
            if (!AtomUtils::read16(fp, tmp))
            {
                return;
            }
            _sample_number = tmp;
        }
        break;
        case 2:
            if (!AtomUtils::read24(fp, _sample_number))
            {
                return;
            }
            break;
        case 3:
            if (!AtomUtils::read32(fp, _sample_number))
            {
                return;
            }
            break;
    }

}

TrackFragmentRandomAccessAtom ::TrackFragmentRandomAccessAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    OSCL_UNUSED_ARG(type);
    _trackId = 0;
    _length_size_of_traf_num = 0;
    _length_size_of_trun_num = 0;
    _length_size_of_sample_num = 0;
    _entry_count = 0;
    _version = getVersion();

    iLogger = PVLogger::GetLoggerObject("mp4ffparser");
    iStateVarLogger = PVLogger::GetLoggerObject("mp4ffparser_mediasamplestats");
    iParsedDataLogger = PVLogger::GetLoggerObject("mp4ffparser_parseddata");

    if (_success)
    {
        PV_MP4_FF_NEW(fp->auditCB, TFRAEntriesVecType, (), _pTFRAEntriesVec);

        if (!AtomUtils::read32(fp, _trackId))
        {
            _success = false;
            _mp4ErrorCode = READ_TRACK_FRAGMENT_RANDOM_ACCESS_ATOM_FAILED;
            return;
        }

        if (!AtomUtils::read32(fp, _reserved))
        {
            _success = false;
            _mp4ErrorCode = READ_TRACK_FRAGMENT_RANDOM_ACCESS_ATOM_FAILED;
            return;
        }
        _length_size_of_traf_num = (_reserved & 0x00000003);
        _length_size_of_trun_num = ((_reserved >> 2) & 0x00000003);
        _length_size_of_sample_num = ((_reserved >> 4) & 0x00000003);

        if (!AtomUtils::read32(fp, _entry_count))
        {
            _success = false;
            _mp4ErrorCode = READ_TRACK_FRAGMENT_RANDOM_ACCESS_ATOM_FAILED;
            return;
        }
        for (uint32 idx = 0; idx < _entry_count ;idx++)
        {
            TFRAEntries *pTFRAEntries = NULL;
            PV_MP4_FF_NEW(fp->auditCB, TFRAEntries, (fp, _version, _length_size_of_traf_num,
                          _length_size_of_trun_num,
                          _length_size_of_sample_num),
                          pTFRAEntries);
            _pTFRAEntriesVec->push_back(pTFRAEntries);
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentRandom Access Point _sample_number[%d] =%d", idx, pTFRAEntries->_sample_number));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentRandom Access Point TimeStamp	 [%d] =%d", idx, pTFRAEntries->getTimeStamp()));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentRandom Access Point MoofOffset    [%d] =%d", idx, pTFRAEntries->getTimeMoofOffset()));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentRandom Access Point _traf_number	 [%d] =%d", idx, pTFRAEntries->_traf_number));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentRandom Access Point _trun_number  [%d] =%d", idx, pTFRAEntries->_trun_number));

        }

    }
}
// Destructor
TrackFragmentRandomAccessAtom::~TrackFragmentRandomAccessAtom()
{
    if (_pTFRAEntriesVec != NULL)
    {
        // CLEAN UP VECTOR!!
        for (uint32 i = 0; i < _pTFRAEntriesVec->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, TFRAEntries, (*_pTFRAEntriesVec)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, TFRAEntriesVecType, Oscl_Vector, _pTFRAEntriesVec);
    }

}


