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
#define IMPLEMENT_TrackFragmentAtom

#include "trackfragmentatom.h"
#include "trackfragmentrunatom.h"
typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;
typedef Oscl_Vector<uint8, OsclMemAllocator> uint8VecType;


typedef Oscl_Vector <PVA_FF_TrackFragmentRunAtom*, OsclMemAllocator>
PVA_FF_TrackFragmentRunAtomVecType;

// constructor
PVA_FF_TrackFragmentAtom::PVA_FF_TrackFragmentAtom(uint32 mediaType,
        uint32 codecType,
        uint32 trackId,
        uint32 interleaveDuration,
        uint32 timescale)
        : PVA_FF_Atom(TRACK_FRAGMENT_ATOM)
{
    _mediaType = mediaType;
    _codecType = codecType;

    // no interleaving flag, as movie fragments are there in interleaving mode
    _timescale = timescale;
    _firstEntry = true;
    _prevTS = 0;
    _interleaveDuration = interleaveDuration;
    _fragmentDuration = 0;
    _ofirstTrun = true;

    // intialise track fragment header atom
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackFragmentHeaderAtom, (trackId), _pTfhdAtom);

    // initialise track run vector
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackFragmentRunAtomVecType, (), _pTrunList);

    recomputeSize();

}


//destructor
PVA_FF_TrackFragmentAtom::~PVA_FF_TrackFragmentAtom()
{
    // delete track fragment header
    PV_MP4_FF_DELETE(NULL, PVA_FF_TrackFragmentHeaderAtom, _pTfhdAtom);

    // Delete trum atom list
    for (uint32 ii = 0; ii < _pTrunList->size(); ii++)
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_TrackFragmentRunAtom, (*_pTrunList)[ii]);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, PVA_FF_TrackFragmentRunAtomVecType,
                               Oscl_Vector, _pTrunList);

}


// add new TRUN in the list
PVA_FF_TrackFragmentRunAtom*
PVA_FF_TrackFragmentAtom::addTrackRun()
{
    PVA_FF_TrackFragmentRunAtom	*pTrunAtom;

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackFragmentRunAtom, (), pTrunAtom);

    _pTrunList->push_back(pTrunAtom);
    pTrunAtom->setParent(this);

    recomputeSize();

    return pTrunAtom;
}


// add new sample into track fragment inturn into TRUN
void
PVA_FF_TrackFragmentAtom::addSampleToFragment(uint32 size, uint32 ts, uint8 flags,
        uint32 baseOffset, bool otrunStart)
{

    if (otrunStart == false)
    {
        // add sample to current Trun
        PVA_FF_TrackFragmentRunAtom*	pTrunAtom;
        pTrunAtom = _pTrunList->back();

        pTrunAtom->addSample(size, ts, flags);
    }
    else
    {
        // make new trun
        PVA_FF_TrackFragmentRunAtom*	pTrunAtom = addTrackRun();

        if (_ofirstTrun)
        {
            _ofirstTrun = false;
            setTrackFragmentBaseDataOffset(baseOffset);
            pTrunAtom->setDataOffset(0);
        }
        else
        {
            uint32 offset = _pTfhdAtom->getBaseDataOffset();

            pTrunAtom->setDataOffset(baseOffset - offset);
        }
        pTrunAtom->addSample(size, ts, flags);
    }

    // update track fragment duration
    if (_firstEntry)
    {
        _firstEntry = false;
        _prevTS = ts;
    }
    else
    {
        uint32 delta = ts - _prevTS;
        _prevTS = ts;
        _fragmentDuration = _fragmentDuration + (uint32)((1000.0f / _timescale) * delta + 0.5f);
    }

}


void
PVA_FF_TrackFragmentAtom::setTrackFragmentHeaderData(uint32 flags)
{

    // currently default headers are there, this function arguments will be defined later
    OSCL_UNUSED_ARG(flags);
    _pTfhdAtom->setHeaderData();

}


// set base data offset of track fragment
void
PVA_FF_TrackFragmentAtom::setTrackFragmentBaseDataOffset(uint64 offset)
{
    _pTfhdAtom->setBaseDataOffset(offset);
}


// recompute size of atom
void
PVA_FF_TrackFragmentAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    // add size of tfhd atom
    size += _pTfhdAtom->getSize();

    // add size of Track run atoms
    if (_pTrunList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTrunList->size(); ii++)
        {
            size += ((*_pTrunList)[ii])->getSize();
        }
    }

    _size = size;

    // Update the parent atom size
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}


// get functions
uint32
PVA_FF_TrackFragmentAtom::getTrackId()
{

    return _pTfhdAtom->getTrackId();
}

uint32
PVA_FF_TrackFragmentAtom::getFragmentDuration()
{

    return _fragmentDuration;
}

uint32
PVA_FF_TrackFragmentAtom::getTrunNumber()
{
    return _pTrunList->size();
}


// updates the duration of last sample in TRUN
void
PVA_FF_TrackFragmentAtom::updateLastTSEntry(uint32 ts)
{
    int32 size = _pTrunList->size();

    if (size > 0)
    {
        (*_pTrunList)[size -1]->updateLastTSEntry(ts);
    }
    return;
}

// write atom to target file
bool
PVA_FF_TrackFragmentAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp)
{
    uint32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // Render the movie fragment header atom
    if (!_pTfhdAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pTfhdAtom->getSize();


    // render track extend atoms
    if (_pTrunList->size() != 0)
    {
        for (uint32 ii = 0; ii < _pTrunList->size(); ii++)
        {
            if (!((*_pTrunList)[ii])->renderToFileStream(fp))
            {
                return false;
            }
            rendered += ((*_pTrunList)[ii])->getSize();
        }
    }

    return true;
}
