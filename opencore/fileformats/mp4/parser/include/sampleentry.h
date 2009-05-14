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
/*                           MPEG-4 SampleEntry Class                            */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
	This SampleEntry Class is the base class for the HinstSampleEntry,
	VisualSampleEntry, AudioSampleEntry. and MpegSampleEntry classes.
*/


#ifndef SAMPLEENTRY_H_INCLUDED
#define SAMPLEENTRY_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

class ESDescriptor;
class DecoderSpecificInfo;

class SampleEntry : public Atom
{

    public:
        SampleEntry(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~SampleEntry();

        // Member gets and sets
        uint16 getDataReferenceIndex() const
        {
            return _dataReferenceIndex;
        }

        virtual uint32 getESID() const
        {
            return 0;    // Should get overridden
        }
        virtual const ESDescriptor *getESDescriptor() const
        {
            return NULL;    // Should get overridden
        }

        // Getting and setting the Mpeg4 VOL header
        virtual DecoderSpecificInfo *getDecoderSpecificInfo() const ;
        virtual uint8 getObjectTypeIndication() const ;
        // Get the max size buffer needed to retrieve the media samples
        virtual uint32 getMaxBufferSizeDB() const;
        virtual uint32 getAverageBitrate() const;
        virtual uint32 getMaxBitrate() const;
        virtual uint16 getWidth() const;
        virtual uint16 getHeight() const;


    protected:
        // Reserved constants
        uint8 _reserved[6];
        uint16 _dataReferenceIndex;
};

#endif // SAMPLEENTRY_H_INCLUDED


