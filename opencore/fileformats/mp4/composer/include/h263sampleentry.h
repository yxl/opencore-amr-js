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


#ifndef __H263SampleEntry_H__
#define __H263SampleEntry_H__

#include "sampleentry.h"
#include "decoderspecificinfo.h"
#include "h263decoderspecificinfo3gpp.h"

class PVA_FF_H263SampleEntry : public PVA_FF_SampleEntry
{

    public:

        PVA_FF_H263SampleEntry(); // Constructor

        virtual ~PVA_FF_H263SampleEntry();

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        // Getting and setting the Mpeg4 VOL header
        virtual PVA_FF_DecoderSpecificInfo *getDecoderSpecificInfo() const
        {
            return NULL;
        }
        virtual PVA_FF_H263SpecficAtom *get3GPPDecoderSpecificInfo() const
        {
            return pH263SpecificAtom;
        }

        void setVideoParams(uint16 frame_width, uint16 frame_height)
        {
            _reserved2 = frame_width;
            _reserved2 = (_reserved2 << 16);

            _reserved2 |= frame_height;
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

        PVA_FF_H263SpecficAtom *pH263SpecificAtom;
};


#endif

