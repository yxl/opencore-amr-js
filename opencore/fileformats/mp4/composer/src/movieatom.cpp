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
    This PVA_FF_MovieAtom Class fp the main atom class in the MPEG-4 File that stores
    all the meta data about the MPEG-4 presentation.
*/


#define IMPLEMENT_MovieAtom

#include "movieatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

#define MEDIA_TRACK_ID_OFFSET 3

typedef Oscl_Vector<PVA_FF_TrackAtom*, OsclMemAllocator> PVA_FF_TrackAtomVecType;

// Constructor
PVA_FF_MovieAtom::PVA_FF_MovieAtom(uint32 fileAuthoringFlags)
        :   PVA_FF_Atom(MOVIE_ATOM)
{
    _success = true;

    _scalability = -1;
    _fileType = 0;

    // Initializations

    _pAssetInfoCopyRightAtom		= NULL;
    _pAssetInfoAuthorAtom			= NULL;
    _pAssetInfoTitleAtom			= NULL;
    _pAssetInfoDescAtom				= NULL;
    _pAssetInfoPerformerAtom		= NULL;
    _pAssetInfoGenreAtom			= NULL;
    _pAssetInfoRatingAtom			= NULL;
    _pAssetInfoClassificationAtom	= NULL;
    _pAssetInfoKeyWordsAtom			= NULL;
    _pAssetInfoLocationInfoAtom		= NULL;
    _pAssetInfoKeyAlbumAtom			= NULL;
    _pAssetInfoKeyRecordingYearAtom = NULL;

    _oMovieFragmentEnabled = false;
    //Movie Fragment : Enable movie fragment mode and create movie extends atom
    if ((fileAuthoringFlags & PVMP4FF_MOVIE_FRAGMENT_MODE) == (PVMP4FF_MOVIE_FRAGMENT_MODE))
    {
        _oMovieFragmentEnabled = true;
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_MovieExtendsAtom, (), _pMovieExtendsAtom);

    }

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_MovieHeaderAtom, ((uint8)0, (uint32)0, fileAuthoringFlags), _pmovieHeaderAtom);


    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackAtomVecType, (), _pMediaTrackVec);
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackAtomVecType, (), _pmpeg4TrackVec);

    // Create user data atom
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_UserDataAtom, (), _puserDataAtom);
    _puserDataAtom->setParent(this);

    setTimeScale(DEFAULT_PRESENTATION_TIMESCALE); // Set default value for timescale

    recomputeSize();

    _pmovieHeaderAtom->setParent(this);
}

// Destructor - delete the vector(s) of TrackAtoms from the heap
PVA_FF_MovieAtom::~PVA_FF_MovieAtom()
{
    uint32 i;

    PV_MP4_FF_DELETE(NULL, PVA_FF_MovieHeaderAtom, _pmovieHeaderAtom);

    // Delete mpeg4 general tracks
    for (i = 0; i < _pmpeg4TrackVec->size(); i++)
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_TrackAtom, (*_pmpeg4TrackVec)[i]);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, PVA_FF_TrackAtomVecType, Oscl_Vector, _pmpeg4TrackVec);

    // Delete media tracks
    for (i = 0; i < _pMediaTrackVec->size(); i++)
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_TrackAtom, (*_pMediaTrackVec)[i]);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, PVA_FF_TrackAtomVecType, Oscl_Vector, _pMediaTrackVec);

    if (_puserDataAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_UserDataAtom, _puserDataAtom);
    }
    //Movie Fragment : delete mvex atom
    if (_oMovieFragmentEnabled == true)
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_MovieExtendsAtom, _pMovieExtendsAtom);
    }

}

// Add a new PVA_FF_TrackAtom to this container
void
PVA_FF_MovieAtom::addTrackAtom(PVA_FF_TrackAtom *a)
{
    // Set presentation timescale
    a->setTimeScale(getTimeScale());

    switch (a->getMediaType())
    {
        case MEDIA_TYPE_TEXT://added for timed text track
        {
            _fileType |= FILE_TYPE_TIMED_TEXT;
            _pMediaTrackVec->push_back(a);
            a->setParent(this);
        }
        break;
        case MEDIA_TYPE_AUDIO:
        {
            _fileType |= FILE_TYPE_AUDIO;
            _pMediaTrackVec->push_back(a);
            a->setParent(this);
            // add trex atom in movie fragment mode
            if (_oMovieFragmentEnabled == true)
            {
                PVA_FF_TrackExtendsAtom  *pTrexAtom;
                PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackExtendsAtom, (a->getMediaType(), a->getCodecType(), a->getTrackID()), pTrexAtom);
                _pMovieExtendsAtom->addTrexAtom(pTrexAtom);
            }

        }
        break;
        case MEDIA_TYPE_VISUAL:
        {
            _fileType |= FILE_TYPE_VIDEO;
            _scalability += 1;
            _pMediaTrackVec->push_back(a);
            a->setParent(this);

            // add trex atom in movie fragment mode
            if (_oMovieFragmentEnabled == true)
            {
                PVA_FF_TrackExtendsAtom  *pTrexAtom;
                PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackExtendsAtom, (a->getMediaType(), a->getCodecType(), a->getTrackID()), pTrexAtom);
                _pMovieExtendsAtom->addTrexAtom(pTrexAtom);
            }
        }
        break;
        default:
            PV_MP4_FF_DELETE(NULL, PVA_FF_TrackAtom, a);
            break;
    }
    recomputeSize();

}

void
PVA_FF_MovieAtom::setTargetBitRate(uint32 trackID, uint32 bitrate)
{
    PVA_FF_TrackAtom *track = getMediaTrack(trackID);

    if (track != NULL)
    {
        track->setTargetBitrate(bitrate);
    }
    return;
}

void
PVA_FF_MovieAtom::setTimeScale(uint32 trackID, uint32 rate)
{
    PVA_FF_TrackAtom *track = getMediaTrack(trackID);

    if (track != NULL)
    {
        track->setMediaTimeScale(rate);
    }
    return;
}

PVA_FF_TrackAtom*
PVA_FF_MovieAtom::getMpeg4Track(int32 index)
{
    if (index < (int32)_pmpeg4TrackVec->size())
    {
        return (*_pmpeg4TrackVec)[index];
    }
    else
    {
        return NULL;
    }
}


// timescale for ENTIRE presentation
void
PVA_FF_MovieAtom::setTimeScale(uint32 ts)
{
    // Set timescale in movieheader atom
    _pmovieHeaderAtom->setTimeScale(ts);

}


// Get timescale from movieheader atom for ENTIRE presentation
uint32
PVA_FF_MovieAtom::getTimeScale() const
{
    return _pmovieHeaderAtom->getTimeScale();
}

// Recomputes the size of the current atom by checking all contained atoms
void
PVA_FF_MovieAtom::recomputeSize()
{
    int32 size = getDefaultSize(); // From base class

    size += _pmovieHeaderAtom->getSize();

    if (_puserDataAtom != NULL)
    {
        if (_puserDataAtom->getUserDataAtomVecSize() > 0)
        {
            size += _puserDataAtom->getSize();
        }
    }

    if (_pMediaTrackVec != NULL)
    {
        for (uint32 i = 0; i < _pMediaTrackVec->size(); i++)
        {
            if ((*_pMediaTrackVec)[i]->getSampleCount() > 0)
            {
                size += (*_pMediaTrackVec)[i]->getSize();
            }
        }
    }

    // in movie fragment mode add size of mvex
    if (_oMovieFragmentEnabled == true)
    {
        size += _pMovieExtendsAtom->getSize();
    }

    _size = size;

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }

}

void
PVA_FF_MovieAtom::prepareToRender()
{
    uint32 maxTrackDuration = 0;

    uint32 creationTime = _pmovieHeaderAtom->getCreationTime();
    uint32 modTime      = _pmovieHeaderAtom->getModificationTime();

    if (_pMediaTrackVec != NULL)
    {
        for (uint32 i = 0; i < _pMediaTrackVec->size(); i++)
        {
            PVA_FF_TrackAtom *track = (*_pMediaTrackVec)[i];

            PVA_FF_TrackHeaderAtom *tkhdPtr = track->getTrackHeaderAtomPtr();

            if (tkhdPtr != NULL)
            {
                tkhdPtr->setCreationTime(creationTime);
                tkhdPtr->setModificationTime(modTime);
            }

            track->prepareToRender();

            uint32 TrackDuration = track->getDuration();

            if (TrackDuration > maxTrackDuration)
            {
                maxTrackDuration = TrackDuration;
            }
        }

        if (maxTrackDuration > 0)
        {
            _pmovieHeaderAtom->setDuration(maxTrackDuration);
        }
    }

    recomputeSize();

    return;
}

// Render this atom to a file stream
bool
PVA_FF_MovieAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();


    // Render rest of atoms in container
    // Render the movie header
    if (!_pmovieHeaderAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pmovieHeaderAtom->getSize();
    {
        if (_puserDataAtom != NULL)
        {
            if (_puserDataAtom->getUserDataAtomVecSize() > 0)
            {
                if (!_puserDataAtom->renderToFileStream(fp))
                {
                    return false;
                }
            }
        }
    }
    // Render the object descriptor
    if (_pMediaTrackVec != NULL)
    {
        for (uint32 i = 0; i < _pMediaTrackVec->size(); i++)
        {
            if ((*_pMediaTrackVec)[i]->getSampleCount() > 0)
            {
                if (!((*_pMediaTrackVec)[i]->renderToFileStream(fp)))
                {
                    return false;
                }
                rendered += (*_pMediaTrackVec)[i]->getSize();
            }
        }
    }
    // render MVEX atom in movie fragment mode
    if (_oMovieFragmentEnabled == true)
    {
        if (_pMovieExtendsAtom->getDuration() == 0)
        {
            _pMovieExtendsAtom->setMovieFragmentDuration(getDuration());
        }

        if (!_pMovieExtendsAtom->renderToFileStream(fp))
        {
            return false;
        }
        rendered += _pMovieExtendsAtom->getSize();
    }

    return true;
}

void
PVA_FF_MovieAtom::addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *pinfo, int32 trackID)
{
    PVA_FF_TrackAtom *mediaTrack = getMediaTrack(trackID);

    mediaTrack->addDecoderSpecificInfo(pinfo);
}

//added for timed text
void
PVA_FF_MovieAtom::addTextDecoderSpecificInfo(PVA_FF_TextSampleDescInfo *pinfo, int32 trackID)
{
    PVA_FF_TrackAtom *mediaTrack = getMediaTrack(trackID);
    mediaTrack->addTextDecoderSpecificInfo(pinfo);
}

int32
PVA_FF_MovieAtom::getCodecType(uint32 trackID)
{
    PVA_FF_TrackAtom *track = getMediaTrack(trackID);

    if (track != NULL)
    {
        return (track->getCodecType());
    }
    else
    {
        return (-1);
    }
}

PVA_FF_TrackAtom*
PVA_FF_MovieAtom::getMediaTrack(uint32 trackID)
{
    for (uint32 i = 0; i < _pMediaTrackVec->size(); i++)
    {
        PVA_FF_TrackHeaderAtom *ptkhdr = (*_pMediaTrackVec)[i]->getTrackHeaderAtomPtr();

        if (ptkhdr->getTrackID() == trackID)
        {
            return (*_pMediaTrackVec)[i];
        }
    }

    return NULL;
}


void
PVA_FF_MovieAtom::setMaxBufferSizeDB(uint32 trackID, uint32 max)
{
    PVA_FF_TrackAtom *trackAtom = getMediaTrack(trackID);

    if (trackAtom != NULL)
    {
        trackAtom->setMaxBufferSizeDB(max);
    }
}

void
PVA_FF_MovieAtom::addSampleToTrack(uint32 trackID,
                                   uint8 *psample,
                                   uint32 size,
                                   uint32 ts,
                                   uint8 flags,
                                   uint32 baseOffset,
                                   bool oChunkStart)
{
    PVA_FF_TrackAtom *mediaTrack;
    uint32 mediaType = 0;

    mediaTrack = getMediaTrack(trackID);

    if (mediaTrack != NULL)
    {
        mediaType  = mediaTrack->getMediaType();

        uint32 mediaTimeScale = mediaTrack->getMediaTimeScale();
        if (mediaTimeScale != 0)
        {
            uint32 ts_in_milliseconds = (uint32)((ts * 1000.0f) / mediaTimeScale);

            // Add sample to movie header so can update its _duration
            _pmovieHeaderAtom->addSample(ts_in_milliseconds);
        }
    }


    if (mediaType == MEDIA_TYPE_VISUAL)
    {
        // Updates the members with the next sample received
        mediaTrack->nextSample(MEDIA_TYPE_VISUAL,
                               psample,
                               size,
                               ts,
                               flags,
                               baseOffset,
                               oChunkStart);
    }

    if (mediaType == MEDIA_TYPE_AUDIO)
    {
        // Updates the members with the next sample received
        mediaTrack->nextSample(MEDIA_TYPE_AUDIO,
                               psample,
                               size,
                               ts,
                               flags,
                               baseOffset,
                               oChunkStart);
    }
    if (mediaType == MEDIA_TYPE_TEXT)
    {
        // Updates the members with the next sample received
        mediaTrack->nextSample(MEDIA_TYPE_TEXT,
                               psample,
                               size,
                               ts,
                               flags,
                               baseOffset,
                               oChunkStart);
    }
}

void
PVA_FF_MovieAtom::addTextSampleToTrack(uint32 trackID,
                                       uint8 *psample,
                                       uint32 size,
                                       uint32 ts,
                                       uint8 flags,
                                       int32 index,
                                       uint32 baseOffset,
                                       bool oChunkStart)
{
    PVA_FF_TrackAtom *mediaTrack;
    uint32 mediaType = 0;

    mediaTrack = getMediaTrack(trackID);

    if (mediaTrack != NULL)
    {
        mediaType  = mediaTrack->getMediaType();

        uint32 mediaTimeScale = mediaTrack->getMediaTimeScale();
        if (mediaTimeScale != 0)
        {
            uint32 ts_in_milliseconds = (uint32)((ts * 1000.0f) / mediaTimeScale);

            // Add sample to movie header so can update its _duration
            _pmovieHeaderAtom->addSample(ts_in_milliseconds);
        }
    }


    if (mediaType == MEDIA_TYPE_TEXT)
    {
        // Updates the members with the next sample received
        mediaTrack->nextTextSample(MEDIA_TYPE_TEXT,
                                   psample,
                                   size,
                                   ts,
                                   flags,
                                   index,
                                   baseOffset,
                                   oChunkStart);
    }
}

void
PVA_FF_MovieAtom::addSampleToTrack(uint32 trackID,
                                   Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                                   uint32 size,
                                   uint32 ts,
                                   uint8 flags,
                                   uint32 baseOffset,
                                   bool oChunkStart)
{
    PVA_FF_TrackAtom *mediaTrack;
    uint32 mediaType = 0;

    mediaTrack = getMediaTrack(trackID);

    if (mediaTrack != NULL)
    {
        mediaType  = mediaTrack->getMediaType();

        uint32 mediaTimeScale = mediaTrack->getMediaTimeScale();
        if (mediaTimeScale != 0)
        {
            uint32 ts_in_milliseconds = (uint32)((ts * 1000.0f) / mediaTimeScale);

            // Add sample to movie header so can update its _duration
            _pmovieHeaderAtom->addSample(ts_in_milliseconds);
        }
    }

    if (mediaType == MEDIA_TYPE_VISUAL)
    {
        // Updates the members with the next sample received
        mediaTrack->nextSample(MEDIA_TYPE_VISUAL,
                               fragmentList,
                               size,
                               ts,
                               flags,
                               baseOffset,
                               oChunkStart);
    }

    if (mediaType == MEDIA_TYPE_AUDIO)
    {
        // Updates the members with the next sample received
        mediaTrack->nextSample(MEDIA_TYPE_AUDIO,
                               fragmentList,
                               size,
                               ts,
                               flags,
                               baseOffset,
                               oChunkStart);
    }

    if (mediaType == MEDIA_TYPE_TEXT)//added for the support of timed text track
    {
        // Updates the members with the next sample received
        mediaTrack->nextSample(MEDIA_TYPE_AUDIO,
                               fragmentList,
                               size,
                               ts,
                               flags,
                               baseOffset,
                               oChunkStart);
    }

}

void
PVA_FF_MovieAtom::addTextSampleToTrack(uint32 trackID,
                                       Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                                       uint32 size,
                                       uint32 ts,
                                       uint8 flags,
                                       int32 index,
                                       uint32 baseOffset,
                                       bool oChunkStart)
{
    PVA_FF_TrackAtom *mediaTrack;
    uint32 mediaType = 0;

    mediaTrack = getMediaTrack(trackID);

    if (mediaTrack != NULL)
    {
        mediaType  = mediaTrack->getMediaType();

        uint32 mediaTimeScale = mediaTrack->getMediaTimeScale();
        if (mediaTimeScale != 0)
        {
            uint32 ts_in_milliseconds = (uint32)((ts * 1000.0f) / mediaTimeScale);

            // Add sample to movie header so can update its _duration
            _pmovieHeaderAtom->addSample(ts_in_milliseconds);
        }
    }

    if (mediaType == MEDIA_TYPE_TEXT)//added for the support of timed text track
    {
        // Updates the members with the next sample received
        mediaTrack->nextTextSample(MEDIA_TYPE_TEXT,
                                   fragmentList,
                                   size,
                                   ts,
                                   flags,
                                   index,
                                   baseOffset,
                                   oChunkStart);
    }

}

bool
PVA_FF_MovieAtom::reAuthorFirstSampleInTrack(uint32 trackID,
        uint32 size,
        uint32 baseOffset)
{

    PVA_FF_TrackAtom *mediaTrack;

    mediaTrack = getMediaTrack(trackID);

    return (
               mediaTrack->reAuthorFirstSample(size,
                                               baseOffset));
}


void
PVA_FF_MovieAtom::populateCommonMetadataAtoms()
{

    if (_pAssetInfoCopyRightAtom == NULL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoCopyRightAtom, (), _pAssetInfoCopyRightAtom);
        if (_puserDataAtom != NULL)
        {
            _puserDataAtom->addAtom(_pAssetInfoCopyRightAtom);
        }
    }

    if (_pAssetInfoAuthorAtom == NULL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoAuthorAtom, (), _pAssetInfoAuthorAtom);
        if (_puserDataAtom != NULL)
        {
            _puserDataAtom->addAtom(_pAssetInfoAuthorAtom);
        }
    }
    if (_pAssetInfoTitleAtom == NULL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoTitleAtom, (), _pAssetInfoTitleAtom);
        if (_puserDataAtom != NULL)
        {
            _puserDataAtom->addAtom(_pAssetInfoTitleAtom);
        }
    }
    if (_pAssetInfoDescAtom == NULL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoDescAtom, (), _pAssetInfoDescAtom);
        if (_puserDataAtom != NULL)
        {
            _puserDataAtom->addAtom(_pAssetInfoDescAtom);
        }
    }
}


void
PVA_FF_MovieAtom::createAssetInfoAtoms()
{
    populateCommonMetadataAtoms();

    if (_pAssetInfoPerformerAtom == NULL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoPerformerAtom, (), _pAssetInfoPerformerAtom);
        if (_puserDataAtom != NULL)
        {
            _puserDataAtom->addAtom(_pAssetInfoPerformerAtom);
        }
    }

    if (_pAssetInfoGenreAtom == NULL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoGenreAtom, (), _pAssetInfoGenreAtom);
        if (_puserDataAtom != NULL)
        {
            _puserDataAtom->addAtom(_pAssetInfoGenreAtom);
        }
    }

    if (_pAssetInfoRatingAtom == NULL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoRatingAtom, (), _pAssetInfoRatingAtom);
        if (_puserDataAtom != NULL)
        {
            _puserDataAtom->addAtom(_pAssetInfoRatingAtom);
        }
    }

    if (_pAssetInfoClassificationAtom == NULL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoClassificationAtom, (), _pAssetInfoClassificationAtom);
        if (_puserDataAtom != NULL)
        {
            _puserDataAtom->addAtom(_pAssetInfoClassificationAtom);
        }
    }

    if (_pAssetInfoKeyWordsAtom == NULL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoKeyWordsAtom, (), _pAssetInfoKeyWordsAtom);
        if (_puserDataAtom != NULL)
        {
            _puserDataAtom->addAtom(_pAssetInfoKeyWordsAtom);
        }
    }

    if (_pAssetInfoLocationInfoAtom == NULL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoLocationInfoAtom, (), _pAssetInfoLocationInfoAtom);
        if (_puserDataAtom != NULL)
        {
            _puserDataAtom->addAtom(_pAssetInfoLocationInfoAtom);
        }
    }

    if (_pAssetInfoKeyAlbumAtom == NULL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoAlbumAtom, (), _pAssetInfoKeyAlbumAtom);
        if (_puserDataAtom != NULL)
        {
            _puserDataAtom->addAtom(_pAssetInfoKeyAlbumAtom);
        }
    }

    if (_pAssetInfoKeyRecordingYearAtom == NULL)
    {
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_AssetInfoRecordingYearAtom, (), _pAssetInfoKeyRecordingYearAtom);
        if (_puserDataAtom != NULL)
        {
            _puserDataAtom->addAtom(_pAssetInfoKeyRecordingYearAtom);
        }
    }

}

// functions to set and update fragment duration in MVEX atom
void
PVA_FF_MovieAtom::setMovieFragmentDuration()
{

    _pMovieExtendsAtom->setMovieFragmentDuration(getDuration());

}


void
PVA_FF_MovieAtom::updateMovieFragmentDuration(uint32 trackID, uint32 ts)
{
    PVA_FF_TrackAtom *mediaTrack;
    mediaTrack = getMediaTrack(trackID);
    uint32 mediaTimeScale = mediaTrack->getMediaTimeScale();

    if (mediaTimeScale != 0)
    {
        uint32 ts_moviescale = (uint32)(ts * ((float)getTimeScale() / (float)mediaTimeScale));
        _pMovieExtendsAtom->updateMovieFragmentDuration(ts_moviescale);
    }
}

void
PVA_FF_MovieAtom::writeMovieFragmentDuration(MP4_AUTHOR_FF_FILE_IO_WRAP* fp)
{
    _pMovieExtendsAtom->writeMovieFragmentDuration(fp);
}

void
PVA_FF_MovieAtom::SetMaxSampleSize(uint32 aTrackID, uint32 aSize)
{
    PVA_FF_TrackAtom *mediaTrack = NULL;
    mediaTrack = getMediaTrack(aTrackID);
    if (NULL != mediaTrack)
        mediaTrack->SetMaxSampleSize(aSize);
}

void
PVA_FF_MovieAtom::writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP *_afp)
{
    for (uint32 i = 0; i < _pMediaTrackVec->size(); i++)
    {
        PVA_FF_TrackHeaderAtom *ptkhdr = NULL;
        ptkhdr = (*_pMediaTrackVec)[i]->getTrackHeaderAtomPtr();
        if (NULL != ptkhdr)
        {
            uint32 trackID = ptkhdr->getTrackID();
            PVA_FF_TrackAtom *mediaTrack = NULL;
            mediaTrack = getMediaTrack(trackID);
            if (NULL != mediaTrack)
                mediaTrack->writeMaxSampleSize(_afp);
        }
    }
}
