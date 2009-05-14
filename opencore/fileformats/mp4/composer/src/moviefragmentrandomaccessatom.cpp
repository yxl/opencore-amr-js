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
#define IMPLEMENT_MovieFragmentRandomAccessAtom

#include "moviefragmentrandomaccessatom.h"

typedef Oscl_Vector <PVA_FF_TfraAtom*, OsclMemAllocator>
PVA_FF_TrackFragmentRandomAccessAtomVecType;

// constructor
PVA_FF_MovieFragmentRandomAccessAtom::PVA_FF_MovieFragmentRandomAccessAtom()
        : PVA_FF_Atom(MOVIE_FRAGMENT_RANDOM_ACCESS_ATOM)
{
    // Initialise movie fragment random access offset atom
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_MfroAtom, (), _pMfroAtom);
    _pMfroAtom->setParent(this);

    // initialise track fragment  random access atom list
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackFragmentRandomAccessAtomVecType, (), _pTfraList);

    recomputeSize();
}


//destructor
PVA_FF_MovieFragmentRandomAccessAtom::~PVA_FF_MovieFragmentRandomAccessAtom()
{
    // delete movie fragment random access offset atom
    PV_MP4_FF_DELETE(NULL, PVA_FF_MfroAtom, _pMfroAtom);

    // Delete track fragment  random access atom list
    for (uint32 ii = 0; ii < _pTfraList->size(); ii++)
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_TfraAtom, (*_pTfraList)[ii]);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, PVA_FF_TrackFragmentRandomAccessAtomVecType,
                               Oscl_Vector, _pTfraList);

}

// add new track
void
PVA_FF_MovieFragmentRandomAccessAtom::addTrackFragmentRandomAccessAtom(uint32 trackId)
{
    PVA_FF_TfraAtom*	pTfraAtom;
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TfraAtom, ((trackId)), pTfraAtom);

    _pTfraList->push_back(pTfraAtom);
    pTfraAtom->setParent(this);

    recomputeSize();
}


// return TFRA for given Track id
PVA_FF_TfraAtom*
PVA_FF_MovieFragmentRandomAccessAtom::getTfraAtom(uint32 trackId)
{
    if (_pTfraList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTfraList->size(); ii++)
        {
            if (trackId == ((*_pTfraList)[ii])->getTrackId())
                return ((*_pTfraList)[ii]);
        }
    }

    return NULL;

}

// add new random access entry for given track
void
PVA_FF_MovieFragmentRandomAccessAtom::addSampleEntry(uint32 trackId, uint32 time,
        uint32 moofOffset,	uint32 trafNumber,
        uint32 trunNumber,	uint32 sampleNumber)
{
    if (_pTfraList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTfraList->size(); ii++)
        {
            if (trackId == ((*_pTfraList)[ii])->getTrackId())
                (*_pTfraList)[ii]->addSampleEntry(time, moofOffset, trafNumber,
                                                  trunNumber, sampleNumber);
        }
    }



}


// update moof offset of current moof atom by the given offset for all TFRA atoms
void
PVA_FF_MovieFragmentRandomAccessAtom::updateMoofOffset(uint32 offset)
{
    if (_pTfraList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTfraList->size(); ii++)
        {
            (*_pTfraList)[ii]->updateMoofOffset(offset);
        }
    }

}

// recompute size of atom
void
PVA_FF_MovieFragmentRandomAccessAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    // add size of mfro atom
    size += _pMfroAtom->getSize();

    // add size of Tfra list
    if (_pTfraList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTfraList->size(); ii++)
        {
            size += ((*_pTfraList)[ii])->getSize();
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
PVA_FF_MovieFragmentRandomAccessAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp)
{
    uint32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // render track fragment random access atoms
    if (_pTfraList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTfraList->size(); ii++)
        {
            if (!((*_pTfraList)[ii])->renderToFileStream(fp))
            {
                return false;
            }
            rendered += ((*_pTfraList)[ii])->getSize();
        }
    }

    // Render the movie fragment random access offset atom
    _pMfroAtom->setSize(_size);	// before rendering set the size field in MFRO atom
    if (!_pMfroAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pMfroAtom->getSize();

    return true;
}
