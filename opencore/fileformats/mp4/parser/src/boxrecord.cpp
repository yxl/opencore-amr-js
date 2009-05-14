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
/*                         MPEG-4 BoxRecord Class			                     */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    BoxRecord Class
*/

#include "boxrecord.h"
#include "atomutils.h"
#include "isucceedfail.h"

BoxRecord:: BoxRecord(MP4_FF_FILE *fp)
{
    _mp4ErrorCode = EVERYTHING_FINE;
    _success = true;

    uint16 tmp = 0;

    if (!AtomUtils::read16(fp, tmp))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
        return;
    }
    else
    {
        _top = (int16)tmp;
    }


    if (!AtomUtils::read16(fp, tmp))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
        return;
    }
    else
    {
        _left = (int16)tmp;
    }

    if (!AtomUtils::read16(fp, tmp))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
        return;
    }
    else
    {
        _bottom = (int16)tmp;
    }

    if (!AtomUtils::read16(fp, tmp))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
        return;
    }
    else
    {
        _right = (int16)tmp;
    }
}

BoxRecord:: BoxRecord(uint8 *&buf)
{
    _mp4ErrorCode = EVERYTHING_FINE;
    _success = true;

    uint16 tmp = 0;

    if (!AtomUtils::read16(buf, tmp))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
        return;
    }
    else
    {
        _top = (int16)tmp;
    }


    if (!AtomUtils::read16(buf, tmp))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
        return;
    }
    else
    {
        _left = (int16)tmp;
    }

    if (!AtomUtils::read16(buf, tmp))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
        return;
    }
    else
    {
        _bottom = (int16)tmp;
    }

    if (!AtomUtils::read16(buf, tmp))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
        return;
    }
    else
    {
        _right = (int16)tmp;
    }
}

