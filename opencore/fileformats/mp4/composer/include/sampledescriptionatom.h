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
    This PVA_FF_SampleDescriptionAtom Class gives detailed information about the codeing
    type used, and any initialization information needed for coding.
*/


#ifndef __SampleDescriptionAtom_H__
#define __SampleDescriptionAtom_H__

#include "fullatom.h"
#include "sampleentry.h"
#include "esdescriptor.h"
#include "decoderspecificinfo.h"


class PVA_FF_SampleDescriptionAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_SampleDescriptionAtom(uint32 mediaType,
                                     int32 codecType,
                                     uint32 protocol = 0,
                                     uint8 profile = 1,
                                     uint8 profileComp = 0xFF,
                                     uint8 level = 0xFF); // Constructor

        virtual ~PVA_FF_SampleDescriptionAtom();

        void init(int32 mediaType,
                  uint32 protocol,
                  uint8 profile,
                  uint8 profileComp,
                  uint8 level);

        int32  nextSample(uint32 size, uint8 flags);
        int32  nextTextSample(uint32 size, uint8 flags, int32 index);

        // Member gets and sets
        uint32 getEntryCount() const
        {
            return _entryCount;
        }
        uint32 getHandlerType() const
        {
            return _handlerType;
        }

        // Stream properties
        void setTargetBitrate(uint32 bitrate);

        void addSampleEntry(PVA_FF_SampleEntry *entry);
        void insertSampleEntryAt(int32 index, PVA_FF_SampleEntry *entry);
        void replaceSampleEntryAt(int32 index, PVA_FF_SampleEntry *entry);
        Oscl_Vector<PVA_FF_SampleEntry*, OsclMemAllocator>& getSampleEntries() const
        {
            return *_psampleEntryVec;
        }
        const PVA_FF_SampleEntry* getSampleEntryAt(int32 index) const;
        PVA_FF_SampleEntry* getMutableSampleEntryAt(int32 index);

        // Getting and setting the Mpeg4 VOL header
        void addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *pinfo);

        // Getting and setting the Mpeg4 VOL header for Timed text
        void addTextDecoderSpecificInfo(PVA_FF_TextSampleDescInfo *pinfo);

        void setMaxBufferSizeDB(uint32 max);

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        void setTimeScale(uint32 ts)
        {
            _mediaTimeScale = ts;
        }

        void setVideoParams(uint32 frame_width, uint32 frame_height)
        {
            _frame_width = (uint16)frame_width;
            _frame_height = (uint16)frame_height;
        }

        void setH263ProfileLevel(uint8 profile, uint8 level)
        {
            _h263Profile = profile;
            _h263Level   = level;
        }

        void setESID(uint16 esid)
        {
            for (uint32 i = 0; i < _psampleEntryVec->size(); i++)
            {
                (*_psampleEntryVec)[i]->setESID(esid);
            }
        }

        void SetMaxSampleSize(uint32);
        void writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP*);

    protected:
        virtual void recomputeSize();

    private:
        uint32 _entryCount;
        uint32 _handlerType; // Do NOT render this member or use it in a size calc
        Oscl_Vector<PVA_FF_SampleEntry*, OsclMemAllocator> *_psampleEntryVec; // Careful with vector access and cleanup!

        int32 _mediaType;
        int32 _currAudioBitrate;
        int32 _codecType;

        uint32 _mediaTimeScale;

        uint16 _frame_width;
        uint16 _frame_height;
        uint8  _h263Profile;
        uint8  _h263Level;

        bool _textflag;
        uint32 _text_sample_num;
        //vectors stores the information specific to the timed text file format.
        Oscl_Vector<int32, OsclMemAllocator> _SDIndex;

        Oscl_Vector<uint32, OsclMemAllocator> *_psampleEntryTypeVec; // Careful with vector access and cleanup!
};


#endif

