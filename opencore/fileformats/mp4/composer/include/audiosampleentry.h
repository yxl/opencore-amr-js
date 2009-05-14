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
    This PVA_FF_AudioSampleEntry Class is used for visual streams.
*/
#ifndef __AudioSampleEntry_H__
#define __AudioSampleEntry_H__

#include "sampleentry.h"
#include "esdatom.h"


class PVA_FF_AudioSampleEntry : public PVA_FF_SampleEntry
{

    public:
        PVA_FF_AudioSampleEntry(int32 codecType); // Constructor

        virtual ~PVA_FF_AudioSampleEntry();

        // Member gets and sets
        const PVA_FF_ESDAtom &getESDAtom() const
        {
            return *_pes;
        }
        void setESDAtom(PVA_FF_ESDAtom *esd)
        {
            _pes = esd;
        }

        uint16 getTimeScale()
        {
            return _timeScale;
        }
        void setTimeScale(uint16 ts)
        {
            _timeScale = ts;
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        virtual PVA_FF_DecoderSpecificInfo *getDecoderSpecificInfo() const
        {
            return _pes->getDecoderSpecificInfo();
        }
        virtual void addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *info)
        {
            _pes->addDecoderSpecificInfo(info);
        }

        void setTargetBitrate(uint32 bitrate)
        {
            _pes->setTargetBitrate(bitrate);
        }

        void nextSampleSize(uint32 size)
        {
            _pes->nextSampleSize(size);
        }

        void setMaxBufferSizeDB(uint32 max)
        {
            _pes->setMaxBufferSizeDB(max);
        }

        void setTimeScale(uint32 ts)
        {
            _timeScale = (uint16)(ts);
        }

        void setESID(uint16 esid)
        {
            _pes->setESID(esid);
        }

        void writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP *_afp)
        {
            _pes->writeMaxSampleSize(_afp);
        }

    private:
        virtual void recomputeSize();
        void init();

        // Reserved constants
        uint32 _reserved1[2]; // = { 0, 0 };
        uint16 _reserved2; // = 2;
        uint16 _reserved3; // = 16;
        uint32 _reserved4; // = 0;
        uint16 _reserved5; // = 0;

        uint16 _timeScale;
        PVA_FF_ESDAtom *_pes;
};


#endif

