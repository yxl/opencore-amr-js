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
/* PASP Box is used to contain the Horizontal and vertical spacing */

#define IMPLEMENT_AVCConfigurationBox
#include "avcsampleentry.h"
#include "atomutils.h"
#include "atomdefs.h"

#ifndef PASPBOX_H_INCLUDED
#include "paspatom.h"
#endif


PASPBox::PASPBox(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    _mp4ErrorCode = READ_PIXELASPECTRATIO_BOX_FAILED;  //Error Code for pasp

    if (_success)
    {
        _success = false;
        _pparent = NULL;

        if (!AtomUtils::read32(fp, _hSpacing))
        {
            return;
        }
        if (!AtomUtils::read32(fp, _vSpacing))
        {
            return;
        }

        _success = true;
        _mp4ErrorCode = EVERYTHING_FINE;

    }
}



