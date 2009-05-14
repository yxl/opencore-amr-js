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
	This PVA_FF_SampleEntry Class is the base class for the HinstSampleEntry,
	PVA_FF_VisualSampleEntry, PVA_FF_AudioSampleEntry. and PVA_FF_MpegSampleEntry classes.
*/


#ifndef __SampleEntry_H__
#define __SampleEntry_H__

#include "fullatom.h"
#include "esdescriptor.h"


class PVA_FF_SampleEntry : public PVA_FF_Atom
{

    public:
        PVA_FF_SampleEntry(uint32 format); // Constructor
        virtual ~PVA_FF_SampleEntry();

        // Member gets and sets
        uint16 getDataReferenceIndex() const
        {
            return _dataReferenceIndex;
        }
        void setDataReferenceIndex(uint16 index)
        {
            _dataReferenceIndex = index;
        }

        // Getting and setting the Mpeg4 VOL header
        virtual PVA_FF_DecoderSpecificInfo *getDecoderSpecificInfo() const
        {
            return NULL;
        }
        virtual void addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *pinfo)
        {
            OSCL_UNUSED_ARG(pinfo);
        };

        //call only in the case of timed text
        virtual void addTextDecoderSpecificInfo(PVA_FF_TextSampleDescInfo *pinfo)
        {
            OSCL_UNUSED_ARG(pinfo);
        };

        virtual void setESID(uint16 esid)
        {
            OSCL_UNUSED_ARG(esid);
        };

    protected:
        // Reserved constants
        uint8 _reserved[6];
        uint16 _dataReferenceIndex;

};


#endif

