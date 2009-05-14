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
/*
    This PVA_FF_SampleTableAtom Class contains all the time and data indexing of the
    media samples in a track.
*/


#define IMPLEMENT_SampleTableAtom

#include "sampletableatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

// Constructor
PVA_FF_SampleTableAtom::PVA_FF_SampleTableAtom(uint32 mediaType,
        int32 codecType,
        uint32 fileAuthoringFlags,
        uint32 protocol,
        uint8 profile,
        uint8 profileComp,
        uint8 level)
        : PVA_FF_Atom(SAMPLE_TABLE_ATOM)
{
    _oInterLeaveMode = false;
    if (fileAuthoringFlags & PVMP4FF_SET_MEDIA_INTERLEAVE_MODE)
    {
        _oInterLeaveMode = true;
    }

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TimeToSampleAtom, (mediaType), _ptimeToSampleAtom);
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_SampleDescriptionAtom, (mediaType, codecType, protocol, profile, profileComp, level), _psampleDescriptionAtom);
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_SampleSizeAtom, (mediaType), _psampleSizeAtom);
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_SampleToChunkAtom, (mediaType, fileAuthoringFlags), _psampleToChunkAtom);
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_ChunkOffsetAtom, (mediaType, fileAuthoringFlags), _pchunkOffsetAtom);

    if (mediaType == MEDIA_TYPE_VISUAL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_SyncSampleAtom, (), _psyncSampleAtom);

        _psyncSampleAtom->setParent(this);
    }
    else
    {
        _psyncSampleAtom = NULL;
    }

    _ptimeToSampleAtom->setParent(this);
    _psampleSizeAtom->setParent(this);
    _pchunkOffsetAtom->setParent(this);
    _psampleToChunkAtom->setParent(this);
    _psampleDescriptionAtom->setParent(this);

    recomputeSize();
}

// Destructor
PVA_FF_SampleTableAtom::~PVA_FF_SampleTableAtom()
{
    // Clean up member atoms
    PV_MP4_FF_DELETE(NULL, PVA_FF_TimeToSampleAtom, _ptimeToSampleAtom);
    PV_MP4_FF_DELETE(NULL, PVA_FF_SampleDescriptionAtom, _psampleDescriptionAtom);
    PV_MP4_FF_DELETE(NULL, PVA_FF_SampleSizeAtom, _psampleSizeAtom);
    PV_MP4_FF_DELETE(NULL, PVA_FF_SampleToChunkAtom, _psampleToChunkAtom);
    PV_MP4_FF_DELETE(NULL, PVA_FF_ChunkOffsetAtom, _pchunkOffsetAtom);

    if (_psyncSampleAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_SyncSampleAtom, _psyncSampleAtom);
    }
}

// Update the member atoms with the next sample received
void
PVA_FF_SampleTableAtom::nextSample(int32 mediaType,
                                   void *psample,
                                   uint32 size,
                                   uint32 ts,
                                   uint8 flags,
                                   uint32 baseOffset,
                                   bool oChunkStart)
{
    // Update timeToSampleAtom with new sample
    _ptimeToSampleAtom->nextSample(ts);

    // Update sampleDescriptionAtom with new sample
    int32 index = _psampleDescriptionAtom->nextSample(size, flags);
    // Update sampleSizeAtom with new sample
    _psampleSizeAtom->nextSample(size);

    // Update sampleToChunk atom with new sample - returns whether or not the
    // new sample fp the beginning of a new chunk
    bool isChunkStart;

    isChunkStart = _psampleToChunkAtom->nextSample(index, size, oChunkStart);

    if (_oInterLeaveMode)
    {
        // Updates chunkOffset atom with new sample
        _pchunkOffsetAtom->nextSample(size, isChunkStart, baseOffset);
    }
    else
    {
        // Updates chunkOffset atom with new sample
        _pchunkOffsetAtom->nextSample(size, isChunkStart);
    }

    // Update the syncSampleAtom with the new sample - cast to IMediaSample
    if (_psyncSampleAtom != NULL)
    {
        _psyncSampleAtom->nextSample(flags);
    }
    OSCL_UNUSED_ARG(mediaType);
    OSCL_UNUSED_ARG(psample);
}

void
PVA_FF_SampleTableAtom::nextTextSample(int32 mediaType,
                                       void *psample,
                                       uint32 size,
                                       uint32 ts,
                                       uint8 flags,
                                       int32 index,
                                       uint32 baseOffset,
                                       bool oChunkStart)
{
    OSCL_UNUSED_ARG(psample);
    OSCL_UNUSED_ARG(mediaType);
    // Update timeToSampleAtom with new sample
    _ptimeToSampleAtom->nextSample(ts);

    // Update sampleDescriptionAtom with new sample
    int32 sdindex = _psampleDescriptionAtom->nextTextSample(size, flags, index);
    // Update sampleSizeAtom with new sample
    _psampleSizeAtom->nextSample(size);

    // Update sampleToChunk atom with new sample - returns whether or not the
    // new sample fp the beginning of a new chunk
    bool isChunkStart;

    isChunkStart = _psampleToChunkAtom->nextSample(sdindex, size, oChunkStart);

    if (_oInterLeaveMode)
    {
        // Updates chunkOffset atom with new sample
        _pchunkOffsetAtom->nextSample(size, isChunkStart, baseOffset);
    }
    else
    {
        // Updates chunkOffset atom with new sample
        _pchunkOffsetAtom->nextSample(size, isChunkStart);
    }

    // Update the syncSampleAtom with the new sample - cast to IMediaSample
    if (_psyncSampleAtom != NULL)
    {
        _psyncSampleAtom->nextSample(flags);
    }


}

void
PVA_FF_SampleTableAtom::nextSample(int32 mediaType,
                                   Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                                   uint32 size,
                                   uint32 ts,
                                   uint8 flags,
                                   uint32 baseOffset,
                                   bool oChunkStart)
{

    // Update timeToSampleAtom with new sample
    _ptimeToSampleAtom->nextSample(ts);

    // Update sampleDescriptionAtom with new sample
    int32 index = _psampleDescriptionAtom->nextSample(size, flags);

    // Update sampleSizeAtom with new sample
    _psampleSizeAtom->nextSample(size);

    // Update sampleToChunk atom with new sample - returns whether or not the
    // new sample fp the beginning of a new chunk
    bool isChunkStart;

    isChunkStart = _psampleToChunkAtom->nextSample(index, size, oChunkStart);

    if (_oInterLeaveMode)
    {
        // Updates chunkOffset atom with new sample
        _pchunkOffsetAtom->nextSample(size, isChunkStart, baseOffset);
    }
    else
    {
        // Updates chunkOffset atom with new sample
        _pchunkOffsetAtom->nextSample(size, isChunkStart);
    }

    // Update the syncSampleAtom with the new sample - cast to IMediaSample
    if (_psyncSampleAtom != NULL)
    {
        _psyncSampleAtom->nextSample(flags);
    }

    OSCL_UNUSED_ARG(mediaType);
    OSCL_UNUSED_ARG(fragmentList);
}

void
PVA_FF_SampleTableAtom::nextTextSample(int32 mediaType,
                                       Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                                       uint32 size,
                                       uint32 ts,
                                       uint8 flags,
                                       int32 index,
                                       uint32 baseOffset,
                                       bool oChunkStart)
{
    OSCL_UNUSED_ARG(mediaType);
    OSCL_UNUSED_ARG(fragmentList);
    // Update timeToSampleAtom with new sample
    _ptimeToSampleAtom->nextSample(ts);

    // Update sampleDescriptionAtom with new sample
    int32 sdindex = _psampleDescriptionAtom->nextTextSample(size, flags, index);

    // Update sampleSizeAtom with new sample
    _psampleSizeAtom->nextSample(size);

    // Update sampleToChunk atom with new sample - returns whether or not the
    // new sample fp the beginning of a new chunk
    bool isChunkStart;

    isChunkStart = _psampleToChunkAtom->nextSample(sdindex, size, oChunkStart);

    if (_oInterLeaveMode)
    {
        // Updates chunkOffset atom with new sample
        _pchunkOffsetAtom->nextSample(size, isChunkStart, baseOffset);
    }
    else
    {
        // Updates chunkOffset atom with new sample
        _pchunkOffsetAtom->nextSample(size, isChunkStart);
    }

    // Update the syncSampleAtom with the new sample - cast to IMediaSample
    if (_psyncSampleAtom != NULL)
    {
        _psyncSampleAtom->nextSample(flags);
    }

}

bool
PVA_FF_SampleTableAtom::reAuthorFirstSample(uint32 size,
        uint32 baseOffset)
{
    /* Update the first chunk offset */
    _pchunkOffsetAtom->reAuthorFirstChunkOffset(baseOffset);

    /* Edit the first sample size */
    return (_psampleSizeAtom->reAuthorFirstSample(size));
}


// in movie fragment mode set the actual duration of
// last sample
void
PVA_FF_SampleTableAtom::updateLastTSEntry(uint32 ts)
{

    _ptimeToSampleAtom->updateLastTSEntry(ts);
}


void
PVA_FF_SampleTableAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    size += _ptimeToSampleAtom->getSize();
    size += _psampleSizeAtom->getSize();
    size += _pchunkOffsetAtom->getSize();
    size += _psampleToChunkAtom->getSize();
    size += _psampleDescriptionAtom->getSize();
    if (getSyncSampleAtom() != NULL)
    {
        size += getSyncSampleAtom()->getSize();
    }

    _size = size;

    // Update parent atom size
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_SampleTableAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!_psampleDescriptionAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _psampleDescriptionAtom->getSize();

    if (!_ptimeToSampleAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _ptimeToSampleAtom->getSize();

    if (!_psampleToChunkAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _psampleToChunkAtom->getSize();

    if (!_psampleSizeAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _psampleSizeAtom->getSize();

    if (!_pchunkOffsetAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pchunkOffsetAtom->getSize();

    if (getSyncSampleAtom() != NULL)
    {
        if (!_psyncSampleAtom->renderToFileStream(fp))
        {
            return false;
        }
        rendered += getSyncSampleAtom()->getSize();
    }

    return true;
}

void
PVA_FF_SampleTableAtom::SetMaxSampleSize(uint32 aSize)
{
    _psampleDescriptionAtom->SetMaxSampleSize(aSize);
}

void
PVA_FF_SampleTableAtom::writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP *_afp)
{
    _psampleDescriptionAtom->writeMaxSampleSize(_afp);
}
