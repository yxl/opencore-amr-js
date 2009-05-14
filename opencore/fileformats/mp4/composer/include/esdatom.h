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
    This PVA_FF_ESDAtom Class provides the offset between decoding
    time and composition time.
*/


#ifndef __ESDAtom_H__
#define __ESDAtom_H__

#include "fullatom.h"
#include "esdescriptor.h"


class PVA_FF_ESDAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_ESDAtom(int32 streamType, int32 codecType); // Constructor

        virtual ~PVA_FF_ESDAtom();

        // Getting and setting the Mpeg4 VOL header
        PVA_FF_DecoderSpecificInfo *getDecoderSpecificInfo() const
        {
            return _pdescriptor->getDecoderSpecificInfo();
        }
        void addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *pinfo)
        {
            _pdescriptor->addDecoderSpecificInfo(pinfo);
        }

        void setESID(uint16 esid)
        {
            _pdescriptor->setESID(esid);
        }

        void setTargetBitrate(uint32 bitrate)
        {
            _pdescriptor->setTargetBitrate(bitrate);
        }

        void nextSampleSize(uint32 size)
        {
            _pdescriptor->nextSampleSize(size);
        }

        void setMaxBufferSizeDB(uint32 max)
        {
            _pdescriptor->setMaxBufferSizeDB(max);
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        void writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP *_afp)
        {
            _pdescriptor->writeMaxSampleSize(_afp);
        }

    private:
        void init();
        virtual void recomputeSize();

        PVA_FF_ESDescriptor *_pdescriptor;

};


#endif

