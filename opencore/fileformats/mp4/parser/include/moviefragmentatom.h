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
/*                            MPEG-4 Movie Fragment Atom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
*/


#ifndef MOVIEFRAGMENTATOM_H_INCLUDED
#define MOVIEFRAGMENTATOM_H_INCLUDED

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef TRACKFRAGMENTRUNATOM_H_INCLUDED
#include "trackfragmentrunatom.h"
#endif

#ifndef TRACKFRAGMENTATOM_H_INCLUDED
#include "trackfragmentatom.h"
#endif

#ifndef MOVIEFRAGMENTHEADERATOM_H_INCLUDED
#include "moviefragmentheaderatom.h"
#endif

class MovieFragmentAtom : public Atom
{

    public:

        MovieFragmentAtom(MP4_FF_FILE *fp,
                          uint32 &size,
                          uint32 type,
                          TrackDurationContainer *trackDurationContainer,
                          Oscl_Vector<TrackExtendsAtom*, OsclMemAllocator> *trackExtendAtomVec,
                          bool &parseMoofCompletely,
                          bool &moofParsingCompleted,
                          uint32 &countOfTrunsParsed);


        virtual ~MovieFragmentAtom();

        int32 getSequenceNumber()
        {
            if (_pMovieFragmentHeaderAtom != NULL)
                return _pMovieFragmentHeaderAtom->getSequenceNumber();
            return 0;
        }

        void ParseMoofAtom(MP4_FF_FILE *fp,
                           uint32 &size,
                           uint32 type,
                           TrackDurationContainer *trackDurationContainer,
                           Oscl_Vector<TrackExtendsAtom*, OsclMemAllocator> *trackExtendAtomVec,
                           bool &moofParsingCompleted,
                           uint32 &countOfTrunsParsed);


        uint64 getDataOffset();
        uint32 getSampleCount();
        Oscl_Vector<TFrunSampleTable*, OsclMemAllocator> getSampleTable();
        uint64 getBaseDataOffset();
        uint32 getSampleDescriptionIndex();
        uint32 getDefaultSampleDuration();
        uint32 getDefaultSampleSize();
        uint32 getDefaultSampleFlags();
        int32 getNextBundledAccessUnits(const uint32 trackID, uint32 *n, uint32 totalSampleRead, GAU *pgau);
        int32 peekNextBundledAccessUnits(const uint32 trackID, uint32 *n,
                                         uint32 totalSampleRead,
                                         MediaMetaInfo *mInfo);

        TrackFragmentAtom * getTrackFragmentforID(uint32 id);

        uint32 _currentTrackFragmentOffset;
        int32 resetPlayback(uint32 trackID, uint32 time,
                            uint32 traf_number, uint32 trun_number,
                            uint32 sample_num);
        void resetPlayback();
        uint32 getCurrentTrafDuration(uint32 id);
        uint32 getTotalSampleInTraf(uint32 id);
        int32 getOffsetByTime(uint32 id, uint32 ts, int32* sampleFileOffset);



    private:
        MovieFragmentHeaderAtom * _pMovieFragmentHeaderAtom;
        TrackFragmentAtom *_pTrackFragmentAtom;
        Oscl_Vector<TrackFragmentAtom*, OsclMemAllocator> *_ptrackFragmentArray;
        uint32 _pMovieFragmentCurrentOffset;
        uint32 _pMovieFragmentBaseOffset;
        uint32 _trafIndex;
        PVLogger *iLogger, *iStateVarLogger, *iParsedDataLogger;
        bool parseTrafCompletely;
        bool trafParsingCompleted;
        uint32 sizeRemaining;
        uint32 atomtype;

};

#endif

