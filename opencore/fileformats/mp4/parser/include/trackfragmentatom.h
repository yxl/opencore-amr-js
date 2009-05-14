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


#ifndef TRACKFRAGMENTATOM_H_INCLUDED
#define TRACKFRAGMENTATOM_H_INCLUDED

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef TRACKFRAGMENTHEADERATOM_H_INCLUDED
#include "trackfragmentheaderatom.h"
#endif

#ifndef TRACKFRAGMENTRUNATOM_H_INCLUDED
#include "trackfragmentrunatom.h"
#endif

#ifndef PV_GAU_H_INCLUDED
#include "pv_gau.h"
#endif

#ifndef TRACKEXTENDSATOM_H_INCLUDED
#include "trackextendsatom.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

class TrackDurationInfo : public HeapBase
{
    public:
        TrackDurationInfo(uint32 td, uint32 id)
        {
            trackDuration = td;
            trackId = id;
        }

        ~TrackDurationInfo() {}
        uint32 trackDuration;
        uint32 trackId;
    private:
};

class TrackDurationContainer : public HeapBase
{

    public:
        TrackDurationContainer()
        {
            _pTrackdurationInfoVec = NULL;
        }
        ~TrackDurationContainer() {};

        int32 getNumTrackInfoVec()
        {
            if (_pTrackdurationInfoVec != NULL)
                return _pTrackdurationInfoVec->size();
            return 0;
        }
        TrackDurationInfo *getTrackdurationInfoAt(int32 index)
        {
            if (index >= 0 && index < (int32)_pTrackdurationInfoVec->size())
                return (*_pTrackdurationInfoVec)[index];
            else
                return NULL;
        }

        void updateTrackDurationForTrackId(int32 id, uint32 duration);

        Oscl_Vector<TrackDurationInfo*, OsclMemAllocator> *_pTrackdurationInfoVec;

    private:

};

class TrackFragmentAtom : public Atom
{

    public:

        TrackFragmentAtom(MP4_FF_FILE *fp,
                          uint32 &size,
                          uint32 type,
                          uint32 movieFragmentCurrentOffset,
                          uint32 movieFragmentBaseOffset,
                          uint32 moof_size,
                          TrackDurationContainer *trackDurationContainer,
                          Oscl_Vector<TrackExtendsAtom*, OsclMemAllocator> *trackExtendAtomVec,
                          bool &parseTrafCompletely,
                          bool &trafParsingCompleted,
                          uint32 &countOfTrunsParsed);

        virtual ~TrackFragmentAtom();

        void ParseTrafAtom(MP4_FF_FILE *fp,
                           uint32 &size,
                           uint32 type,
                           uint32 movieFragmentCurrentOffset,
                           uint32 movieFragmentBaseOffset,
                           uint32 moofSize,
                           TrackDurationContainer *trackDurationContainer,
                           Oscl_Vector<TrackExtendsAtom*, OsclMemAllocator> *trackExtendAtomVec,
                           bool &trafParsingCompleted,
                           uint32 &countOfTrunsParsed);

        uint32 getTrackId()
        {
            if (_pTrackFragmentHeaderAtom != NULL)
            {
                return _pTrackFragmentHeaderAtom->getTrackId();
            }
            return 0;
        }
        uint32 getSampleCount();
        Oscl_Vector<TFrunSampleTable*, OsclMemAllocator>* getSampleTable();
        uint64 getBaseDataOffset();
        uint32 getSampleDescriptionIndex();
        uint32 getDefaultSampleDuration();
        uint32 getDefaultSampleSize();
        uint32 getDefaultSampleFlags();
        TrackFragmentRunAtom *getTrackFragmentRunForSampleNum(uint32 samplenum, uint32 &samplecount);
        int32 getNextNSamples(uint32 startSampleNum, uint32 *n, uint32 totalSampleRead, GAU    *pgau);
        int32 getNextBundledAccessUnits(uint32 *n, uint32 totalSampleRead, GAU *pgau);
        int32 peekNextNSamples(uint32 startSampleNum, uint32 *n, uint32 totalSampleRead, MediaMetaInfo *mInfo);
        int32 peekNextBundledAccessUnits(uint32 *n, uint32 totalSampleRead, MediaMetaInfo *mInfo);
        uint32 getTotalNumSampleInTraf();
        uint32 _trackFragmentEndOffset;
        int32 resetPlayback(uint32 time, uint32 trun_number, uint32 sample_num);
        void resetPlayback();
        uint32 getSampleNumberFromTimestamp(uint32 time);
        uint32 getTimestampForSampleNumber(uint32 sampleNumber);
        uint32 getCurrentTrafDuration();
        int32 getOffsetByTime(uint32 id, uint32 ts, int32* sampleFileOffset);
        int32 resetPlayback(uint32 time);


    private:

        TrackFragmentHeaderAtom * _pTrackFragmentHeaderAtom;
        TrackFragmentRunAtom *_pTrackFragmentRunAtom;
        Oscl_Vector<TrackFragmentRunAtom*, OsclMemAllocator> *_pTrackFragmentRunAtomVec;
        uint32 _currentPlaybackSampleTimestamp;
        uint32 _currentTrackFragmentRunSampleNumber;
        uint32 _peekPlaybackSampleNumber;
        MP4_FF_FILE *_pinput;
        MP4_FF_FILE *_commonFilePtr;
        uint32 _startTrackFragmentTSOffset;
        uint32 _fileSize;
        uint32 _movieFragmentOffset;
        uint32 _prevSampleOffset;
        PVLogger *iLogger, *iStateVarLogger, *iParsedDataLogger;
        uint64 _trackEndDuration;
        Oscl_Vector<uint32, OsclMemAllocator> *_pFragmentptrOffsetVec;
        uint32 _cnt;
        uint32 _default_duration;
        bool _use_default_duratoin;
        TrackDurationContainer *_pTrackDurationContainer;
        uint32 trackId;
        uint32 tf_flags;
        uint32 trun_offset;
        bool trunParsingCompleted;

};

#endif


