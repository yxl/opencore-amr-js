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
/*                            MPEG-4 Movie Extends Atom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
*/

#define IMPLEMENT_MovieExtendsAtom

#include "movieextendsatom.h"
#include "atomdefs.h"
#include "atomutils.h"


typedef Oscl_Vector<TrackExtendsAtom*, OsclMemAllocator> trackExtendsAtomVecType;
// Constructor
MovieExtendsAtom::MovieExtendsAtom(MP4_FF_FILE *fp,
                                   uint32 size,
                                   uint32 type)
        : Atom(fp, size, type)
{
    _pMovieExtendsHeaderAtom		= NULL;

    PV_MP4_FF_NEW(fp->auditCB, trackExtendsAtomVecType, (), _pTrackExtendsAtomVec);

    uint32 count = size - DEFAULT_ATOM_SIZE;

    if (_success)
    {
        while (count > 0)
        {
            uint32 atomType = UNKNOWN_ATOM;
            uint32 atomSize = 0;

            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            if (atomType == MOVIE_EXTENDS_HEADER_ATOM)
            {
                if (_pMovieExtendsHeaderAtom == NULL)
                {
                    PV_MP4_FF_NEW(fp->auditCB, MovieExtendsHeaderAtom, (fp, atomSize, atomType), _pMovieExtendsHeaderAtom);
                    if (!_pMovieExtendsHeaderAtom->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = READ_MOVIE_EXTENDS_HEADER_FAILED;
                        return;
                    }
                    count -= _pMovieExtendsHeaderAtom->getSize();
                }
                else
                {
                    //duplicate atom
                    count -= atomSize;
                    atomSize -= DEFAULT_ATOM_SIZE;
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                }
            }
            else if (atomType == TRACK_EXTENDS_ATOM)
            {
                TrackExtendsAtom *pTrackExtendsAtom = NULL;
                PV_MP4_FF_NEW(fp->auditCB, TrackExtendsAtom, (fp, atomSize, atomType), pTrackExtendsAtom);
                if (!pTrackExtendsAtom->MP4Success())
                {
                    _success = false;
                    PV_MP4_FF_DELETE(NULL, TrackExtendsAtom, pTrackExtendsAtom);
                    _mp4ErrorCode = READ_TRACK_EXTENDS_ATOM_FAILED;
                    return;
                }
                count -= pTrackExtendsAtom->getSize();
                _pTrackExtendsAtomVec->push_back(pTrackExtendsAtom);
            }

        }
    }
    else
    {
        _mp4ErrorCode = READ_MOVIE_EXTENDS_ATOM_FAILED;
    }
}

MovieExtendsAtom::~MovieExtendsAtom()
{
    if (_pMovieExtendsHeaderAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, MovieExtendsHeaderAtom, _pMovieExtendsHeaderAtom);
        _pMovieExtendsHeaderAtom = NULL;
    }
    for (uint32 i = 0; i < _pTrackExtendsAtomVec->size(); i++)
    {
        PV_MP4_FF_DELETE(NULL, TrackExtendsAtom, (*_pTrackExtendsAtomVec)[i]);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, trackExtendsAtomVecType, Oscl_Vector, _pTrackExtendsAtomVec);

}
