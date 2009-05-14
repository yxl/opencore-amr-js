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
#ifndef __MovieFragmentRandomAccessOffsetAtom_H__
#define __MovieFragmentRandomAccessOffsetAtom_H__

#include "fullatom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

class PVA_FF_MfroAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_MfroAtom();

        virtual ~PVA_FF_MfroAtom();

        void	setSize(uint32 size);

        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp);

    private:
        uint32		_atomSize;

        virtual void recomputeSize();

};

#endif
