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
    This PVA_FF_SampleTableAtom Class contains all the time and data indexing of the
    media samples in a track.
*/


#ifndef __SampleTableAtom_H__
#define __SampleTableAtom_H__

#include "atom.h"
#include "timetosampleatom.h"
#include "samplesizeatom.h"
#include "chunkoffsetatom.h"
#include "sampletochunkatom.h"
#include "sampledescriptionatom.h"
#include "syncsampleatom.h"
#include "esdescriptor.h"
#include "a_atomdefs.h"

class PVA_FF_SampleTableAtom : public PVA_FF_Atom
{

    public:
        // The protocol parameter is used when creating a HINT track.  It is passed
        // down to the PVA_FF_SampleDescriptionAtom so it can create the appropriate PVA_FF_HintSampleEntry.
        // If this constructor is used to create any other typ eof track atom, the protocol
        // parameter is ignored - hence the default parameter value.
        PVA_FF_SampleTableAtom(uint32 mediaType,
                               int32 codecType,
                               uint32 fileAuthoringFlags,
                               uint32 protocol = 0,
                               uint8 profile = 1,
                               uint8 profileComp = 0xFF,
                               uint8 level = 0xFF); // Constructor

        virtual ~PVA_FF_SampleTableAtom();

        // Update members with next sample received
        void nextSample(int32 mediaType,
                        void *psample,
                        uint32 size,
                        uint32 ts,
                        uint8 flags = 0,
                        uint32 baseOffset = 0,
                        bool oChunkStart = false);

        void nextTextSample(int32 mediaType,
                            void *psample,
                            uint32 size,
                            uint32 ts,
                            uint8 flags = 0,
                            int32 index = 0,
                            uint32 baseOffset = 0,
                            bool oChunkStart = false);

        void nextSample(int32 mediaType,
                        Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                        uint32 size,
                        uint32 ts,
                        uint8 flags = 0,
                        uint32 baseOffset = 0,
                        bool oChunkStart = false);

        void nextTextSample(int32 mediaType,
                            Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                            uint32 size,
                            uint32 ts,
                            uint8 flags = 0,
                            int32 index = 0,
                            uint32 baseOffset = 0,
                            bool oChunkStart = false);

        bool reAuthorFirstSample(uint32 size,
                                 uint32 baseOffset);

        void updateLastTSEntry(uint32 ts);
        // Called to update the member atoms if they depend on the actual
        // file offsets where the media data atom (for which this track stores
        // the meta data) is stored
        void updateAtomFileOffsets(uint32 offset)
        {
            _pchunkOffsetAtom->updateChunkEntries(offset);
        }


        // Stream properties
        void setTargetBitrate(uint32 bitrate)
        {
            _psampleDescriptionAtom->setTargetBitrate(bitrate);
        }

        // Gets for optional member atoms
        const PVA_FF_SyncSampleAtom *getSyncSampleAtom() const
        {
            return _psyncSampleAtom;
        }
        void setSyncSampleAtom(PVA_FF_SyncSampleAtom *ssa)
        {
            _psyncSampleAtom = ssa;
        }
        // Creation methods for optional member atoms
        void createCompositionOffsetAtom() {};
        void createSyncSampleAtom() {};
        void createShadowSyncSampleAtom() {};
        void createDegradationPriorityAtom() {};

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        // Getting and setting the Mpeg4 VOL header
        void addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *pinfo)
        {
            _psampleDescriptionAtom->addDecoderSpecificInfo(pinfo);
        }

        // Getting and setting the Mpeg4 VOL header for Timed text
        void addTextDecoderSpecificInfo(PVA_FF_TextSampleDescInfo *pinfo)
        {
            _psampleDescriptionAtom->addTextDecoderSpecificInfo(pinfo);
        }

        void setMaxBufferSizeDB(uint32 max)
        {
            _psampleDescriptionAtom->setMaxBufferSizeDB(max);
        }

        void setTimeScale(uint32 ts)
        {
            _psampleDescriptionAtom->setTimeScale(ts);
        }

        void setVideoParams(uint32 frame_width, uint32 frame_height)
        {
            _psampleDescriptionAtom->setVideoParams(frame_width, frame_height);
        }

        void setH263ProfileLevel(uint8 profile, uint8 level)
        {
            _psampleDescriptionAtom->setH263ProfileLevel(profile, level);
        }

        void setESID(uint16 esid)
        {
            _psampleDescriptionAtom->setESID(esid);
        }
        void SetMaxSampleSize(uint32);
        void writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP*);


        uint32 getSampleCount() const
        {
            return _psampleSizeAtom->getSampleCount();
        }
    private:
        virtual void recomputeSize();

        PVA_FF_TimeToSampleAtom *_ptimeToSampleAtom;
        PVA_FF_SampleDescriptionAtom *_psampleDescriptionAtom;
        PVA_FF_SampleSizeAtom *_psampleSizeAtom;
        PVA_FF_SampleToChunkAtom *_psampleToChunkAtom;
        PVA_FF_ChunkOffsetAtom *_pchunkOffsetAtom;

        // Optional member atoms
        PVA_FF_SyncSampleAtom *_psyncSampleAtom;

        bool _oInterLeaveMode;
};



#endif

