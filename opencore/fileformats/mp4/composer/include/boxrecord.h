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
#ifndef BOXRECORD_H_INCLUDED
#define BOXRECORD_H_INCLUDED


#include "atom.h"
#include "atomutils.h"
#include "oscl_file_io.h"

//This class is specific to the Timed text file format
//It holds the information of the dimensions of text box in which text will be rendered
//on the screen.
class PVA_FF_BoxRecord : public PVA_FF_Atom
{

    public:
        PVA_FF_BoxRecord();  // Default constructor
        PVA_FF_BoxRecord(int16 Top, int16 Bottom, int16 Left, int16 Right);
        virtual ~PVA_FF_BoxRecord() {};  // Destructor

        void setBoxTop(int16 top1);
        void setBoxLeft(int16 left1);
        void setBoxBottom(int16 bottom1);
        void setBoxRight(int16 right1);
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();
        virtual uint32 getSize();

    private:
        uint8* _sample;
        int16 _top;
        int16 _left;
        int16 _bottom;
        int16 _right;

};


#endif // BOXRECORD_H_INCLUDED
