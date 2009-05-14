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
    This PVA_FF_MediaInformationAtom Class contains all the objects that declare
    characteristic information about the media data within the stream.
*/


#ifndef __MediaInformationAtom_H__
#define __MediaInformationAtom_H__

#include "atom.h"
#include "mediainformationheaderatom.h"
#include "datainformationatom.h"
#include "sampletableatom.h"
#include "esdescriptor.h"

class PVA_FF_MediaInformationAtom : public PVA_FF_Atom
{

    public:
        // The protocol parameter is used when creating a HINT track.  It is passed
        // down to the PVA_FF_SampleDescriptionAtom so it can create the appropriate PVA_FF_HintSampleEntry.
        // If this constructor is used to create any other typ eof track atom, the protocol
        // parameter is ignored - hence the default parameter value.
        PVA_FF_MediaInformationAtom(uint32 mediaType,
                                    int32 codecType,
                                    uint32 fileAuthoringFlags,
                                    uint32 protocol = 0,
                                    uint8 profile = 1,
                                    uint8 profileComp = 0xFF,
                                    uint8 level = 0xFF); // Constructor

        virtual ~PVA_FF_MediaInformationAtom();

        // Stream properties
        void setTargetBitrate(uint32 bitrate)
        {
            _psampleTableAtom->setTargetBitrate(bitrate);
        }

        // Called to update the member atoms if they depend on the actual
        // file offsets where the media data atom (for which this track stores
        // the meta data) is stored
        void updateAtomFileOffsets(uint32 offset)
        {
            _psampleTableAtom->updateAtomFileOffsets(offset);
        }

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
        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        // Getting and setting the Mpeg4 VOL header
        void addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *pinfo)
        {
            _psampleTableAtom->addDecoderSpecificInfo(pinfo);
        }

        // Getting and setting the Mpeg4 VOL header for timed text case
        void addTextDecoderSpecificInfo(PVA_FF_TextSampleDescInfo *pinfo)
        {
            _psampleTableAtom->addTextDecoderSpecificInfo(pinfo);
        }
        void setMaxBufferSizeDB(uint32 max)
        {
            _psampleTableAtom->setMaxBufferSizeDB(max);
        }

        void setTimeScale(uint32 ts)
        {
            _psampleTableAtom->setTimeScale(ts);
        }

        void setVideoParams(uint32 frame_width, uint32 frame_height)
        {
            _psampleTableAtom->setVideoParams(frame_width, frame_height);
        }

        void setH263ProfileLevel(uint8 profile, uint8 level)
        {
            _psampleTableAtom->setH263ProfileLevel(profile, level);
        }

        void setESID(uint16 esid)
        {
            _psampleTableAtom->setESID(esid);
        }
        void SetMaxSampleSize(uint32);
        void writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP*);

        uint32 getSampleCount() const
        {
            return _psampleTableAtom->getSampleCount();
        }

    private:
        void recomputeSize();
        void init(int32 mediaType);

        // This is actually a pointer to the base class.  The possible
        // derived classes include PVA_FF_VideoMediaHeaderAtom, PVA_FF_SoundMediaHeaderAtom,
        // PVA_FF_HintMediaHeaderAtom, and PVA_FF_Mpeg4MediaHeaderAtom.
        PVA_FF_MediaInformationHeaderAtom* _pmediaInformationHeader;

        PVA_FF_DataInformationAtom *_pdataInformationAtom;
        PVA_FF_SampleTableAtom *_psampleTableAtom;
        int32 iMediaType;
};



#endif

