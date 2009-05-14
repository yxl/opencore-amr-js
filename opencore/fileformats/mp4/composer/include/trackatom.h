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
/*
    This PVA_FF_TrackAtom Class is the container for a single track in the MPEG-4
    presentation.
*/


#ifndef __TrackAtom_H__
#define __TrackAtom_H__

#include "pv_mp4ffcomposer_config.h"
#include "atom.h"
#include "a_isucceedfail.h"

#include "trackheaderatom.h"
#include "mediaatom.h"
#include "trackreferenceatom.h"
#include "editatom.h"

#include "userdataatom.h"

#include "oscl_byte_order.h"
#include "oscl_bin_stream.h"
typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;
typedef Oscl_Vector<int32, OsclMemAllocator> int32VecType;
typedef Oscl_Vector<uint8, OsclMemAllocator> uint8VecType;

class PVA_FF_TrackAtom : public PVA_FF_Atom, public PVA_FF_ISucceedFail
{

    public:
        // The protocol parameter is used when creating a HINT track.  It is passed
        // down to the PVA_FF_SampleDescriptionAtom so it can create the appropriate PVA_FF_HintSampleEntry.
        // If this constructor is used to create any other typ eof track atom, the protocol
        // parameter is ignored - hence the default parameter value.
        PVA_FF_TrackAtom(int32 type,
                         uint32 id,
                         uint32 fileAuthoringFlags,
                         int32 codecType = 0,
                         uint32 protocol = 0,
                         uint8 profile = 1,
                         uint8 profileComp = 0xFF,
                         uint8 level = 0xFF); // Constructor

        int32 getCodecType()
        {
            return _codecType;
        }

        virtual ~PVA_FF_TrackAtom();

        // Member get methods
        const PVA_FF_TrackHeaderAtom &getTrackHeaderAtom() const
        {
            return *_ptrackHeader;
        }
        PVA_FF_TrackHeaderAtom *getTrackHeaderAtomPtr()
        {
            return _ptrackHeader;
        }
        PVA_FF_TrackHeaderAtom &getMutableTrackHeaderAtom()
        {
            return *_ptrackHeader;
        }
        const PVA_FF_MediaAtom &getMediaAtom() const
        {
            return *_pmediaAtom;
        }
        const PVA_FF_MediaAtom &getMutableMediaAtom()
        {
            return *_pmediaAtom;
        }
        PVA_FF_TrackReferenceAtom *getTrackReferenceAtomPtr()
        {
            return _ptrackReference;
        }

        // Add a reference to another track - return index of reference in table (1-based)
        int32 addTrackReference(uint32 ref);
        uint32 getTrackID() const
        {
            return _ptrackHeader->getTrackID();
        }

        // Update members with next sample received
        void nextSample(int32 mediaType,
                        void *psample,
                        uint32 size,
                        uint32 ts = 0,
                        uint8 flags = 0,
                        uint32 baseOffset = 0,
                        bool oChunkStart = false);

        void nextTextSample(int32 mediaType,
                            void *psample,
                            uint32 size,
                            uint32 ts = 0,
                            uint8 flags = 0,
                            int32 index = 0,
                            uint32 baseOffset = 0,
                            bool oChunkStart = false);

        void nextSample(int32 mediaType,
                        Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                        uint32 size,
                        uint32 ts = 0,
                        uint8 flags = 0,
                        uint32 baseOffset = 0,
                        bool oChunkStart = false);

        void nextTextSample(int32 mediaType,
                            Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                            uint32 size,
                            uint32 ts = 0,
                            uint8 flags = 0,
                            int32 index = 0,
                            uint32 baseOffset = 0,
                            bool oChunkStart = false);

        bool reAuthorFirstSample(uint32 size,
                                 uint32 baseOffset);

        // Create methods for the optional member atoms
        void createTrackReferenceAtom();
        void createEditAtom();

        // Called to update the member atoms if they depend on the actual
        // file offsets where the media data atom (for which this track stores
        // the meta data) is stored
        void updateAtomFileOffsets(uint32 offset)
        {
            _pmediaAtom->updateAtomFileOffsets(offset);
        }


        // Mpeg4 movie timescale
        void setTimeScale(uint32 ts)
        {
            _ptrackHeader->setTimeScale(ts);
        }
        uint32 getTimeScale() const
        {
            return _ptrackHeader->getTimeScale();
        }

        uint32 convertTrackDurationToMediaTimeScale(uint32 duration);

        void setDuration(uint32 duration)
        {
            _ptrackHeader->setDuration(duration);

            uint32 trackDurationInMediaTimeScale =
                convertTrackDurationToMediaTimeScale(duration);

            _pmediaAtom->setDuration(trackDurationInMediaTimeScale);
        }

        void setLanguage(uint32 language)
        {
            _pmediaAtom->setLanguage(language);
        }

        uint32 getDuration() const
        {
            return _ptrackHeader->getDuration();
        }

        // Stream properties
        void setTargetBitrate(uint32 bitrate)
        {
            _trackTargetBitRate = bitrate;
            _pmediaAtom->setTargetBitrate(bitrate);
        }

        // Media timescale
        void setMediaTimeScale(uint32 ts)
        {
            _pmediaAtom->setTimeScale(ts);
        }
        uint32 getMediaTimeScale() const
        {
            return _pmediaAtom->getTimeScale();
        }

        // Track media type
        int32 getTrackMediaType() const
        {
            return _ptrackHeader->getTrackMediaType();
        }
        uint32 getMediaType() const
        {
            return _mediaType;
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        void prepareToRender();

        // Getting and setting the Mpeg4 VOL header
        void addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *pinfo)
        {
            _setDecoderSpecificInfoDone = true;
            _pmediaAtom->addDecoderSpecificInfo(pinfo);
        }

        // Getting and setting the Mpeg4 VOL header for Timed text
        void addTextDecoderSpecificInfo(PVA_FF_TextSampleDescInfo *pinfo)
        {
            _setDecoderSpecificInfoDone = true;
            _pmediaAtom->addTextDecoderSpecificInfo(pinfo);
        }

        void setMaxBufferSizeDB(uint32 max)
        {
            _pmediaAtom->setMaxBufferSizeDB(max);
        }

        void setVideoParams(uint32 frame_width, uint32 frame_height);

        void setH263ProfileLevel(uint8 profile, uint8 level)
        {
            _pmediaAtom->setH263ProfileLevel(profile, level);
        }

        void setESID(uint16 esid)
        {
            _pmediaAtom->setESID(esid);
        }

        bool IsDecoderSpecificInfoSet()
        {
            return _setDecoderSpecificInfoDone;
        }

        uint32 getSampleCount() const
        {
            return _pmediaAtom->getSampleCount();
        }

        bool IsFirstSample()
        {
            return FIRST_SAMPLE;
        }
        void updateLastTSEntry(uint32 ts);
        void SetMaxSampleSize(uint32);
        void writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP*);

    private:
        virtual void recomputeSize();

        PVA_FF_TrackHeaderAtom    *_ptrackHeader;
        PVA_FF_MediaAtom          *_pmediaAtom;
        PVA_FF_TrackReferenceAtom *_ptrackReference; // Optional atom - MANDATORY for hint tracks

        PVA_FF_UserDataAtom       *_pUserDataAtom;

        int32 _codecType;
        int32 _mediaType;

        bool FIRST_SAMPLE;
        PVA_FF_EditAtom	*_eList;

        bool   _oInterLeaveMode;

        bool _setDecoderSpecificInfoDone;
        uint32 _trackTargetBitRate;

        uint32 _intialTrackTimeOffsetInMilliSeconds;


};



#endif

