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
/*                          MPEG-4 Util: DecoderSpecificInfo                     */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This DecoderSpecificInfo Class that holds the Mpeg4 VOL header for the
	video stream
*/

#define __IMPLEMENT_DecoderSpecificInfo__

#include "decoderspecificinfo.h"

#include "atomutils.h"

// Stream-in Constructor
DecoderSpecificInfo::DecoderSpecificInfo(MP4_FF_FILE *fp, bool o3GPPTrack, bool read_struct)
        : BaseDescriptor(fp, o3GPPTrack)
{
    _pinfo    = NULL;
    _infoSize = 0;

    if (o3GPPTrack)
    {
        return;
    }

    if (_success)
    {
        if (o3GPPTrack)
        {
            _infoSize = _sizeOfClass;
        }
        else
        {
            _infoSize = _sizeOfClass - 1 - getSizeOfSizeField();
        }

        if (_infoSize > 0)
        {
            _pinfo = (uint8 *)oscl_malloc(_infoSize);

            if (read_struct)
            {
                if (!AtomUtils::readByteData(fp, _infoSize, _pinfo))
                {
                    _success = false;
                    _mp4ErrorCode = READ_DECODER_SPECIFIC_INFO_FAILED;
                }
            }
            else
            {
                if (!AtomUtils::readByteData(fp, _infoSize, _pinfo))
                {
                    _success = false;
                    _mp4ErrorCode = READ_DECODER_SPECIFIC_INFO_FAILED;
                }
                else
                {
                    AtomUtils::rewindFilePointerByN(fp, _infoSize);
                }
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_DECODER_SPECIFIC_INFO_FAILED;
        }

    }
    else
    {
        _mp4ErrorCode = READ_DECODER_SPECIFIC_INFO_FAILED;
    }
}

// Destructor
DecoderSpecificInfo::~DecoderSpecificInfo()
{
    if (_pinfo != NULL)
    {
        oscl_free(_pinfo);
    }
}



