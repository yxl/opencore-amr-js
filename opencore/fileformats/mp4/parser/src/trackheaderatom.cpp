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
/*                         MPEG-4 TrackHeaderAtom Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This TrackHeaderAtom Class specifies the characteristics of a single MPEG-4
    track.
*/


#define IMPLEMENT_TrackHeaderAtom

#include "trackheaderatom.h"
#include "atomutils.h"
#include "atomdefs.h"

// Stream-in Constructor
TrackHeaderAtom::TrackHeaderAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    _layer			= 0;
    _alternateGroup	= 0;
    _width			= 0;
    _height		    = 0;
    _tx			    = 0;
    _ty			    = 0;

    if (_success)
    {
        _pparent = NULL;
        if (getVersion() == 1)
        {
            if (!AtomUtils::read64(fp, _creationTime64))
                _success = false;
            if (!AtomUtils::read64(fp, _modificationTime64))
                _success = false;
            if (!AtomUtils::read32(fp, _trackID))
                _success = false;

            if (!AtomUtils::read32(fp, _reserved1))
                _success = false;

            if (!AtomUtils::read64(fp, _duration64))
                _success = false;
        }
        else
        {
            if (!AtomUtils::read32(fp, _creationTime))
                _success = false;
            if (!AtomUtils::read32(fp, _modificationTime))
                _success = false;
            if (!AtomUtils::read32(fp, _trackID))
                _success = false;

            if (!AtomUtils::read32(fp, _reserved1))
                _success = false;

            if (!AtomUtils::read32(fp, _duration))
                _success = false;

        }

        if (!AtomUtils::read32(fp, _reserved2[0]))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved2[1]))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved2[2]))
            _success = false;
        if (!AtomUtils::read16(fp, _reserved3))
            _success = false;
        if (!AtomUtils::read16(fp, _reserved4))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved5[0]))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved5[1]))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved5[2]))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved5[3]))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved5[4]))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved5[5]))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved5[6]))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved5[7]))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved5[8]))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved6))
            _success = false;
        if (!AtomUtils::read32(fp, _reserved7))
            _success = false;


        if (_success)
        {
            if (_reserved3 == 0x0100)
            {
                // Check flags for audio track
                _mediaType = MEDIA_TYPE_AUDIO;
            }
            else if ((_reserved6 == 0x01400000) && (_reserved7 == 0x00f00000))
            {
                // Check flags for visual track
                _mediaType = MEDIA_TYPE_VISUAL;
            }
            else
            {
                /*
                 * Store these parameters, just in case
                 * the track later on turn out to be a text one
                 * Also Width, Height, XOffset & YOffsets are stored
                 * as 16.16 fixed point values, with lower 16 bits
                 * always set to zero
                 */

                _width  = ((_reserved6 >> 16) & 0xFFFF);
                _height = ((_reserved7 >> 16) & 0xFFFF);

                _layer  = (int16)((_reserved2[2] >> 16) & 0xFF);
                _alternateGroup =
                    (uint16)((_reserved2[2]) & 0xFF);

                _tx = ((_reserved5[6] >> 16) & 0xFFFF);
                _ty = ((_reserved5[7] >> 16) & 0xFFFF);

                _mediaType = MEDIA_TYPE_UNKNOWN;
            }
        }
        else
        {
            _mp4ErrorCode = READ_TRACK_HEADER_ATOM_FAILED;
        }
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_TRACK_HEADER_ATOM_FAILED;
    }

}

// Destructor
TrackHeaderAtom::~TrackHeaderAtom()
{
    // Empty
}

