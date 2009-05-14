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
#ifndef FONTTABLEATOM_H_INCLUDED
#define FONTTABLEATOM_H_INCLUDED


#include "atom.h"
#include "atomutils.h"
#include "oscl_file_io.h"

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef FONTRECORD_H_INCLUDED
#include "fontrecord.h"
#endif

//This class is specific for Timed text fileformat
//It stores and sets the no. of type of fontrecords are present

typedef Oscl_Vector<PVA_FF_FontRecord*, OsclMemAllocator> fontRecordVecType;

class PVA_FF_FontTableAtom : public PVA_FF_Atom
{

    public:
        PVA_FF_FontTableAtom();  // Default constructor
        virtual ~PVA_FF_FontTableAtom();  // Destructor

        void setFontListSize(uint16 FontListSize);
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();
        virtual uint32 getSize();
        void setFontRecord(uint16 FontListID, uint16 FontId, int8 FontLength, uint8* FontName);

        PVA_FF_FontRecord * getFontRecordAt(uint16 index)
        {
            if ((index) > _pFontRecordArray->size())
            {
                return NULL;
            }

            if (index < _entryCount)
            {
                return (PVA_FF_FontRecord *)(*_pFontRecordArray)[(int32)index];
            }
            else
            {
                return NULL;
            }
        }

    private:
        uint8* _sample;
        uint16 _entryCount;
        uint16 _fontlistsize;
        Oscl_Vector<PVA_FF_FontRecord *, OsclMemAllocator> *_pFontRecordArray;

};


#endif // FONTTABLEATOM_H_INCLUDED
