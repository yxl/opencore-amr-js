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
/*                            MPEG-4 Track Fragment Atom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
*/

#define IMPLEMENT_MovieFragmentRandomAccessAtom

#include "atomdefs.h"
#include "atomutils.h"
#include "moviefragmentrandomaccess.h"

typedef Oscl_Vector<TrackFragmentRandomAccessAtom*, OsclMemAllocator> trackFragmentRandomAccessAtomVecType;

// Constructor
MovieFragmentRandomAccessAtom::MovieFragmentRandomAccessAtom(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        : Atom(fp, size, type)
{
    _pTrackFragmentRandomAccessAtomVec		= NULL;
    _pMfraOffsetAtom = NULL;
#if (DISABLE_REPOS_ON_CLIPS_HAVING_UNEQUAL_TFRA_ENTRY_COUNT)
    // This will store the entry count of TFRA for Video for Vonly,AVT,AV,VT clips and Audio TFRA entry count for Aonly,AT clips
    // and Text TFRA entry count for Tonly clips.
    oVideoAudioTextTrackTfraCount = 0;
#endif // DISABLE_REPOS_ON_CLIPS_HAVING_UNEQUAL_TFRA_ENTRY_COUNT

    iLogger = PVLogger::GetLoggerObject("mp4ffparser");
    iStateVarLogger = PVLogger::GetLoggerObject("mp4ffparser_mediasamplestats");
    iParsedDataLogger = PVLogger::GetLoggerObject("mp4ffparser_parseddata");

    uint32 count = size - DEFAULT_ATOM_SIZE;
    if (_success)
    {
        PV_MP4_FF_NEW(fp->auditCB, trackFragmentRandomAccessAtomVecType, (), _pTrackFragmentRandomAccessAtomVec);
        while (count > 0)
        {
            uint32 atomType = UNKNOWN_ATOM;
            uint32 atomSize = 0;
            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            if (atomType == MOVIE_FRAGMENT_RANDOM_ACCESS_OFFSET_ATOM)
            {
                if (_pMfraOffsetAtom == NULL)
                {
                    PV_MP4_FF_NEW(fp->auditCB, MfraOffsetAtom, (fp, atomSize, atomType), _pMfraOffsetAtom);
                    if (!_pMfraOffsetAtom->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = READ_MOVIE_FRAGMENT_RANDOM_ACCESS_OFFSET_FAILED;
                        return;
                    }
                    count -= _pMfraOffsetAtom->getSize();
                }
                else
                {
                    //duplicate atom
                    count -= atomSize;
                    atomSize -= DEFAULT_ATOM_SIZE;
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                }
            }
            else if (atomType == TRACK_FRAGMENT_RANDOM_ACCESS_ATOM)
            {
                TrackFragmentRandomAccessAtom *pTrackFragmentRandomAccessAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, TrackFragmentRandomAccessAtom, (fp, atomSize, atomType),
                              pTrackFragmentRandomAccessAtom);
                if (!pTrackFragmentRandomAccessAtom->MP4Success())
                {
                    PV_MP4_FF_DELETE(NULL, TrackFragmentRandomAccessAtom, pTrackFragmentRandomAccessAtom);
                    _success = false;
                    _mp4ErrorCode = READ_TRACK_FRAGMENT_RANDOM_ACCESS_ATOM_FAILED;
                    return;
                }
                count -= pTrackFragmentRandomAccessAtom->getSize();
                _pTrackFragmentRandomAccessAtomVec->push_back(pTrackFragmentRandomAccessAtom);
            }

        }
    }
    else
    {
        _mp4ErrorCode = READ_MOVIE_FRAGMENT_RANDOM_ACCESS_ATOM_FAILED;
    }
}

MovieFragmentRandomAccessAtom::~MovieFragmentRandomAccessAtom()
{
    if (_pMfraOffsetAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, MfraOffsetAtom, _pMfraOffsetAtom);
    }

    for (uint32 i = 0; i < _pTrackFragmentRandomAccessAtomVec->size(); i++)
    {
        PV_MP4_FF_DELETE(NULL, TrackFragmentRandomAccessAtom, (*_pTrackFragmentRandomAccessAtomVec)[i]);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, trackFragmentRandomAccessAtomVecType, Oscl_Vector, _pTrackFragmentRandomAccessAtomVec);

}

bool MovieFragmentRandomAccessAtom::IsTFRAPresentForTrack(uint32 trackID, bool oVideoAudioTextTrack)
{

    uint32 num_tfra = 0;
    if (_pTrackFragmentRandomAccessAtomVec != NULL)
    {
        num_tfra = _pTrackFragmentRandomAccessAtomVec->size();
        for (uint32 idx = 0; idx < num_tfra; idx++)
        {
            TrackFragmentRandomAccessAtom *tfraAtom = (*_pTrackFragmentRandomAccessAtomVec)[idx];
            if (tfraAtom->getTrackID() == trackID)
            {
                if (tfraAtom->_entry_count > 0)
                {
#if (DISABLE_REPOS_ON_CLIPS_HAVING_UNEQUAL_TFRA_ENTRY_COUNT)
                    if (oVideoAudioTextTrack)
                    {
                        oVideoAudioTextTrackTfraCount = tfraAtom->_entry_count;
                        // If oVideoAudioTextTrack flag is set we just store the entry count to be checked
                        // for other tracks.
                        return true;
                    }
                    else if (oVideoAudioTextTrackTfraCount == tfraAtom->_entry_count)
#else
                    OSCL_UNUSED_ARG(oVideoAudioTextTrack);
#endif // DISABLE_REPOS_ON_CLIPS_HAVING_UNEQUAL_TFRA_ENTRY_COUNT
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;

}
int32 MovieFragmentRandomAccessAtom::getSyncSampleInfoClosestToTime(uint32 trackID, uint32 &time, uint32 &moof_offset,
        uint32 &traf_number, uint32 &trun_number,
        uint32 &sample_num)
{
    uint32 num_tfra = 0;
    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "MovieFragmentRandomAccessAtom::getSyncSampleInfoClosestToTime Input Time =%d", time));
    if (_pTrackFragmentRandomAccessAtomVec == NULL)
    {
        return -1;
    };
    num_tfra = _pTrackFragmentRandomAccessAtomVec->size();
    for (uint32 idx = 0; idx < num_tfra; idx++)
    {
        TrackFragmentRandomAccessAtom *tfraAtom = (*_pTrackFragmentRandomAccessAtomVec)[idx];
        if (! tfraAtom)		// (unlikely)
            return -1;
        if (tfraAtom->getTrackID() == trackID)
        {
            uint32 entries = tfraAtom->_entry_count;
            if (entries == 0)
                return -1;

            Oscl_Vector<TFRAEntries*, OsclMemAllocator>* tfraEntries = tfraAtom->getTrackFragmentRandomAccessEntries();
            if (!tfraEntries)       // unlikely/error
                return -1;
            int32 prevTime = 0;
            for (uint32 idy = 0; idy < entries; idy++)
            {
                if (time < (*tfraEntries)[idy]->getTimeStamp())
                {
                    uint32 tmp = (*tfraEntries)[idy]->getTimeStamp();
                    uint32 diffwithbeforeTS = time - prevTime;
                    uint32 diffwithafterTS = tmp - time;
                    if (diffwithbeforeTS > diffwithafterTS)
                    {
                        time = tmp;
                        moof_offset = (*tfraEntries)[idy]->getTimeMoofOffset();
                        traf_number = (*tfraEntries)[idy]->_traf_number;
                        trun_number = (*tfraEntries)[idy]->_trun_number;
                        sample_num = (*tfraEntries)[idy]->_sample_number;
                        PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "MovieFragmentRandomAccessAtom::getSyncSampleInfoClosestToTime Return Time =%d", time));
                        return 0;
                    }
                    else
                    {
                        time = prevTime;
                        PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "MovieFragmentRandomAccessAtom::getSyncSampleInfoClosestToTime Return Time =%d", time));
                        return 0;
                    }
                }
                moof_offset = (*tfraEntries)[idy]->getTimeMoofOffset();
                traf_number = (*tfraEntries)[idy]->_traf_number;
                trun_number = (*tfraEntries)[idy]->_trun_number;
                sample_num = (*tfraEntries)[idy]->_sample_number;

                prevTime = (*tfraEntries)[idy]->getTimeStamp();
            }
            time = prevTime;
            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "MovieFragmentRandomAccessAtom::getSyncSampleInfoClosestToTime Return Time =%d", time));
            return 0;
        }
    }

    // error return if we get here.
    return -1;
}

int32 MovieFragmentRandomAccessAtom::queryRepositionTime(uint32 trackID, int32 time, bool oDependsOn, bool bBeforeRequestedTime)
{
    OSCL_UNUSED_ARG(oDependsOn);

    uint32 num_tfra = 0;
    int32 closestTime = 0;
    PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "MovieFragmentRandomAccessAtom::getSyncSampleInfoClosestToTime Input Time =%d", time));
    if (_pTrackFragmentRandomAccessAtomVec != NULL)
    {
        num_tfra = _pTrackFragmentRandomAccessAtomVec->size();
        for (uint32 idx = 0; idx < num_tfra; idx++)
        {
            TrackFragmentRandomAccessAtom* tfraAtom = (*_pTrackFragmentRandomAccessAtomVec)[idx];
            if (tfraAtom->getTrackID() == trackID)
            {
                uint32 entries = tfraAtom->_entry_count;
                Oscl_Vector<TFRAEntries*, OsclMemAllocator>* tfraEntries = tfraAtom->getTrackFragmentRandomAccessEntries();
                if (!tfraEntries)	// unlikely/error
                    return closestTime;
                int32 prevTime = 0;
                for (uint32 idy = 0; idy < entries; idy++)
                {
                    if (bBeforeRequestedTime)
                    {
                        if (time < (int32)(*tfraEntries)[idy]->getTimeStamp())
                        {
                            time = prevTime;
                            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "MovieFragmentRandomAccessAtom::getSyncSampleInfoClosestToTime Return Time =%d", time));
                            break;
                        }
                        else if ((time == (int32)(*tfraEntries)[idy]->getTimeStamp()) ||
                                 ((idy == (entries - 1)) && (time > (int32)(*tfraEntries)[idy]->getTimeStamp())))
                        {
                            time = (*tfraEntries)[idy]->getTimeStamp();
                            break;

                        }
                        prevTime = (*tfraEntries)[idy]->getTimeStamp();
                    }
                    else
                    {
                        if (time <= (int32)(*tfraEntries)[idy]->getTimeStamp())
                        {
                            time = (*tfraEntries)[idy]->getTimeStamp();
                            PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "MovieFragmentRandomAccessAtom::getSyncSampleInfoClosestToTime Return Time =%d", time));
                            break;
                        }
                        else if ((idy == (entries - 1)) && (time > (int32)(*tfraEntries)[idy]->getTimeStamp()))
                        {
                            time = (*tfraEntries)[idy]->getTimeStamp();
                            break;

                        }

                        prevTime = (*tfraEntries)[idy]->getTimeStamp();

                    }
                }
                closestTime = time;
                PVMF_MP4FFPARSER_LOGMEDIASAMPELSTATEVARIABLES((0, "MovieFragmentRandomAccessAtom::getSyncSampleInfoClosestToTime Return Time =%d", time));
                return closestTime;
            }
        }
    }
    return closestTime;
}

int32 MovieFragmentRandomAccessAtom::getTimestampForRandomAccessPoints(uint32 id, uint32 *num, uint32 *tsBuf, uint32* numBuf, uint32* offsetBuff, uint32 samplesFromMovie)
{
    uint32 num_tfra = 0;
    uint32 samplesfromMoov = samplesFromMovie;
    TrackFragmentRandomAccessAtom *tfraAtom = NULL;

    if (_pTrackFragmentRandomAccessAtomVec != NULL)
    {
        num_tfra = _pTrackFragmentRandomAccessAtomVec->size();

        for (uint32 idx = 0; idx < num_tfra; idx++)
        {
            tfraAtom = (*_pTrackFragmentRandomAccessAtomVec)[idx];
            if (tfraAtom->getTrackID() == id)
            {
                break;
            }
        }
    }
    if (tfraAtom != NULL)
    {
        uint32 entries = tfraAtom->_entry_count;
        Oscl_Vector<TFRAEntries*, OsclMemAllocator>* tfraEntries = tfraAtom->getTrackFragmentRandomAccessEntries();
        if (!tfraEntries)       // unlikely/error
            return 0;


        uint32 tmp = entries;

        if (*num == 0)
        {
            *num = tmp;
            return 1;	//success. This is only the query mode.
        }

        uint32 sampleToBeRead = *num;
        if (tmp < sampleToBeRead)
            sampleToBeRead = tmp;

        for (uint32 idx = samplesfromMoov; idx < sampleToBeRead + samplesfromMoov; idx++)
        {
            //it may crash if this buffer is not big enough
            if (tsBuf != NULL)
                tsBuf[idx] = (*tfraEntries)[idx]->getTimeStamp();
            if (numBuf)
                numBuf[idx] = (*tfraEntries)[idx]->_sample_number;
            if (offsetBuff)
                offsetBuff[idx] = (*tfraEntries)[idx]->getTimeMoofOffset();
        }
        *num = sampleToBeRead + samplesfromMoov;

        return	1;	//success
    }
    return 0;
}


int32 MovieFragmentRandomAccessAtom::getTimestampForRandomAccessPointsBeforeAfter(uint32 id, uint32 ts, uint32 *tsBuf, uint32* numBuf,
        uint32& numsamplestoget,
        uint32 howManyKeySamples)
{
    uint32 num_tfra = 0, prevSampleNum = 0, sample_num = 0;
    uint32 time = ts;
    uint32 startIdx = 0, endIdx = 0;

    TrackFragmentRandomAccessAtom *tfraAtom = NULL;
    if (_pTrackFragmentRandomAccessAtomVec != NULL)
    {
        num_tfra = _pTrackFragmentRandomAccessAtomVec->size();

        for (uint32 idx = 0; idx < num_tfra; idx++)
        {
            tfraAtom = (*_pTrackFragmentRandomAccessAtomVec)[idx];
            if (tfraAtom != NULL)
            {
                if (tfraAtom->getTrackID() == id)
                {
                    break;
                }
            }
        }
    }
    if (tfraAtom != NULL)
    {
        uint32 entries = tfraAtom->_entry_count;
        startIdx = entries;
        endIdx = entries;
        Oscl_Vector<TFRAEntries*, OsclMemAllocator>* tfraEntries = tfraAtom->getTrackFragmentRandomAccessEntries();
        if (!tfraEntries)       // unlikely/error
            return 0;
        int32 prevTime = 0;
        for (uint32 idy = 0; idy < entries; idy++)
        {
            if (time <= (*tfraEntries)[idy]->getTimeStamp())
            {
                uint32 tmp = (*tfraEntries)[idy]->getTimeStamp();
                uint32 diffwithbeforeTS = time - prevTime;
                uint32 diffwithafterTS = tmp - time;
                if (diffwithbeforeTS > diffwithafterTS)
                {
                    sample_num = (*tfraEntries)[idy]->_sample_number;
                }
                else
                {
                    sample_num = prevSampleNum;
                }
                startIdx = idy;
                endIdx = entries;
                break;
            }
            prevSampleNum = (*tfraEntries)[idy]->_sample_number;
            prevTime = (*tfraEntries)[idy]->getTimeStamp();
        }
        if ((startIdx + howManyKeySamples) <= entries)
            endIdx = startIdx + howManyKeySamples;

        if (startIdx > howManyKeySamples)
            startIdx -= howManyKeySamples;
        else
        {
            startIdx = 0;
        }

        uint32 idx = 0;
        uint32 k = 0;
        for (idx = startIdx; idx < endIdx; idx++)
        {
            int32 keySampleNum = (*tfraEntries)[idx]->_sample_number;
            int32 keySampleTS = (*tfraEntries)[idx]->getTimeStamp();
            numBuf[k] = keySampleNum;
            tsBuf[k] = keySampleTS;
            k++;
        }
        numsamplestoget = k;
    }
    else
        return 0;

    return	1;	//success

}
