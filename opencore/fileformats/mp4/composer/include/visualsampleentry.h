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
    This PVA_FF_VisualSampleEntry Class is used for visual streams.
*/


#ifndef __VisualSampleEntry_H__
#define __VisualSampleEntry_H__

#include "sampleentry.h"
#include "esdatom.h"


class PVA_FF_VisualSampleEntry : public PVA_FF_SampleEntry
{

    public:
        PVA_FF_VisualSampleEntry(int32 codecType); // Constructor

        virtual ~PVA_FF_VisualSampleEntry();

        // Member gets and sets
        const PVA_FF_ESDAtom &getESDAtom() const
        {
            return *_pes;
        }
        void setESDAtom(PVA_FF_ESDAtom *esd)
        {
            _pes = esd;
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        // Getting and setting the Mpeg4 VOL header
        virtual PVA_FF_DecoderSpecificInfo *getDecoderSpecificInfo() const
        {
            return _pes->getDecoderSpecificInfo();
        }
        virtual void addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *pinfo)
        {
            _pes->addDecoderSpecificInfo(pinfo);
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

        void setVideoParams(uint16 frame_width, uint16 frame_height)
        {
            _reserved2 = frame_width;
            _reserved2 = (_reserved2 << 16);

            _reserved2 |= frame_height;
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
        void init();
        virtual void recomputeSize();

        // Reserved constants
        uint32 _reserved1[4]; // = { 0, 0, 0, 0 };
        uint32 _reserved2; // = 0x014000f0;
        uint32 _reserved3; // = 0x00480000;
        uint32 _reserved4; // = 0x00480000;
        uint32 _reserved5; // = 0;
        uint16 _reserved6; // = 1;
        uint8  _reserved7[32]; // = 0;
        uint16 _reserved8; // = 24;
        int16  _reserved9; // = -1; // (16) SIGNED!

        PVA_FF_ESDAtom *_pes;


};


#endif

