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
/**
 * This PVA_FF_ISucceedFail Class is the base class for all other classes here (or will
 * be SOON) - this provides teh ability to check for failure when reading in an mp4 file
 */

#ifndef A_ISUCCEEDFAIL_H_INCLUDED
#define A_ISUCCEEDFAIL_H_INCLUDED

#include "oscl_types.h"

typedef enum {  DEFAULT_ERROR	= -1,
                EVERYTHING_FINE,
                READ_FAILED,
                READ_USER_DATA_ATOM_FAILED,
                READ_PV_USER_DATA_ATOM_FAILED,
                READ_MEDIA_DATA_ATOM_FAILED,
                READ_MOVIE_ATOM_FAILED,
                READ_TRACK_ATOM_FAILED,
                READ_OBJECT_DESCRIPTOR_ATOM_FAILED,
                DUPLICATE_MOVIE_ATOMS,
                DUPLICATE_OBJECT_DESCRIPTORS,
                DUPLICATE_MOVIE_HEADERS,
                READ_UNKNOWN_ATOM,
                MEMORY_ALLOCATION_FAILED
             } MP4_ERROR_CODE;

class PVA_FF_ISucceedFail
{

    public:
        bool Mp4Success()
        {
            return _success;
        }
        bool MP4Success()
        {
            return _success;
        }
        int GetMP4Error()
        {
            return _mp4ErrorCode;
        }

    protected:
        bool _success;
        int _mp4ErrorCode;
};

#endif
