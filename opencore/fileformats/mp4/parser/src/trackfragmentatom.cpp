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
/*                            MPEG-4 Track Fragment Atom Class                   */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
*/

#define IMPLEMENT_TrackFragmentAtom

#include "trackfragmentatom.h"
#include "trackfragmentheaderatom.h"
#include "trackfragmentrunatom.h"
#include "atomdefs.h"
#include "atomutils.h"
#include "oscl_int64_utils.h"

typedef Oscl_Vector<TrackFragmentRunAtom*, OsclMemAllocator> trackFragmentRunAtomVecType;
typedef Oscl_Vector<uint32, OsclMemAllocator> trackFragmentRunOffsetVecType;
typedef Oscl_Vector<uint32, OsclMemAllocator> fragmentptrOffsetVecType;

// Constructor
TrackFragmentAtom::TrackFragmentAtom(MP4_FF_FILE *fp,
                                     uint32 &size,
                                     uint32 type,
                                     uint32 movieFragmentCurrentOffset,
                                     uint32 movieFragmentBaseOffset,
                                     uint32 moofSize,
                                     TrackDurationContainer *trackDurationContainer,
                                     Oscl_Vector<TrackExtendsAtom*, OsclMemAllocator> *trackExtendAtomVec,
                                     bool &parseTrafCompletely,
                                     bool &trafParsingCompleted,
                                     uint32 &countOfTrunsParsed)
        : Atom(fp, size, type)
{
    OSCL_UNUSED_ARG(movieFragmentCurrentOffset);

    _pTrackFragmentHeaderAtom		= NULL;
    _pTrackFragmentRunAtom			= NULL;
    _pinput = NULL;
    _commonFilePtr = NULL;
    _fileSize = 0;
    _currentTrackFragmentRunSampleNumber = 0;
    _currentPlaybackSampleTimestamp = 0;
    _movieFragmentOffset = 0;
    _prevSampleOffset = 0;
    _trackEndDuration = 0;
    _startTrackFragmentTSOffset = 0;
    _pFragmentptrOffsetVec = NULL;
    _peekPlaybackSampleNumber = 0;
    _default_duration = 0;
    _use_default_duratoin = false;
    _pTrackDurationContainer = trackDurationContainer;

    tf_flags = 0;
    trun_offset = 0;

    trunParsingCompleted = true;

    iLogger = PVLogger::GetLoggerObject("mp4ffparser_traf");
    iStateVarLogger = PVLogger::GetLoggerObject("mp4ffparser_mediasamplestats_traf");
    iParsedDataLogger = PVLogger::GetLoggerObject("mp4ffparser_parseddata_traf");

    OsclAny*ptr = oscl_malloc(sizeof(MP4_FF_FILE));
    if (ptr == NULL)
    {
        _success = false;
        _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
        return;
    }
    _pinput = OSCL_PLACEMENT_NEW(ptr, MP4_FF_FILE(*fp));

    _pinput->_fileServSession = fp->_fileServSession;
    _pinput->_pvfile.SetCPM(fp->_pvfile.GetCPM());
    _pinput->_fileSize = fp->_fileSize;
    _pinput->_pvfile.Copy(fp->_pvfile);

    uint32 trun_start = 0;
    uint32 count = size - DEFAULT_ATOM_SIZE;

    uint32 _movieFragmentBaseOffset = movieFragmentBaseOffset - DEFAULT_ATOM_SIZE;
    bool bdo_present = false;
    uint32 base_data_offset = _movieFragmentBaseOffset;
    trackId = 0;
    trun_offset = moofSize + DEFAULT_ATOM_SIZE;

    if (_success)
    {
        PV_MP4_FF_NEW(fp->auditCB, trackFragmentRunAtomVecType, (), _pTrackFragmentRunAtomVec);
        while (count > 0)
        {
            uint32 atomType = UNKNOWN_ATOM;
            uint32 atomSize = 0;
            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            if (atomType == TRACK_FRAGMENT_HEADER_ATOM)
            {
                if (_pTrackFragmentHeaderAtom == NULL)
                {
                    PV_MP4_FF_NEW(fp->auditCB, TrackFragmentHeaderAtom, (fp, atomSize, atomType), _pTrackFragmentHeaderAtom);
                    if (!_pTrackFragmentHeaderAtom->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = READ_MOVIE_EXTENDS_HEADER_FAILED;
                        return;
                    }
                    count -= _pTrackFragmentHeaderAtom->getSize();
                    trackId = _pTrackFragmentHeaderAtom->getTrackId();
                    tf_flags = _pTrackFragmentHeaderAtom->getFlags();
                    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "@@@@@@@@@@@@@@@****** Track ID= %d ********@@@@@@@@@@@@@@@@", trackId));
                    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "#### tf_flag =0x%x######", tf_flags));

                    if (tf_flags & 0x000001)
                    {
                        uint64 bdo = _pTrackFragmentHeaderAtom->getBaseDataOffset();
                        base_data_offset = Oscl_Int64_Utils::get_uint64_lower32(bdo);
                        bdo_present = true;
                    }
                    else
                        base_data_offset = _movieFragmentBaseOffset;

                    trun_start = base_data_offset;
                    if (trackDurationContainer != NULL)
                    {
                        for (int32 i = 0; i < trackDurationContainer->getNumTrackInfoVec();i++)
                        {
                            TrackDurationInfo* trackInfo = trackDurationContainer->getTrackdurationInfoAt(i);
                            if (trackInfo->trackId == trackId)
                            {
                                _trackEndDuration = trackInfo->trackDuration;
                                _startTrackFragmentTSOffset = Oscl_Int64_Utils::get_uint64_lower32(_trackEndDuration);
                            }
                        }
                    }
                }
                else
                {
                    //duplicate atom
                    count -= atomSize;
                    atomSize -= DEFAULT_ATOM_SIZE;
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                }
            }
            else if (atomType == TRACK_FRAGMENT_RUN_ATOM)
            {
                uint32 trunsParsed = countOfTrunsParsed;
                if (countOfTrunsParsed > COUNT_OF_TRUNS_PARSED_THRESHOLD)
                {
                    countOfTrunsParsed = COUNT_OF_TRUNS_PARSED_THRESHOLD;
                }
                // here we want parser to parse complete TRUN atom.

                PV_MP4_FF_NEW(fp->auditCB, TrackFragmentRunAtom, (fp, atomSize, atomType,
                              base_data_offset,
                              trun_start,
                              trun_offset,
                              _trackEndDuration,
                              bdo_present,
                              trunParsingCompleted,
                              countOfTrunsParsed),
                              _pTrackFragmentRunAtom);

                countOfTrunsParsed = trunsParsed + 1;

                if (!_pTrackFragmentRunAtom->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = READ_TRACK_EXTENDS_ATOM_FAILED;
                    return;
                }
                bdo_present = false;
                count -= _pTrackFragmentRunAtom->getSize();

                size = count;
                uint32 trunFlags = _pTrackFragmentRunAtom->getFlags();
                if (!(trunFlags & 0x000100))
                {
                    _use_default_duratoin = true;
                    if (tf_flags & 0x000008)
                    {
                        _default_duration = _pTrackFragmentHeaderAtom->getDefaultSampleDuration();
                        _pTrackFragmentRunAtom->setDefaultDuration(_default_duration);
                    }
                    else
                    {
                        for (uint32 idx = 0; idx < trackExtendAtomVec->size();idx++)
                        {
                            TrackExtendsAtom* pTrackExtendAtom = (*trackExtendAtomVec)[idx];
                            uint32 id = pTrackExtendAtom->getTrackId();
                            if (id == trackId)
                            {
                                uint32 trexDefaultDuration = pTrackExtendAtom->getDefaultSampleDuration();
                                _default_duration = trexDefaultDuration;
                                _pTrackFragmentRunAtom->setDefaultDuration(trexDefaultDuration);
                            }
                        }
                    }
                }
                if (!(trunFlags & 0x000200))
                {
                    if (tf_flags & 0x000010)
                        _pTrackFragmentRunAtom->setDefaultSampleSize(_pTrackFragmentHeaderAtom->getDefaultSampleSize(), trun_offset);
                    else
                    {
                        for (uint32 idx = 0; idx < trackExtendAtomVec->size();idx++)
                        {
                            TrackExtendsAtom* pTrackExtendAtom = (*trackExtendAtomVec)[idx];
                            uint32 id = pTrackExtendAtom->getTrackId();
                            if (id == trackId)
                            {
                                uint32 trexDefaultSampleSize = pTrackExtendAtom->getDefaultSampleSize();
                                _pTrackFragmentRunAtom->setDefaultSampleSize(trexDefaultSampleSize, trun_offset);
                            }
                        }
                    }
                }
                _pTrackFragmentRunAtomVec->push_back(_pTrackFragmentRunAtom);
                _trackEndDuration = _pTrackFragmentRunAtom->_sampleTimeStamp;

                PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "****** tr_flag =0x%x ********", trunFlags));
                if (!parseTrafCompletely)
                {
                    trafParsingCompleted = false;
                    uint32 duration = Oscl_Int64_Utils::get_uint64_lower32(_trackEndDuration);
                    trackDurationContainer->updateTrackDurationForTrackId(trackId, duration);
                    break;
                }
            }
            uint32 track_duration = Oscl_Int64_Utils::get_uint64_lower32(_trackEndDuration);
            trackDurationContainer->updateTrackDurationForTrackId(trackId, track_duration);
            trafParsingCompleted = true;
        }
    }
    else
    {
        _mp4ErrorCode = READ_MOVIE_EXTENDS_ATOM_FAILED;
    }
}

void TrackFragmentAtom::ParseTrafAtom(MP4_FF_FILE *fp,
                                      uint32 &size,
                                      uint32 type,
                                      uint32 movieFragmentCurrentOffset,
                                      uint32 movieFragmentBaseOffset,
                                      uint32 moofSize,
                                      TrackDurationContainer *trackDurationContainer,
                                      Oscl_Vector<TrackExtendsAtom*, OsclMemAllocator> *trackExtendAtomVec,
                                      bool &trafParsingCompleted,
                                      uint32 &countOfTrunsParsed)
{
    OSCL_UNUSED_ARG(type);
    OSCL_UNUSED_ARG(movieFragmentCurrentOffset);
    OSCL_UNUSED_ARG(moofSize);
    uint32 count = size;
    uint32 trun_start = 0;
    uint32 _movieFragmentBaseOffset = movieFragmentBaseOffset - DEFAULT_ATOM_SIZE;
    bool bdo_present = false;
    uint32 base_data_offset = _movieFragmentBaseOffset;

    if (tf_flags & 0x000001)
    {
        base_data_offset = Oscl_Int64_Utils::get_uint64_lower32(_pTrackFragmentHeaderAtom->getBaseDataOffset());
        bdo_present = true;
    }
    else
        base_data_offset = _movieFragmentBaseOffset;

    trun_start = base_data_offset;

    if (_success)
    {
        for (uint32 i = 0;i < 1;i++)
        {
            if (count > 0)
            {
                if (trunParsingCompleted)
                {
                    uint32 atomType = UNKNOWN_ATOM;
                    uint32 atomSize = 0;
                    AtomUtils::getNextAtomType(fp, atomSize, atomType);

                    if (atomType == TRACK_FRAGMENT_RUN_ATOM)
                    {
                        PV_MP4_FF_NEW(fp->auditCB, TrackFragmentRunAtom, (fp, atomSize, atomType,
                                      base_data_offset,
                                      trun_start,
                                      trun_offset,
                                      _trackEndDuration,
                                      bdo_present,
                                      trunParsingCompleted,
                                      countOfTrunsParsed),
                                      _pTrackFragmentRunAtom);

                        if (!_pTrackFragmentRunAtom->MP4Success())
                        {
                            _success = false;
                            _mp4ErrorCode = READ_TRACK_EXTENDS_ATOM_FAILED;
                            return;
                        }
                        _pTrackFragmentRunAtomVec->push_back(_pTrackFragmentRunAtom);
                        if (trunParsingCompleted)
                        {
                            bdo_present = false;
                            count -= _pTrackFragmentRunAtom->getSize();
                            size = count;
                            PVMF_MP4FFPARSER_LOGPARSEDINFO((0, "count %d", count));
                            uint32 trunFlags = _pTrackFragmentRunAtom->getFlags();
                            if (!(trunFlags & 0x000100))
                            {
                                _use_default_duratoin = true;
                                if (tf_flags & 0x000008)
                                {
                                    _default_duration = _pTrackFragmentHeaderAtom->getDefaultSampleDuration();
                                    _pTrackFragmentRunAtom->setDefaultDuration(_default_duration);
                                }
                                else
                                {
                                    for (uint32 idx = 0; idx < trackExtendAtomVec->size();idx++)
                                    {
                                        TrackExtendsAtom* pTrackExtendAtom = (*trackExtendAtomVec)[idx];
                                        uint32 id = pTrackExtendAtom->getTrackId();
                                        if (id == trackId)
                                        {
                                            uint32 trexDefaultDuration = pTrackExtendAtom->getDefaultSampleDuration();
                                            _default_duration = trexDefaultDuration;
                                            _pTrackFragmentRunAtom->setDefaultDuration(trexDefaultDuration);
                                        }
                                    }
                                }
                            }
                            if (!(trunFlags & 0x000200))
                            {
                                if (tf_flags & 0x000010)
                                    _pTrackFragmentRunAtom->setDefaultSampleSize(_pTrackFragmentHeaderAtom->getDefaultSampleSize(), trun_offset);
                                else
                                {
                                    for (uint32 idx = 0; idx < trackExtendAtomVec->size();idx++)
                                    {
                                        TrackExtendsAtom* pTrackExtendAtom = (*trackExtendAtomVec)[idx];
                                        uint32 id = pTrackExtendAtom->getTrackId();
                                        if (id == trackId)
                                        {
                                            uint32 trexDefaultSampleSize = pTrackExtendAtom->getDefaultSampleSize();
                                            _pTrackFragmentRunAtom->setDefaultSampleSize(trexDefaultSampleSize, trun_offset);
                                        }
                                    }
                                }
                            }
                            _trackEndDuration = _pTrackFragmentRunAtom->_sampleTimeStamp;

                            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "****** tr_flag =0x%x ********", trunFlags));
                        }
                        trackDurationContainer->updateTrackDurationForTrackId(trackId,
                                Oscl_Int64_Utils::get_uint64_lower32(_trackEndDuration));
                    }
                    trafParsingCompleted = false;
                }
                else
                {
                    _pTrackFragmentRunAtom->ParseTrunAtom(fp,
                                                          trun_offset,
                                                          trunParsingCompleted,
                                                          countOfTrunsParsed);

                    if (!_pTrackFragmentRunAtom->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = READ_TRACK_EXTENDS_ATOM_FAILED;
                        return;
                    }

                    if (trunParsingCompleted)
                    {
                        bdo_present = false;
                        count -= _pTrackFragmentRunAtom->getSize();
                        size = count;
                        PVMF_MP4FFPARSER_LOGPARSEDINFO((0, "count %d", count));
                        uint32 trunFlags = _pTrackFragmentRunAtom->getFlags();
                        if (!(trunFlags & 0x000100))
                        {
                            _use_default_duratoin = true;
                            if (tf_flags & 0x000008)
                            {
                                _default_duration = _pTrackFragmentHeaderAtom->getDefaultSampleDuration();
                                _pTrackFragmentRunAtom->setDefaultDuration(_default_duration);
                            }
                            else
                            {
                                for (uint32 idx = 0; idx < trackExtendAtomVec->size();idx++)
                                {
                                    TrackExtendsAtom* pTrackExtendAtom = (*trackExtendAtomVec)[idx];
                                    uint32 id = pTrackExtendAtom->getTrackId();
                                    if (id == trackId)
                                    {
                                        uint32 trexDefaultDuration = pTrackExtendAtom->getDefaultSampleDuration();
                                        _default_duration = trexDefaultDuration;
                                        _pTrackFragmentRunAtom->setDefaultDuration(trexDefaultDuration);
                                    }
                                }
                            }
                        }
                        if (!(trunFlags & 0x000200))
                        {
                            if (tf_flags & 0x000010)
                                _pTrackFragmentRunAtom->setDefaultSampleSize(_pTrackFragmentHeaderAtom->getDefaultSampleSize(), trun_offset);
                            else
                            {
                                for (uint32 idx = 0; idx < trackExtendAtomVec->size();idx++)
                                {
                                    TrackExtendsAtom* pTrackExtendAtom = (*trackExtendAtomVec)[idx];
                                    uint32 id = pTrackExtendAtom->getTrackId();
                                    if (id == trackId)
                                    {
                                        uint32 trexDefaultSampleSize = pTrackExtendAtom->getDefaultSampleSize();
                                        _pTrackFragmentRunAtom->setDefaultSampleSize(trexDefaultSampleSize, trun_offset);
                                    }
                                }
                            }
                        }
                        _trackEndDuration = _pTrackFragmentRunAtom->_sampleTimeStamp;

                        PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "****** tr_flag =0x%x ********", trunFlags));
                    }

                    trackDurationContainer->updateTrackDurationForTrackId(trackId,
                            Oscl_Int64_Utils::get_uint64_lower32(_trackEndDuration));
                    trafParsingCompleted = false;
                }
            }
            else if (count == 0)
            {
                PVMF_MP4FFPARSER_LOGPARSEDINFO((0, "TRAF END count %d", count));
                trafParsingCompleted = true;
                break;
            }
            else
            {
                break;
            }
        }
        if (count == 0)
        {
            PVMF_MP4FFPARSER_LOGPARSEDINFO((0, "TRAF END count %d", count));
            trafParsingCompleted = true;
        }
    }
    else
    {
        _mp4ErrorCode = READ_MOVIE_EXTENDS_ATOM_FAILED;
    }
}

TrackFragmentAtom::~TrackFragmentAtom()
{
    if (_pTrackFragmentHeaderAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, TrackFragmentHeaderAtom, _pTrackFragmentHeaderAtom);
    }

    for (uint32 i = 0; i < _pTrackFragmentRunAtomVec->size(); i++)
    {
        PV_MP4_FF_DELETE(NULL, TrackFragmentRunAtom, (*_pTrackFragmentRunAtomVec)[i]);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, trackFragmentRunAtomVecType, Oscl_Vector, _pTrackFragmentRunAtomVec);

    if (_pinput != NULL)
    {
        oscl_free(_pinput);
    }
}

int32
TrackFragmentAtom::getNextNSamples(uint32 startSampleNum,
                                   uint32 *n, uint32 totalSampleRead,
                                   GAU    *pgau)
{
    uint32  numSamplesInCurrentTrackFragmentRun = 0;
    int32 currTSBase = 0;
    uint32 samplesLeftInChunk = 0;
    uint32 numSamples = 0;
    _startTrackFragmentTSOffset = 0;

    uint32 samplesYetToBeRead = *n;
    uint32 samplesReadBefore;
    samplesReadBefore = *n;
    uint32 sampleNum = startSampleNum;
    uint32 sampleFileOffset = 0;

    int32 sampleBeforeGet = (int32)startSampleNum;

    uint32 s = 0;
    uint32 end = 0;

    uint32 i = 0, k = 0;

    int32  _mp4ErrorCode = EVERYTHING_FINE;

    currTSBase = _currentPlaybackSampleTimestamp;
    end = pgau->buf.num_fragments;

    uint32 start = 0;
    uint32 gauIdx = 0;
    uint32 frgptr = 0;
    samplesReadBefore = totalSampleRead;
    uint32 totalnumSamples = 0;

    totalnumSamples = getTotalNumSampleInTraf();
    if (sampleNum >= totalnumSamples)
    {
        _currentTrackFragmentRunSampleNumber = 0;
        *n = 0;
        _mp4ErrorCode = END_OF_TRACK;
        return (_mp4ErrorCode);
    }

    PV_MP4_FF_NEW(fp->auditCB, fragmentptrOffsetVecType, (), _pFragmentptrOffsetVec);
    for (i = 0; i < samplesReadBefore; i++)
    {
        frgptr +=  pgau->info[i].len;
        if (pgau->info[i].len != 0)
        {
            gauIdx++;
        }
    }
    s = samplesReadBefore;
    for (k = start; k < end; k++)
    {
        _pFragmentptrOffsetVec->push_back(frgptr);
    }

    GAU tempGau;
    tempGau = *pgau;
    GAU* tempgauPtr = &tempGau;

    k = 0;

    while (samplesYetToBeRead)
    {
        TrackFragmentRunAtom *tfRun;
        uint32 sampleCount;
        if (_mp4ErrorCode == END_OF_TRACK)
        {
            break;
        }
        tfRun = getTrackFragmentRunForSampleNum(_currentTrackFragmentRunSampleNumber, sampleCount);
        if (tfRun != NULL)
        {
            numSamplesInCurrentTrackFragmentRun = tfRun->getSampleCount();
        }
        else
        {
            *n = 0;
            _mp4ErrorCode = END_OF_TRACK;
            PV_MP4_FF_TEMPLATED_DELETE(NULL, fragmentptrOffsetVecType,
                                       Oscl_Vector, _pFragmentptrOffsetVec);
            return (_mp4ErrorCode);
        }

        int32 tfrunoffset = 0;
        tfrunoffset = Oscl_Int64_Utils::get_uint64_lower32(tfRun->getDataOffset());
        int32 sampleSizeOffset = 0;

        uint32 sigmaSampleSize = 0, k = 0;
        uint32 debugOffset = _prevSampleOffset + sampleSizeOffset;

        samplesLeftInChunk = ((sampleCount - _currentTrackFragmentRunSampleNumber));

        if (samplesLeftInChunk  >=  samplesYetToBeRead)
        {
            numSamples = samplesYetToBeRead;
            samplesYetToBeRead = 0;
        }
        else
        {
            samplesYetToBeRead -= samplesLeftInChunk;
            numSamples = samplesLeftInChunk;
        }

        uint32 StartReadingFromSampleNum = numSamplesInCurrentTrackFragmentRun - samplesLeftInChunk;
        uint32 sampleLeft = (numSamples + StartReadingFromSampleNum);

        uint32 baseSampleNum  = StartReadingFromSampleNum;
        while (StartReadingFromSampleNum < sampleLeft)
        {
            if (StartReadingFromSampleNum >= numSamplesInCurrentTrackFragmentRun)
            {
                if (sampleCount == totalnumSamples)
                {
                    samplesYetToBeRead = sampleLeft - StartReadingFromSampleNum;
                    _mp4ErrorCode = END_OF_TRACK;
                    break;
                }
                tfRun = getTrackFragmentRunForSampleNum(StartReadingFromSampleNum, sampleCount);
                if (tfRun != NULL)
                {
                    numSamplesInCurrentTrackFragmentRun = tfRun->getSampleCount();
                }
                else
                {
                    *n = 0;
                    _mp4ErrorCode = END_OF_TRACK;
                    PV_MP4_FF_TEMPLATED_DELETE(NULL, fragmentptrOffsetVecType,
                                               Oscl_Vector, _pFragmentptrOffsetVec);
                    return (_mp4ErrorCode);
                }

                sampleLeft = sampleLeft - StartReadingFromSampleNum;
                StartReadingFromSampleNum = 0;
                baseSampleNum = 0;
            }
            k = StartReadingFromSampleNum;

            int32 tsDelta = 0;
            int32 tempSize = 0;
            Oscl_Vector<TFrunSampleTable*, OsclMemAllocator>* _tfRunSampleInfo = tfRun->getSampleTable();
            if (_tfRunSampleInfo != NULL)
            {


                currTSBase =  Oscl_Int64_Utils::get_uint64_lower32((*_tfRunSampleInfo)[k]->_sample_timestamp);
                tempSize = (*_tfRunSampleInfo)[k]->_sample_size;
                tsDelta = (*_tfRunSampleInfo)[k]->_sample_duration;

            }
            if (_tfRunSampleInfo == NULL || tempSize == -1)  // doesnt seem like one can continue if no _tfRunSampleInfo
            {
                *n = 0;
                _mp4ErrorCode =  INVALID_SAMPLE_SIZE;
                PV_MP4_FF_TEMPLATED_DELETE(NULL, fragmentptrOffsetVecType,
                                           Oscl_Vector, _pFragmentptrOffsetVec);

                return (_mp4ErrorCode);
            }
            sigmaSampleSize += tempSize;
            pgau->info[s].len = tempSize;
            sampleFileOffset = (*_tfRunSampleInfo)[baseSampleNum]->_sample_offset;
            pgau->info[s].ts_delta = tsDelta;
            pgau->info[s].ts = currTSBase;
            currTSBase += tsDelta;
            debugOffset = sampleFileOffset;

            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentAtom::getNextNSamples- Track Fragment Run Offset[%d] =%d", s, tfrunoffset));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentAtom::getNextNSamples- pgau->info[%d].len =%d", s, pgau->info[s].len));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentAtom::getNextNSamples- pgau->info[%d].ts_delta =%d", s, pgau->info[s].ts_delta));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentAtom::getNextNSamples- pgau->info[%d].ts =%d", s, pgau->info[s].ts));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentAtom::getNextNSamples- Offset =%d", debugOffset));

            s++;
            StartReadingFromSampleNum++;
        }



        Oscl_Int64_Utils::set_uint64(pgau->SampleOffset, 0, (uint32)sampleFileOffset);

        AtomUtils::getCurrentFileSize(_pinput, _fileSize);
        if ((sampleFileOffset + sigmaSampleSize) > _fileSize)
        {
            _mp4ErrorCode = INSUFFICIENT_DATA;
            _currentTrackFragmentRunSampleNumber = startSampleNum;
            *n = 0;
            for (uint32 i = 0; i < pgau->numMediaSamples; i++)
            {
                pgau->info[i].len         = 0;
                pgau->info[i].ts          = 0;
                pgau->info[i].sample_info = 0;
            }
            PV_MP4_FF_TEMPLATED_DELETE(NULL, fragmentptrOffsetVecType,
                                       Oscl_Vector, _pFragmentptrOffsetVec);

            return (_mp4ErrorCode);
        }

        if (sampleFileOffset != 0)
            AtomUtils::seekFromStart(_pinput, sampleFileOffset);

        for (k = start; k < end; k++)
        {
            uint8* read_fragment_ptr = NULL;
            read_fragment_ptr = (uint8 *)(tempgauPtr->buf.fragments[k].ptr);
            read_fragment_ptr += (*_pFragmentptrOffsetVec)[k];
            (*_pFragmentptrOffsetVec)[k] = 0;
            uint32 tmpSize =
                (tempgauPtr->buf.fragments[k].len > sigmaSampleSize) ? sigmaSampleSize : tempgauPtr->buf.fragments[k].len;
            if (tmpSize)
            {
                if (!AtomUtils::readByteData(_pinput, tmpSize,
                                             (uint8 *)(read_fragment_ptr)))
                {
                    *n = 0;

                    _mp4ErrorCode =  READ_FAILED;
                    PV_MP4_FF_TEMPLATED_DELETE(NULL, fragmentptrOffsetVecType,
                                               Oscl_Vector, _pFragmentptrOffsetVec);


                    return (_mp4ErrorCode);
                }
                tempgauPtr->buf.fragments[k].len -= tmpSize;
                read_fragment_ptr += tmpSize;
                tempgauPtr->buf.fragments[k].ptr = read_fragment_ptr;

                sigmaSampleSize -= tmpSize;

            }

            if (sigmaSampleSize == 0)
            {
                break;
            }
        }

        if (sigmaSampleSize > 0)
        {
            _currentTrackFragmentRunSampleNumber = startSampleNum;
            _currentPlaybackSampleTimestamp = _startTrackFragmentTSOffset;
            _mp4ErrorCode =  INSUFFICIENT_BUFFER_SIZE;
            break;
        }
        sampleNum = sampleNum + numSamples;

        if (_currentTrackFragmentRunSampleNumber == (uint32)sampleBeforeGet)
        {
            _currentTrackFragmentRunSampleNumber += numSamples;
            sampleBeforeGet += numSamples;
        }
        else
        {
            break;
        }

        if (_currentTrackFragmentRunSampleNumber == totalnumSamples)
        {
            break;
        }

        if (_currentTrackFragmentRunSampleNumber > totalnumSamples)
        {
            _mp4ErrorCode = END_OF_TRACK;
            break;
        }
    }

    if (_currentTrackFragmentRunSampleNumber == (uint32)sampleBeforeGet)
    {
        _currentPlaybackSampleTimestamp = currTSBase;
        *n = (*n - samplesYetToBeRead);
    }
    else
    {
        _currentPlaybackSampleTimestamp = _startTrackFragmentTSOffset;
        *n = 0;
        _mp4ErrorCode = READ_FAILED;
    }


    PV_MP4_FF_TEMPLATED_DELETE(NULL, fragmentptrOffsetVecType,
                               Oscl_Vector, _pFragmentptrOffsetVec);

    return (_mp4ErrorCode);

}

TrackFragmentRunAtom *
TrackFragmentAtom::getTrackFragmentRunForSampleNum(uint32 samplenum, uint32 &sampleCount)
{
    if (_pTrackFragmentRunAtomVec != NULL)
    {
        uint32 samplecount = 0;
        uint32 numTrackFragment =  _pTrackFragmentRunAtomVec->size();
        for (uint32 idx = 0; idx < numTrackFragment; idx++)
        {
            samplecount += (*_pTrackFragmentRunAtomVec)[idx]->getSampleCount();
            if (samplenum < samplecount)
            {
                sampleCount = samplecount;
                return (*_pTrackFragmentRunAtomVec)[idx];
            }
        }

    }
    return NULL;
}
uint32
TrackFragmentAtom::getSampleNumberFromTimestamp(uint32 time)
{
    if (_pTrackFragmentRunAtomVec != NULL)
    {
        uint32 samplecount = 0;
        uint32 samplenum = 0;
        uint32 numTrackFragment =  _pTrackFragmentRunAtomVec->size();

        for (uint32 idx = 0; idx < numTrackFragment; idx++)
        {
            TrackFragmentRunAtom *tfrun = (*_pTrackFragmentRunAtomVec)[idx];
            Oscl_Vector<TFrunSampleTable*, OsclMemAllocator>* trackFragmentRunSampleInfo = tfrun->getSampleTable();
            samplecount = (*_pTrackFragmentRunAtomVec)[idx]->getSampleCount();
            for (uint32 idy = 0; idy < samplecount; idy++)
            {
                if (time >= (uint32)(*trackFragmentRunSampleInfo)[idy]->_sample_timestamp)
                {
                    return samplenum;
                }
                samplenum++;
            }
        }

    }
    return 0;
}

uint32
TrackFragmentAtom::getTimestampForSampleNumber(uint32 sampleNumber)
{
    if (_pTrackFragmentRunAtomVec != NULL)
    {
        uint32 samplecount = 0;
        uint32 numTrackFragment =  _pTrackFragmentRunAtomVec->size();

        for (uint32 idx = 0; idx < numTrackFragment; idx++)
        {
            TrackFragmentRunAtom *tfrun = (*_pTrackFragmentRunAtomVec)[idx];
            Oscl_Vector<TFrunSampleTable*, OsclMemAllocator>* trackFragmentRunSampleInfo = tfrun->getSampleTable();
            samplecount = (*_pTrackFragmentRunAtomVec)[idx]->getSampleCount();
            for (uint32 idy = 0; idy < samplecount; idy++)
            {
                if (sampleNumber == idy + 1)
                {
                    return Oscl_Int64_Utils::get_uint64_lower32(
                               (*trackFragmentRunSampleInfo)[idy]->_sample_timestamp);
                }
            }
        }

    }
    return 0;
}

int32
TrackFragmentAtom::getNextBundledAccessUnits(uint32 *n, uint32 totalSampleRead,
        GAU    *pgau)
{
    int32 nReturn = -1;

    if (_currentTrackFragmentRunSampleNumber == 0)
    {
        _currentPlaybackSampleTimestamp =  _startTrackFragmentTSOffset;
    }
    nReturn = getNextNSamples(_currentTrackFragmentRunSampleNumber, n, totalSampleRead, pgau);
    return nReturn;
}

uint64 TrackFragmentAtom::getBaseDataOffset()
{
    if (_pTrackFragmentHeaderAtom != NULL)
    {
        return _pTrackFragmentHeaderAtom->getBaseDataOffset();;
    }
    return 0;
}

uint32 TrackFragmentAtom::getSampleCount()
{
    return 0;
}
Oscl_Vector<TFrunSampleTable*, OsclMemAllocator>* TrackFragmentAtom::getSampleTable()
{
    return NULL;
}

uint32 TrackFragmentAtom::getSampleDescriptionIndex()
{
    if (_pTrackFragmentHeaderAtom != NULL)
    {
        return _pTrackFragmentHeaderAtom->getSampleDescriptionIndex();
    }
    return 0;
}
uint32 TrackFragmentAtom::getDefaultSampleDuration()
{
    if (_pTrackFragmentHeaderAtom != NULL)
    {
        return _pTrackFragmentHeaderAtom->getDefaultSampleDuration();
    }
    return 0;
}
uint32 TrackFragmentAtom::getDefaultSampleSize()
{
    if (_pTrackFragmentHeaderAtom != NULL)
    {
        return _pTrackFragmentHeaderAtom->getDefaultSampleSize();
    }
    return 0;
}
uint32 TrackFragmentAtom::getDefaultSampleFlags()
{
    if (_pTrackFragmentHeaderAtom != NULL)
    {
        return _pTrackFragmentHeaderAtom->getDefaultSampleFlag();
    }
    return 0;
}

uint32 TrackFragmentAtom::getTotalNumSampleInTraf()
{
    uint32 totalSampleNum = 0;
    if (_pTrackFragmentRunAtomVec != NULL)
    {
        uint32 numTrackFragment =  _pTrackFragmentRunAtomVec->size();
        for (uint32 idx = 0; idx < numTrackFragment; idx++)
        {
            uint32 samplecount = (*_pTrackFragmentRunAtomVec)[idx]->getSampleCount();
            totalSampleNum += samplecount;
        }
    }
    return totalSampleNum;
}


int32
TrackFragmentAtom::peekNextNSamples(uint32 startSampleNum,
                                    uint32 *n, uint32 totalSampleRead,
                                    MediaMetaInfo *mInfo)
{
    uint32  numSamplesInCurrentTrackFragmentRun = 0;
    int32 currTSBase = 0;
    uint32 samplesLeftInChunk = 0;
    uint32 numSamples = 0;
    _startTrackFragmentTSOffset = 0;

    uint32 samplesToBePeek = *n;
    uint32 sampleNum = startSampleNum;
    int32 sampleFileOffset = 0;
    int32 sampleBeforeGet = (int32)startSampleNum;
    uint32 s = totalSampleRead;

    int32  _mp4ErrorCode = EVERYTHING_FINE;
    currTSBase = _currentPlaybackSampleTimestamp;

    uint32 totalnumSamples = 0;
    totalnumSamples = getTotalNumSampleInTraf();

    if (sampleNum >= totalnumSamples)
    {
        _peekPlaybackSampleNumber = 0;
        *n = 0;
        _mp4ErrorCode = END_OF_TRACK;
        return (_mp4ErrorCode);
    }

    while (samplesToBePeek)
    {
        TrackFragmentRunAtom *tfRun;
        uint32 sampleCount;
        if (_mp4ErrorCode == END_OF_TRACK)
        {
            break;
        }
        tfRun = getTrackFragmentRunForSampleNum(_peekPlaybackSampleNumber, sampleCount);
        if (tfRun != NULL)
        {
            numSamplesInCurrentTrackFragmentRun = tfRun->getSampleCount();
        }
        else
        {
            *n = 0;
            _mp4ErrorCode = END_OF_TRACK;
            return (_mp4ErrorCode);
        }

        int32 tfrunoffset = 0;
        tfrunoffset = Oscl_Int64_Utils::get_uint64_lower32(tfRun->getDataOffset());
        int32 sampleSizeOffset = 0;

        uint32 sigmaSampleSize = 0, k = 0;
        uint32 debugOffset = _prevSampleOffset + sampleSizeOffset;

        samplesLeftInChunk = ((sampleCount - _peekPlaybackSampleNumber));

        if (samplesLeftInChunk  >=  samplesToBePeek)
        {
            numSamples = samplesToBePeek;
            samplesToBePeek = 0;
        }
        else
        {
            samplesToBePeek -= samplesLeftInChunk;
            numSamples = samplesLeftInChunk;
        }

        uint32 StartPeekFromSampleNum = numSamplesInCurrentTrackFragmentRun - samplesLeftInChunk;
        uint32 sampleLeft = (numSamples + StartPeekFromSampleNum);

        uint32 baseSampleNum  = StartPeekFromSampleNum;
        while (StartPeekFromSampleNum < sampleLeft)
        {
            if (StartPeekFromSampleNum >= numSamplesInCurrentTrackFragmentRun)
            {
                if (sampleCount == totalnumSamples)
                {
                    samplesToBePeek = sampleLeft - StartPeekFromSampleNum;
                    _mp4ErrorCode = END_OF_TRACK;
                    break;
                }
                tfRun = getTrackFragmentRunForSampleNum(StartPeekFromSampleNum, sampleCount);
                if (tfRun != NULL)
                {
                    numSamplesInCurrentTrackFragmentRun = tfRun->getSampleCount();
                }
                else
                {
                    *n = 0;
                    _mp4ErrorCode = END_OF_TRACK;
                    return (_mp4ErrorCode);
                }

                sampleLeft = sampleLeft - StartPeekFromSampleNum;
                StartPeekFromSampleNum = 0;
                baseSampleNum = 0;
            }
            k = StartPeekFromSampleNum;

            int32 tsDelta = 0;
            int32 tempSize = 0;
            Oscl_Vector<TFrunSampleTable*, OsclMemAllocator>* _tfRunSampleInfo = tfRun->getSampleTable();
            if (_tfRunSampleInfo != NULL)
            {
                currTSBase =  Oscl_Int64_Utils::get_uint64_lower32((*_tfRunSampleInfo)[k]->_sample_timestamp);
                tempSize = (*_tfRunSampleInfo)[k]->_sample_size;
                tsDelta = (*_tfRunSampleInfo)[k]->_sample_duration;
            }
            if (tempSize == 0)
                tempSize = getDefaultSampleSize();

            if (tsDelta == 0)
                tsDelta = getDefaultSampleDuration();

            if (_tfRunSampleInfo == NULL || tempSize == -1)  // doesn't seem like one can continue if no _tfRunSampleInfo
            {
                *n = 0;
                _mp4ErrorCode =  INVALID_SAMPLE_SIZE;
                return (_mp4ErrorCode);
            }
            sigmaSampleSize += tempSize;
            mInfo[s].len = tempSize;
            sampleFileOffset = (*_tfRunSampleInfo)[baseSampleNum]->_sample_offset;
            mInfo[s].ts_delta = tsDelta;
            mInfo[s].ts = currTSBase;
            currTSBase += tsDelta;
            debugOffset = sampleFileOffset;

            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentAtom::peekNextNSamples- Track Fragment Run Offset[%d] =%d", s, tfrunoffset));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentAtom::peekNextNSamples- pgau->info[%d].len =%d", s, mInfo[s].len));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentAtom::peekNextNSamples- pgau->info[%d].ts_delta =%d", s, mInfo[s].ts_delta));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentAtom::peekNextNSamples- pgau->info[%d].ts =%d", s, mInfo[s].ts));
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentAtom::peekNextNSamples- Offset =%d", debugOffset));

            s++;
            StartPeekFromSampleNum++;
        }

        if (_peekPlaybackSampleNumber == (uint32)sampleBeforeGet)
        {
            _peekPlaybackSampleNumber += numSamples;
            sampleBeforeGet += numSamples;
        }
        else
        {
            break;
        }

        if (_peekPlaybackSampleNumber == totalnumSamples)
        {
            break;
        }

        if (_peekPlaybackSampleNumber > totalnumSamples)
        {
            _mp4ErrorCode = END_OF_TRACK;
            break;
        }
    }

    if (_peekPlaybackSampleNumber == (uint32) sampleBeforeGet)
    {
        *n = (*n - samplesToBePeek);
    }
    else
    {
        *n = 0;
        _mp4ErrorCode = READ_FAILED;
    }

    return (_mp4ErrorCode);

}

int32
TrackFragmentAtom::peekNextBundledAccessUnits(uint32 *n, uint32 totalSampleRead,
        MediaMetaInfo *mInfo)
{
    int32 nReturn = -1;
    nReturn = peekNextNSamples(_peekPlaybackSampleNumber, n, totalSampleRead , mInfo);
    return nReturn;
}


int32 TrackFragmentAtom::resetPlayback(uint32 time, uint32 trun_number, uint32 sample_num)
{
    int32 Return = -1;
    uint32 samplesInPrevTrun = 0;

    for (uint32 idx = 0; idx < trun_number - 1; idx++)
    {
        samplesInPrevTrun += (*_pTrackFragmentRunAtomVec)[idx]->getSampleCount();
    }
    TrackFragmentRunAtom *trackFragmentRunAtom = (*_pTrackFragmentRunAtomVec)[trun_number-1];
    if (trackFragmentRunAtom != NULL)
    {
        trackFragmentRunAtom->setSampleDurationAndTimeStampFromSampleNum(sample_num - 1, time, _default_duration);
        Oscl_Vector<TFrunSampleTable*, OsclMemAllocator>* _tfRunSampleInfo = trackFragmentRunAtom->getSampleTable();
        if (_tfRunSampleInfo != NULL)
        {
            uint32 TimeStamp = Oscl_Int64_Utils::get_uint64_lower32(
                                   (*_tfRunSampleInfo)[sample_num-1]->_sample_timestamp);
            if (time >= TimeStamp)
            {
                _currentTrackFragmentRunSampleNumber = samplesInPrevTrun + (sample_num - 1);
                _currentPlaybackSampleTimestamp = time;
                _peekPlaybackSampleNumber = samplesInPrevTrun + (sample_num - 1);
                Return = time;
            }
        }
        _trackEndDuration = trackFragmentRunAtom->_sampleTimeStamp;
        for (uint32 idx = trun_number; idx < _pTrackFragmentRunAtomVec->size(); idx++)
        {
            trackFragmentRunAtom = (*_pTrackFragmentRunAtomVec)[idx];
            trackFragmentRunAtom->setSampleDurationAndTimeStampFromSampleNum(0,
                    Oscl_Int64_Utils::get_uint64_lower32(_trackEndDuration),
                    _default_duration);
            _trackEndDuration = trackFragmentRunAtom->_sampleTimeStamp;
        }

        _pTrackDurationContainer->updateTrackDurationForTrackId(trackId,
                Oscl_Int64_Utils::get_uint64_lower32(_trackEndDuration));
    }
    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentAtom::Return TS =%d", Return));

    return Return;

}

int32 TrackFragmentAtom::resetPlayback(uint32 time)
{
    int32 Return = -1;
    if (_pTrackFragmentRunAtomVec != NULL)
    {
        uint32 samplecount = 0;
        uint32 numTrackFragmentRun =  _pTrackFragmentRunAtomVec->size();

        for (uint32 idx = 0; idx < numTrackFragmentRun; idx++)
        {
            TrackFragmentRunAtom *tfrun = (*_pTrackFragmentRunAtomVec)[idx];
            tfrun->setSampleDurationAndTimeStampFromSampleNum(0, time, _default_duration);

            Oscl_Vector<TFrunSampleTable*, OsclMemAllocator>* trackFragmentRunSampleInfo = tfrun->getSampleTable();
            samplecount = (*_pTrackFragmentRunAtomVec)[idx]->getSampleCount();
            for (uint32 idy = 0; idy < samplecount; idy++)
            {
                uint32 TimeStamp = Oscl_Int64_Utils::get_uint64_lower32((*trackFragmentRunSampleInfo)[idy]->_sample_timestamp);
                if (time >= TimeStamp)
                {
                    _currentTrackFragmentRunSampleNumber = idy;
                    _currentPlaybackSampleTimestamp = time;
                    _peekPlaybackSampleNumber = idy;
                    Return = time;
                    break;
                }

            }
            _trackEndDuration = tfrun->_sampleTimeStamp;
            for (uint32 idx = 1; idx < _pTrackFragmentRunAtomVec->size(); idx++)
            {
                tfrun = (*_pTrackFragmentRunAtomVec)[idx];
                tfrun->setSampleDurationAndTimeStampFromSampleNum(0,
                        Oscl_Int64_Utils::get_uint64_lower32(_trackEndDuration),
                        _default_duration);
                _trackEndDuration = tfrun->_sampleTimeStamp;
            }
            _pTrackDurationContainer->updateTrackDurationForTrackId(trackId,
                    Oscl_Int64_Utils::get_uint64_lower32(_trackEndDuration));

            if (Return != -1)
            {
                break;
            }
        }
    }
    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "TrackFragmentAtom::Return TS =%d", Return));
    return Return;
}
void TrackFragmentAtom::resetPlayback()
{
    _currentTrackFragmentRunSampleNumber = 0;
    _peekPlaybackSampleNumber = 0;
    _startTrackFragmentTSOffset = 0;
    _currentPlaybackSampleTimestamp = _startTrackFragmentTSOffset;
}

uint32 TrackFragmentAtom::getCurrentTrafDuration()
{
    return Oscl_Int64_Utils::get_uint64_lower32(_trackEndDuration);
}

int32
TrackFragmentAtom::getOffsetByTime(uint32 id, uint32 ts, int32* sampleFileOffset)
{
    OSCL_UNUSED_ARG(id);
    uint32 time = ts;
    uint32 prevTime = 0, prevOffset = 0;
    if (_pTrackFragmentRunAtomVec != NULL)
    {
        for (uint32 idx = 0; idx < _pTrackFragmentRunAtomVec->size(); idx++)
        {
            TrackFragmentRunAtom *trackFragmentRunAtom = (*_pTrackFragmentRunAtomVec)[idx];
            if (trackFragmentRunAtom != NULL)
            {
                Oscl_Vector<TFrunSampleTable*, OsclMemAllocator>* _tfRunSampleInfo = trackFragmentRunAtom->getSampleTable();
                if (_tfRunSampleInfo != NULL)
                {

                    for (uint32 i = 0; i < _tfRunSampleInfo->size(); i++)
                    {
                        if (time < Oscl_Int64_Utils::get_uint64_lower32((*_tfRunSampleInfo)[i]->_sample_timestamp))
                        {
                            uint32 tmp = Oscl_Int64_Utils::get_uint64_lower32((*_tfRunSampleInfo)[i]->_sample_timestamp);
                            uint32 diffwithbeforeTS = time - prevTime;
                            uint32 diffwithafterTS = tmp - time;
                            if (diffwithbeforeTS > diffwithafterTS)
                            {
                                *sampleFileOffset = (*_tfRunSampleInfo)[i]->_sample_offset;;
                                return EVERYTHING_FINE;
                            }
                            else
                            {
                                *sampleFileOffset = prevOffset;
                                return EVERYTHING_FINE;
                            }
                        }
                        prevTime = Oscl_Int64_Utils::get_uint64_lower32((*_tfRunSampleInfo)[i]->_sample_timestamp);
                        prevOffset = (*_tfRunSampleInfo)[i]->_sample_offset;
                    }
                }

            }
        }
    }
    return EVERYTHING_FINE;
}


void TrackDurationContainer::updateTrackDurationForTrackId(int32 id, uint32 duration)
{
    if (_pTrackdurationInfoVec != NULL)
    {
        for (uint32 i = 0; i < _pTrackdurationInfoVec->size();i++)
        {
            if ((int32)((*_pTrackdurationInfoVec)[i]->trackId) == id)
            {
                (*_pTrackdurationInfoVec)[i]->trackDuration = duration;
            }
        }
    }
}
