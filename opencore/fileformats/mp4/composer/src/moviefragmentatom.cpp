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
#define IMPLEMENT_MovieFragmentAtom

#include "moviefragmentatom.h"

typedef Oscl_Vector < PVA_FF_TrackFragmentAtom*,
OsclMemAllocator > PVA_FF_TrackFragmentAtomVecType;



// constructor
PVA_FF_MovieFragmentAtom::PVA_FF_MovieFragmentAtom(uint32 sequenceNumber,
        uint32 movieFragmentDuration,
        uint32 interleaveDuration)
        : PVA_FF_Atom(MOVIE_FRAGMENT_ATOM)
{
    _movieFragmentDuration = movieFragmentDuration;
    _interleaveDuration = interleaveDuration;

    // Initialise movie fragment header atom
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_MovieFragmentHeaderAtom, ((sequenceNumber)), _pMfhdAtom);
    _pMfhdAtom->setParent(this);

    // initialise track fragment list
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackFragmentAtomVecType, (), _pTrafList);

    recomputeSize();

}


//destructor
PVA_FF_MovieFragmentAtom::~PVA_FF_MovieFragmentAtom()
{
    // delete header atom
    PV_MP4_FF_DELETE(NULL, PVA_FF_MovieFragmentHeaderAtom, _pMfhdAtom);

    // Delete track fragment atom list
    for (uint32 ii = 0; ii < _pTrafList->size(); ii++)
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_TrackFragmentAtom, (*_pTrafList)[ii]);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, PVA_FF_TrackFragmentAtomVecType, Oscl_Vector, _pTrafList);

}


// adds new track fragment to the moof
void
PVA_FF_MovieFragmentAtom::addTrackFragment(uint32 mediaType, uint32 codecType,
        uint32 trackId, uint32 timescale)
{

    PVA_FF_TrackFragmentAtom*	pTrafAtom;
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackFragmentAtom, ((mediaType), (codecType),
                  (trackId), (_interleaveDuration), (timescale)), pTrafAtom);

    _pTrafList->push_back(pTrafAtom);
    pTrafAtom->setParent(this);

    // no need to recompute size. that is called by trun for parent
    recomputeSize();

    return;
}


// get track fragment atom with given trackId
PVA_FF_TrackFragmentAtom*
PVA_FF_MovieFragmentAtom::getTrackFragment(uint32 trackId)
{
    if (_pTrafList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTrafList->size(); ii++)
        {
            if (trackId == ((*_pTrafList)[ii])->getTrackId())
                return ((*_pTrafList)[ii]);
        }
    }

    return NULL;
}


// add new sample to this fragment ( in turn added to trun)
void
PVA_FF_MovieFragmentAtom::addSampleToFragment(uint32 trackId, uint32 size,
        uint32 ts, uint8 flags, uint32 baseOffset,
        bool otrunStart)
{

    PVA_FF_TrackFragmentAtom*	pTrafAtom = getTrackFragment(trackId);

    pTrafAtom->addSampleToFragment(size, ts, flags, baseOffset, otrunStart);

}


// return the current duration of samples in given track
int32
PVA_FF_MovieFragmentAtom::getTrackFragmentDuration(uint32 trackId)
{
    PVA_FF_TrackFragmentAtom*	pTrafAtom = getTrackFragment(trackId);

    return pTrafAtom->getFragmentDuration();
}

// return duration of longest track
int32
PVA_FF_MovieFragmentAtom::getMaxTrackDuration()
{
    uint32 fragmentDuration = 0;
    if (_pTrafList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTrafList->size(); ii++)
        {
            if (fragmentDuration < ((*_pTrafList)[ii])->getFragmentDuration())
                fragmentDuration = ((*_pTrafList)[ii])->getFragmentDuration();
        }
    }


    return fragmentDuration;
}

// return sequence number of this MOOF
uint32
PVA_FF_MovieFragmentAtom::getSequenceNumber()
{
    return _pMfhdAtom->getSequenceNumber();
}


// return TRAF number for track id
uint32
PVA_FF_MovieFragmentAtom::getTrackFragmentNumber(uint32 trackId)
{
    if (_pTrafList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTrafList->size(); ii++)
        {
            if (trackId == ((*_pTrafList)[ii])->getTrackId())
                return ii + 1;
        }
    }
    return 0;

}


// set header data for current track ( not in use )
void
PVA_FF_MovieFragmentAtom::setTrackFragmentHeaderData(uint32 trackId, uint32 flags)
{
    if (_pTrafList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTrafList->size(); ii++)
        {
            if (trackId == ((*_pTrafList)[ii])->getTrackId())
                (*_pTrafList)[ii]->setTrackFragmentHeaderData(flags);
        }
    }
    recomputeSize();
}


// set base data offset for given track fragment
void
PVA_FF_MovieFragmentAtom::setTrackFragmentBaseDataOffset(uint32 trackId, uint32 offset)
{
    if (_pTrafList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTrafList->size(); ii++)
        {
            if (trackId == ((*_pTrafList)[ii])->getTrackId())
                (*_pTrafList)[ii]->setTrackFragmentBaseDataOffset(offset);
        }
    }
}



// recompute size of atom
void
PVA_FF_MovieFragmentAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    // add size of mfhd atom
    size += _pMfhdAtom->getSize();

    // add size of Track fragments
    if (_pTrafList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTrafList->size(); ii++)
        {
            if (((*_pTrafList)[ii])->getSampleCount() > 0)
            {
                size += ((*_pTrafList)[ii])->getSize();
            }
        }
    }

    _size = size;

    // Update the parent atom size
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}


// write atom in target file
bool
PVA_FF_MovieFragmentAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp)
{

    uint32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // Render the movie fragment header atom
    if (!_pMfhdAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pMfhdAtom->getSize();


    // render track extend atoms
    if (_pTrafList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTrafList->size(); ii++)
        {
            if (((*_pTrafList)[ii])->getSampleCount() > 0)
            {
                if (!((*_pTrafList)[ii])->renderToFileStream(fp))
                {
                    return false;
                }
                rendered += ((*_pTrafList)[ii])->getSize();
            }
        }
    }

    return true;
}
