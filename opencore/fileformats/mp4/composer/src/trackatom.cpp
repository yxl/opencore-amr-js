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
    This PVA_FF_TrackAtom Class fp the container for a single track in the MPEG-4
    presentation.
*/

#define IMPLEMENT_TrackAtom

#include "trackatom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;
typedef Oscl_Vector<int32, OsclMemAllocator> int32VecType;
typedef Oscl_Vector<uint8, OsclMemAllocator> uint8VecType;

// Constructor
PVA_FF_TrackAtom::PVA_FF_TrackAtom(int32 type,
                                   uint32 id,
                                   uint32 fileAuthoringFlags,
                                   int32 codecType,
                                   uint32 protocol,
                                   uint8 profile,
                                   uint8 profileComp,
                                   uint8 level)
        : PVA_FF_Atom(TRACK_ATOM)
{
    _success = true;
    FIRST_SAMPLE = true;
    _intialTrackTimeOffsetInMilliSeconds = 0;

    _eList = NULL;
    _mediaType = type;

    _codecType = codecType;

    _oInterLeaveMode = false;
    if (fileAuthoringFlags & PVMP4FF_SET_MEDIA_INTERLEAVE_MODE)
    {
        _oInterLeaveMode = true;
    }


    _pUserDataAtom    = NULL;

    if ((codecType == CODEC_TYPE_MPEG4_VIDEO) ||
            (codecType == CODEC_TYPE_AAC_AUDIO))
    {
        _setDecoderSpecificInfoDone = false;
    }
    else
    {
        _setDecoderSpecificInfoDone = true;
    }

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackHeaderAtom, (type, id, (uint8)0, (uint32)0x000001, fileAuthoringFlags), _ptrackHeader);

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_MediaAtom, (type,
                  codecType,
                  fileAuthoringFlags,
                  protocol, profile,
                  profileComp, level),
                  _pmediaAtom);

    _ptrackReference = NULL;
    recomputeSize();

    _ptrackHeader->setParent(this);
    _pmediaAtom->setParent(this);
}

// Destructor
PVA_FF_TrackAtom::~PVA_FF_TrackAtom()
{
    PV_MP4_FF_DELETE(NULL, PVA_FF_TrackHeaderAtom, _ptrackHeader);

    PV_MP4_FF_DELETE(NULL, PVA_FF_MediaAtom, _pmediaAtom);

    if (_ptrackReference != NULL)
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_TrackReferenceAtom, _ptrackReference);
    }

    if (_eList != NULL)
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_EditAtom, _eList);
    }

    if (_pUserDataAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_UserDataAtom, _pUserDataAtom);
    }
}

void
PVA_FF_TrackAtom::nextSample(int32 mediaType,
                             void *psample,
                             uint32 size,
                             uint32 ts,
                             uint8 flags,
                             uint32 baseOffset,
                             bool oChunkStart)
{
    uint32 ts_in_milliseconds = 0;
    uint32 mediaTimeScale = getMediaTimeScale();

    if (mediaTimeScale != 0)
    {
        ts_in_milliseconds = (uint32)((ts * 1000.0f) / mediaTimeScale);

        // Add sample to track header so can update its _duration
        // which is supposed to be in movie time scale
        _ptrackHeader->addSample(ts_in_milliseconds);
    }

    if (FIRST_SAMPLE)
    {
        FIRST_SAMPLE = false;
        if (ts != 0)
        {
            PV_MP4_FF_NEW(fp->auditCB, PVA_FF_EditAtom, (), _eList);

            _eList->setParent(this);

            //Edit durations are sposed to be movie time scale
            _eList->addEmptyEdit(ts_in_milliseconds);

            _intialTrackTimeOffsetInMilliSeconds = ts_in_milliseconds;
        }
        else
        {
            _eList = NULL;
        }
    }

    _pmediaAtom->nextSample(mediaType, psample, size, ts,
                            flags, baseOffset, oChunkStart);
}

void
PVA_FF_TrackAtom::nextTextSample(int32 mediaType,
                                 void *psample,
                                 uint32 size,
                                 uint32 ts,
                                 uint8 flags,
                                 int32 index,
                                 uint32 baseOffset,
                                 bool oChunkStart)
{
    uint32 ts_in_milliseconds = 0;
    uint32 mediaTimeScale = getMediaTimeScale();

    if (mediaTimeScale != 0)
    {
        ts_in_milliseconds = (uint32)((ts * 1000.0f) / mediaTimeScale);

        // Add sample to track header so can update its _duration
        // which is supposed to be in movie time scale
        _ptrackHeader->addSample(ts_in_milliseconds);
    }

    if (FIRST_SAMPLE)
    {
        FIRST_SAMPLE = false;
        if (ts != 0)
        {
            PV_MP4_FF_NEW(fp->auditCB, PVA_FF_EditAtom, (), _eList);

            _eList->setParent(this);

            //Edit durations are sposed to be movie time scale
            _eList->addEmptyEdit(ts_in_milliseconds);

            _intialTrackTimeOffsetInMilliSeconds = ts_in_milliseconds;
        }
        else
        {
            _eList = NULL;
        }
    }

    _pmediaAtom->nextTextSample(mediaType, psample, size, ts,
                                flags, index, baseOffset, oChunkStart);
}

void
PVA_FF_TrackAtom::nextSample(int32 mediaType,
                             Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                             uint32 size,
                             uint32 ts,
                             uint8 flags,
                             uint32 baseOffset,
                             bool oChunkStart)
{
    uint32 ts_in_milliseconds = 0;
    uint32 mediaTimeScale = getMediaTimeScale();

    if (mediaTimeScale != 0)
    {
        ts_in_milliseconds = (uint32)((ts * 1000.0f) / mediaTimeScale);

        // Add sample to track header so can update its _duration
        // which is supposed to be in movie time scale
        _ptrackHeader->addSample(ts_in_milliseconds);
    }

    if (FIRST_SAMPLE)
    {
        FIRST_SAMPLE = false;
        if (ts != 0)
        {
            PV_MP4_FF_NEW(fp->auditCB, PVA_FF_EditAtom, (), _eList);

            _eList->setParent(this);

            //Edit durations are sposed to be movie time scale
            _eList->addEmptyEdit(ts_in_milliseconds);

            _intialTrackTimeOffsetInMilliSeconds = ts_in_milliseconds;
        }
        else
        {
            _eList = NULL;
        }
    }

    _pmediaAtom->nextSample(mediaType, fragmentList, size, ts,
                            flags, baseOffset, oChunkStart);

}

void
PVA_FF_TrackAtom::nextTextSample(int32 mediaType,
                                 Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                                 uint32 size,
                                 uint32 ts,
                                 uint8 flags,
                                 int32 index,
                                 uint32 baseOffset,
                                 bool oChunkStart)
{
    uint32 ts_in_milliseconds = 0;
    uint32 mediaTimeScale = getMediaTimeScale();

    if (mediaTimeScale != 0)
    {
        ts_in_milliseconds = (uint32)((ts * 1000.0f) / mediaTimeScale);

        // Add sample to track header so can update its _duration
        // which is supposed to be in movie time scale
        _ptrackHeader->addSample(ts_in_milliseconds);
    }

    if (FIRST_SAMPLE)
    {
        FIRST_SAMPLE = false;
        if (ts != 0)
        {
            PV_MP4_FF_NEW(fp->auditCB, PVA_FF_EditAtom, (), _eList);

            _eList->setParent(this);

            //Edit durations are sposed to be movie time scale
            _eList->addEmptyEdit(ts_in_milliseconds);

            _intialTrackTimeOffsetInMilliSeconds = ts_in_milliseconds;
        }
        else
        {
            _eList = NULL;
        }
    }

    _pmediaAtom->nextTextSample(mediaType, fragmentList, size, ts,
                                flags, index, baseOffset, oChunkStart);

}

bool
PVA_FF_TrackAtom::reAuthorFirstSample(uint32 size,
                                      uint32 baseOffset)
{
    return(
              _pmediaAtom->reAuthorFirstSample(size,
                                               baseOffset));
}

int32
PVA_FF_TrackAtom::addTrackReference(uint32 ref)
{
    if (_ptrackReference == NULL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackReferenceAtom, (TREF_TYPE_DEPEND), _ptrackReference);
        _ptrackReference->setParent(this);
    }

    if (_ptrackReference != NULL)
    {
        return _ptrackReference->addTrackReference(ref);
    }
    return 0;
}

// Create methods for the optional member atoms
void
PVA_FF_TrackAtom::createTrackReferenceAtom()
{
    // Empty
}

void
PVA_FF_TrackAtom::createEditAtom()
{
    // Empty
}

// Recomputes the size of the current atom by checking all contained atoms
void
PVA_FF_TrackAtom::recomputeSize()
{
    int32 size = getDefaultSize(); // From base class

    if (_pUserDataAtom != NULL)
    {
        size += _pUserDataAtom->getSize();
    }

    if (_eList != NULL)
    {
        size += _eList->getSize();
    }
    size += _ptrackHeader->getSize();
    size += _pmediaAtom->getSize();
    if (_ptrackReference != NULL)
    {
        size += _ptrackReference->getSize();
    }
    _size = size;

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_TrackAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    uint32 creationTime = _ptrackHeader->getCreationTime();
    uint32 modTime      = _ptrackHeader->getModificationTime();

    PVA_FF_MediaHeaderAtom *mdhdPtr = _pmediaAtom->getMediaHeaderAtomPtr();

    mdhdPtr->setCreationTime(creationTime);
    mdhdPtr->setModificationTime(modTime);

    recomputeSize();

    // Render PVA_FF_Atom type and size
    renderAtomBaseMembers(fp);
    rendered += getDefaultSize();

    // Set ESID
    _pmediaAtom->setESID((uint16)(_ptrackHeader->getTrackID()));
    {
        if (_pUserDataAtom != NULL)
        {
            if (!_pUserDataAtom->renderToFileStream(fp))
            {
                return false;
            }
        }
    }
    // Render the track header atom
    if (!_ptrackHeader->renderToFileStream(fp))
    {
        return false;
    }
    rendered += getTrackHeaderAtom().getSize();

    //Render Edit atom
    if (_eList != NULL)
    {
        if (!_eList->renderToFileStream(fp))
        {
            return false;
        }
    }

    // Render trackReference atom if present
    if (_ptrackReference != NULL)
    {
        if (!_ptrackReference->renderToFileStream(fp))
        {
            return false;
        }
    }

    // Render the media atom
    if (!_pmediaAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += getMediaAtom().getSize();

    return true;
}

void
PVA_FF_TrackAtom::prepareToRender()
{
    if (_eList != NULL)
    {
        //subtract any intial time offset
        uint32 segDuration = getDuration();

        segDuration -= _intialTrackTimeOffsetInMilliSeconds;

        _eList->addEditEntry(segDuration, 0, 1);
    }
    recomputeSize();
}

uint32
PVA_FF_TrackAtom::convertTrackDurationToMediaTimeScale(uint32 duration)
{
    uint32 mediaHeaderDuration = 0;

    float timescaleconversionFactor = 0;
    uint32 mediaTimeScale = getMediaTimeScale();

    if (mediaTimeScale >= 1000)
    {
        timescaleconversionFactor = mediaTimeScale / 1000.0f;
    }
    if (timescaleconversionFactor > 0)
    {
        mediaHeaderDuration = (uint32)(duration * timescaleconversionFactor);
    }
    else
    {
        float temp = (float)(duration * mediaTimeScale);
        mediaHeaderDuration = (uint32)(temp / 1000.00f);
    }
    return (mediaHeaderDuration);
}

// in movie fragment mode set the actual duration of
// last sample
void
PVA_FF_TrackAtom::updateLastTSEntry(uint32 ts)
{
    uint32 ts_in_milliseconds = 0;
    uint32 mediaTimeScale = getMediaTimeScale();
    if (mediaTimeScale != 0)
    {
        ts_in_milliseconds = (uint32)((ts * 1000.0f) / mediaTimeScale);
        _ptrackHeader->updateLastTSEntry(ts_in_milliseconds);
    }

    _pmediaAtom->updateLastTSEntry(ts);

}

void
PVA_FF_TrackAtom::SetMaxSampleSize(uint32 aSize)
{
    _pmediaAtom->SetMaxSampleSize(aSize);
}

void
PVA_FF_TrackAtom::writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP *_afp)
{
    _pmediaAtom->writeMaxSampleSize(_afp);
}

void
PVA_FF_TrackAtom::setVideoParams(uint32 frame_width, uint32 frame_height)
{
    if (_ptrackHeader != NULL)
        _ptrackHeader->setVideoWidthHeight((int16)frame_width, (int16)frame_height);

    if (_pmediaAtom != NULL)
        _pmediaAtom->setVideoParams(frame_width, frame_height);
}
