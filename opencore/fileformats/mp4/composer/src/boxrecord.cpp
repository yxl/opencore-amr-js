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
#include "atom.h"
#include "atomutils.h"
#include "oscl_file_io.h"
#include "boxrecord.h"
#include"a_atomdefs.h"




PVA_FF_BoxRecord::PVA_FF_BoxRecord() : PVA_FF_Atom(FourCharConstToUint32('t', 't', 's', 'b'))
{
    _top = 0;
    _left = 0;
    _bottom = 0;
    _right = 0;
    recomputeSize();
}

void PVA_FF_BoxRecord::setBoxTop(int16 top1)
{
    if (_top == 0)
    {
        _top = top1;
    }
}

void PVA_FF_BoxRecord::setBoxLeft(int16 left1)
{
    if (_left == 0)
    {
        _left = left1;
    }
}

void PVA_FF_BoxRecord::setBoxBottom(int16 bottom1)
{
    if (_bottom == 0)
    {
        _bottom = bottom1;
    }
}

void PVA_FF_BoxRecord::setBoxRight(int16 right1)
{
    if (_right == 0)
    {
        _right = right1;
    }
}


bool PVA_FF_BoxRecord::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!PVA_FF_AtomUtils::render16(fp, _top))
    {
        return false;
    }
    rendered += 2;
    if (!PVA_FF_AtomUtils::render16(fp, _left))
    {
        return false;
    }
    rendered += 2;
    if (!PVA_FF_AtomUtils::render16(fp, _bottom))
    {
        return false;
    }
    rendered += 2;
    if (!PVA_FF_AtomUtils::render16(fp, _right))
    {
        return false;
    }
    rendered += 2;

    return true;
}

void PVA_FF_BoxRecord::recomputeSize()
{
    int32 size = 0;

    size += 2;
    size += 2;
    size += 2;
    size += 2;

    _size = size;

    // Update size of parent
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

uint32 PVA_FF_BoxRecord::getSize()
{
    recomputeSize();
    return (_size);
}
