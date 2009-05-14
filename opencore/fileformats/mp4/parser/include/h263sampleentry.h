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
/*                        MPEG-4 H263SampleEntry Class                           */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/


#ifndef H263SAMPLEENTRY_H_INCLUDED
#define H263SAMPLEENTRY_H_INCLUDED

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef GPP_H263DECODERSPECIFICINFO_H_INCLUDED
#include "3gpp_h263decoderspecificinfo.h"
#endif

#ifndef H263DECODERSPECIFICINFO_H_INCLUDED
#include "h263decoderspecificinfo.h"
#endif

class H263SampleEntry : public Atom
{

    public:
        H263SampleEntry(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~H263SampleEntry();

        // Getting and setting the Mpeg4 VOL header
        virtual DecoderSpecificInfo *get3GPPH263DecoderSpecificInfo()
        {
            return _pH263decSpecificInfo;
        }

        uint32 getAverageBitRate()
        {
            if (_pH263SpecificAtom != NULL)
            {
                return (_pH263SpecificAtom->getAverageBitRate());
            }
            return 0;
        }

        uint32 getMaxBitRate()
        {
            if (_pH263SpecificAtom != NULL)
            {
                return (_pH263SpecificAtom->getMaxBitRate());
            }
            return 0;
        }

        int16  getWidth()
        {
            return (_width);
        }

        int16  getHeight()
        {
            return (_height);
        }

    private:
        // Reserved constants
        uint8 _reserved[6];
        uint16 _dataReferenceIndex;
        uint32 _reserved1[4]; // = { 0, 0, 0, 0 };
        uint16 _width;
        uint16 _height;
        uint32 _reserved3; // = 0x00480000;
        uint32 _reserved4; // = 0x00480000;
        uint32 _reserved5; // = 0;
        uint16 _reserved6; // = 1;
        uint8  _reserved7[32]; // = 0;
        uint16 _reserved8; // = 24;
        uint16 _reserved9; // = -1; // (16) SIGNED!

        H263SpecficAtom          *_pH263SpecificAtom;
        H263DecoderSpecificInfo  *_pH263decSpecificInfo;
};


#endif // H263SAMPLEENTRY_H_INCLUDED


