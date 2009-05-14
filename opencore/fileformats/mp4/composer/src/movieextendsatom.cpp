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
#define IMPLEMENT_MovieExtendsAtom

#include "movieextendsatom.h"

typedef Oscl_Vector <PVA_FF_TrackExtendsAtom*, OsclMemAllocator> PVA_FF_TrackExtendsAtomVecType;

// constructor
PVA_FF_MovieExtendsAtom::PVA_FF_MovieExtendsAtom()
        : PVA_FF_Atom(MOVIE_EXTENDS_ATOM)
{
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_MovieExtendsHeaderAtom, (), _pMehdAtom);
    _pMehdAtom->setParent(this);

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackExtendsAtomVecType, (), _pTrexAtomVec);

    recomputeSize();

}

//destructor
PVA_FF_MovieExtendsAtom::~PVA_FF_MovieExtendsAtom()
{
    PV_MP4_FF_DELETE(NULL, PVA_FF_MovieExtendsHeaderAtom, _pMehdAtom);

    // Delete trex atoms
    for (uint32 ii = 0; ii < _pTrexAtomVec->size(); ii++)
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_TrackExtendAtom, (*_pTrexAtomVec)[ii]);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, PVA_FF_TrackExtendsAtomVecType, Oscl_Vector, _pTrexAtomVec);

}


// returns pointer to movie extension header atom
PVA_FF_MovieExtendsHeaderAtom*
PVA_FF_MovieExtendsAtom::getMehdAtom()
{
    return _pMehdAtom;
}


// set movie fragment duration
void
PVA_FF_MovieExtendsAtom::setMovieFragmentDuration(uint32 duration)
{
    _pMehdAtom->setFragmentDuration(duration);
}


// update fragment duration
void
PVA_FF_MovieExtendsAtom::updateMovieFragmentDuration(uint32 ts)
{

    _pMehdAtom->updateMovieFragmentDuration(ts);
}

// write fragment duration into target file
void
PVA_FF_MovieExtendsAtom::writeMovieFragmentDuration(MP4_AUTHOR_FF_FILE_IO_WRAP* fp)
{
    _pMehdAtom->writeMovieFragmentDuration(fp);
}

// return fragement duration
uint32
PVA_FF_MovieExtendsAtom::getDuration()
{
    return	_pMehdAtom->getFragmentDuration();
}


// add new trex atom to the list
void
PVA_FF_MovieExtendsAtom::addTrexAtom(PVA_FF_TrackExtendsAtom *pTrexAtom)
{
    _pTrexAtomVec->push_back(pTrexAtom);
    pTrexAtom->setParent(this);

    recomputeSize();
}


// returns trex atom corressponding to track id
PVA_FF_TrackExtendsAtom*
PVA_FF_MovieExtendsAtom::getTrexAtom(uint32 trackId)
{
    if (_pTrexAtomVec->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTrexAtomVec->size(); ii++)
        {
            if (trackId == ((*_pTrexAtomVec)[ii])->getTrackId())
                return ((*_pTrexAtomVec)[ii]);
        }
    }

    return NULL;
}


// Recomputes the size of the current atom by checking all contained atoms
void
PVA_FF_MovieExtendsAtom::recomputeSize()
{
    int32 size = getDefaultSize(); // From base class

    // add size of mehd atom
    size += _pMehdAtom->getSize();

    // add size of Trex atoms
    if (_pTrexAtomVec->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTrexAtomVec->size(); ii++)
        {
            size += ((*_pTrexAtomVec)[ii])->getSize();
        }
    }

    _size = size;

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}


// Write the atom into target file
bool
PVA_FF_MovieExtendsAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp)
{
    uint32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // Render the movie extend header atom
    if (!_pMehdAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += getMehdAtom()->getSize();

    // render track extend atoms
    if (_pTrexAtomVec->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTrexAtomVec->size(); ii++)
        {
            if (!((*_pTrexAtomVec)[ii])->renderToFileStream(fp))
            {
                return false;
            }
            rendered += ((*_pTrexAtomVec)[ii])->getSize();
        }
    }

    return true;
}
