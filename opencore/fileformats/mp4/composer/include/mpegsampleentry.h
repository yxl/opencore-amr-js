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
    This PVA_FF_MpegSampleEntry is used for hint tracks.
*/


#ifndef __MpegSampleEntry_H__
#define __MpegSampleEntry_H__

#include "sampleentry.h"
#include "esdatom.h"


class PVA_FF_MpegSampleEntry : public PVA_FF_SampleEntry
{

    public:
        PVA_FF_MpegSampleEntry(int32 mediaType); // Constructor
        PVA_FF_MpegSampleEntry(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, int32 mediaType); // Stream-in Constructor
        PVA_FF_MpegSampleEntry(const PVA_FF_MpegSampleEntry &entry); // Copy Constructor
        virtual ~PVA_FF_MpegSampleEntry();

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

        void nextSampleSize(uint32 size)
        {
            _pes->nextSampleSize(size);
        }

        void setMaxBufferSizeDB(uint32 max)
        {
            _pes->setMaxBufferSizeDB(max);
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

        PVA_FF_ESDAtom *_pes;

};


#endif

