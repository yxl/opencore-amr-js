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
/*                            MPEG-4 Movie Fragment Random Access  Atom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
*/

#ifndef MOVIEFRAGMENTRANDOMACCESSATOM_H_INCLUDED
#define MOVIEFRAGMENTRANDOMACCESSATOM_H_INCLUDED

#ifndef FULLATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef ATOMUTILS_H_INCLUDED
#include "atomutils.h"
#endif

#ifndef MFRAOFFSETATOM_H_INCLUDED
#include "mfraoffsetatom.h"
#endif

#ifndef CKFRAGMENTRANDOMACCESS_H_INCLUDED
#include "trackfragmentrandomaccess.h"
#endif

class MovieFragmentRandomAccessAtom : public Atom
{

    public:
        MovieFragmentRandomAccessAtom(MP4_FF_FILE *fp,
                                      uint32 size,
                                      uint32 type);

        virtual ~MovieFragmentRandomAccessAtom();
        int32 getSyncSampleInfoClosestToTime(uint32 trackID, uint32 &time, uint32 &moof_offset,
                                             uint32 &traf_number, uint32 &trun_number,
                                             uint32 &sample_num);
        int32 queryRepositionTime(uint32 trackID, int32 time, bool oDependsOn, bool bBeforeRequestedTime);
        int32 getTimestampForRandomAccessPointsBeforeAfter(uint32 id, uint32 ts, uint32 *tsBuf, uint32* numBuf,
                uint32& numsamplestoget,
                uint32 howManyKeySamples);
        int32 getTimestampForRandomAccessPoints(uint32 id, uint32 *num, uint32 *tsBuf, uint32* numBuf, uint32* offsetBuff = NULL, uint32 samplesFromMovie = 0);

        /*
        *  This function will check if TFRA is present for all tracks and the entry count in TFRA for all
        *  tracks are equal.
        */
        bool IsTFRAPresentForTrack(uint32 trackID, bool  oVideoAudioTextTrack);
#if (DISABLE_REPOS_ON_CLIPS_HAVING_UNEQUAL_TFRA_ENTRY_COUNT)
        int32 oVideoAudioTextTrackTfraCount;
#endif // DISABLE_REPOS_ON_CLIPS_HAVING_UNEQUAL_TFRA_ENTRY_COUNT

    private:
        Oscl_Vector<TrackFragmentRandomAccessAtom*, OsclMemAllocator> *_pTrackFragmentRandomAccessAtomVec;
        MfraOffsetAtom *_pMfraOffsetAtom;
        PVLogger *iLogger, *iStateVarLogger, *iParsedDataLogger;
};

#endif
