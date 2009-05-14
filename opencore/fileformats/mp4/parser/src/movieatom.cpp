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
/*                            MPEG-4 MovieAtom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MovieAtom Class is the main atom class in the MPEG-4 File that stores
    all the meta data about the MPEG-4 presentation.
*/


#define IMPLEMENT_MovieAtom

#include "movieatom.h"
#include "atomutils.h"
#include "atomdefs.h"
#include "amrdecoderspecificinfo.h"

//HEADER FILES REQD FOR MULTIPLE SAMPLE RETRIEVAL API
#include "oscl_media_data.h"
#include "pv_gau.h"
#include "media_clock_converter.h"

typedef Oscl_Vector<TrackAtom*, OsclMemAllocator> trackAtomVecType;

// Stream-in ctor
OSCL_EXPORT_REF MovieAtom::MovieAtom(MP4_FF_FILE *fp,
                                     OSCL_wString& filename,
                                     uint32 size,
                                     uint32 type,
                                     bool oPVContent,
                                     bool oPVContentDownloadable,
                                     uint32 parsingMode)
        : Atom(fp, size, type)
{
    PV_MP4_FF_NEW(fp->auditCB, trackAtomVecType, (), _ptrackArray);
    _pmovieHeaderAtom = NULL;
    _pobjectDescriptorAtom = NULL;
    _pUserDataAtom = NULL;
    _pMovieExtendsAtom = NULL;
    _isMovieFragmentPresent = false;
    _oVideoTrackPresent = false;



    if (_success)
    {
        _scalability = -1;
        _fileType = 0;

        // initialization
        _pparent = NULL;

        // Generalized so can read in in ANY ORDER!!!
        int32 count = _size - DEFAULT_ATOM_SIZE;

        uint32 atomType = UNKNOWN_ATOM;
        uint32 atomSize = 0;

        //top level below the "moov" => mvhd trak iods udta
        while (((atomType == MOVIE_HEADER_ATOM) ||
                (atomType == OBJECT_DESCRIPTOR_ATOM) ||
                (atomType == TRACK_ATOM) ||
                (atomType == USER_DATA_ATOM) ||
                (atomType == FREE_SPACE_ATOM) ||
                (atomType == UUID_ATOM) ||
                (atomType == OMADRM_KMS_BOX) ||
                (atomType == MOVIE_EXTENDS_ATOM) ||
                (atomType == UNKNOWN_ATOM)) &&
                (count > 0))
        {
            uint32 currPtr = AtomUtils::getCurrentFilePosition(fp);
            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            if (atomType == USER_DATA_ATOM)
            {
                if (_pUserDataAtom == NULL)
                {
                    PV_MP4_FF_NEW(fp->auditCB, UserDataAtom, (fp, atomSize, atomType), _pUserDataAtom);

                    if (!_pUserDataAtom->MP4Success())
                    {
                        AtomUtils::seekFromStart(fp, currPtr);
                        AtomUtils::seekFromCurrPos(fp, atomSize);
                        PV_MP4_FF_DELETE(NULL, UserDataAtom, _pUserDataAtom);
                        _pUserDataAtom = NULL;
                        count -= atomSize;
                    }
                    else
                    {
                        _pUserDataAtom->setParent(this);
                        count -= _pUserDataAtom->getSize();
                    }
                }
                else
                {
                    //at most one udat atom allowed
                    if (atomSize < DEFAULT_ATOM_SIZE)
                    {
                        _success = false;
                        _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                        break;
                    }
                    if (count < (int32)atomSize)
                    {
                        _success = false;
                        _mp4ErrorCode = READ_FAILED;
                        break;
                    }
                    count -= atomSize;
                    atomSize -= DEFAULT_ATOM_SIZE;
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                }
            }
            else if ((atomType == FREE_SPACE_ATOM) ||
                     (atomType == UUID_ATOM) ||
                     (atomType == UNKNOWN_ATOM))
            {
                if (atomSize < DEFAULT_ATOM_SIZE)
                {
                    _success = false;
                    _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                    break;
                }
                if (count < (int32)atomSize)
                {
                    _success = false;
                    _mp4ErrorCode = READ_FAILED;
                    break;
                }
                count -= atomSize;
                atomSize -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, atomSize);
            }
            else if (atomType == MOVIE_HEADER_ATOM)
            {
                // mvhd
                if (_pmovieHeaderAtom == NULL)
                {
                    PV_MP4_FF_NEW(fp->auditCB, MovieHeaderAtom, (fp, atomSize, atomType), _pmovieHeaderAtom);

                    if (!_pmovieHeaderAtom->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = _pmovieHeaderAtom->GetMP4Error();
                        break;
                    }
                    _pmovieHeaderAtom->setParent(this);
                    count -= _pmovieHeaderAtom->getSize();
                }
                else
                {
                    _success = false;
                    _mp4ErrorCode = DUPLICATE_MOVIE_HEADERS;
                    break;
                }
            }
            else if (atomType == MOVIE_EXTENDS_ATOM)
            {
                if (_pMovieExtendsAtom == NULL)
                {
                    PV_MP4_FF_NEW(fp->auditCB, MovieExtendsAtom, (fp, atomSize, atomType), _pMovieExtendsAtom);

                    if (!_pMovieExtendsAtom->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = _pMovieExtendsAtom->GetMP4Error();
                        break;
                    }
                    _pMovieExtendsAtom->setParent(this);
                    count -= _pMovieExtendsAtom->getSize();
                    _isMovieFragmentPresent = true;
                }
                else
                {
                    //duplicate atom
                    count -= atomSize;
                    atomSize -= DEFAULT_ATOM_SIZE;
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                }
            }
            else if (atomType == OBJECT_DESCRIPTOR_ATOM)
            {
                // iods
                if (_pobjectDescriptorAtom == NULL)
                {
                    PV_MP4_FF_NEW(fp->auditCB, ObjectDescriptorAtom,
                                  (fp, atomSize, atomType), _pobjectDescriptorAtom);

                    if (!_pobjectDescriptorAtom->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = _pobjectDescriptorAtom->GetMP4Error();
                        break;
                    }
                    _pobjectDescriptorAtom->setParent(this);
                    count -= _pobjectDescriptorAtom->getSize();
                }
                else
                {
                    _success = false;
                    _mp4ErrorCode = DUPLICATE_OBJECT_DESCRIPTORS;
                    break;
                }
            }
            else if (atomType == TRACK_ATOM)
            {
                TrackAtom *track = NULL;
                // trak
                if (oPVContent)
                {
                    // Read in and add all the track atoms
                    PV_MP4_FF_NEW(fp->auditCB,
                                  TrackAtom,
                                  (fp, filename, atomSize,
                                   atomType, oPVContent,
                                   oPVContentDownloadable,
                                   parsingMode),
                                  track);


                    if (track->MP4Success())
                    {
                        count -= track->getSize();
                        addTrackAtom(track);
                    }
                    else
                    {
                        _success = false;
                        _mp4ErrorCode = track->GetMP4Error();
                        if (track != NULL)
                        {
                            PV_MP4_FF_DELETE(NULL, TrackAtom, track);
                            track = NULL;
                        }
                        break;
                    }
                }
                else
                {
                    // Read in and add all the track atoms
                    uint32 currPos = AtomUtils::getCurrentFilePosition(fp);
                    PV_MP4_FF_NEW(fp->auditCB,
                                  TrackAtom,
                                  (fp, filename, atomSize,
                                   atomType, false,
                                   false,
                                   parsingMode),
                                  track);

                    if (track->MP4Success())
                    {
                        count -= track->getSize();
                        addTrackAtom(track);
                    }
                    else
                    {
                        count -= atomSize;
                        atomSize -= DEFAULT_ATOM_SIZE;
                        currPos += atomSize;
                        AtomUtils::seekFromStart(fp, currPos);

                        if (track != NULL)
                        {
                            PV_MP4_FF_DELETE(NULL, TrackAtom, track);
                            track = NULL;
                        }
                    }
                }

                /* max limit- 1024 tracks*/
                if ((_ptrackArray->size()) > MAX_LIMIT_FOR_NUMBER_OF_TRACKS)
                {
                    _success = false;
                    _mp4ErrorCode = EXCEED_MAX_LIMIT_SUPPORTED_FOR_TOTAL_TRACKS;
                    return ;
                }
            }
        }

        // IF SUCCESS IS FALSE, RETURN RIGHT AWAY
        if (_success)
        {
            if (NULL == _pmovieHeaderAtom)
            {
                _success = false;
                _mp4ErrorCode = NO_MOVIE_HEADER_ATOM_PRESENT;
                return;
            }
            else
            {
                uint32 ts =	_pmovieHeaderAtom->getTimeScale();
                if (NULL != _ptrackArray)
                {
                    for (uint i = 0; i < _ptrackArray->size(); i++)
                    {
                        TrackAtom *trackAtom = (*_ptrackArray)[i];
                        trackAtom->NEWsetTrackTSOffset(ts);
                    }
                }
            }
        }
    }
    else
    {
        _mp4ErrorCode = READ_MOVIE_ATOM_FAILED;
    }
}


// Destructor - delete the vector(s) of TrackAtoms from the heap
OSCL_EXPORT_REF MovieAtom::~MovieAtom()
{
    uint32 i;

    if (_pmovieHeaderAtom != NULL)
        PV_MP4_FF_DELETE(NULL, MovieHeaderAtom, _pmovieHeaderAtom);

    if (_pobjectDescriptorAtom != NULL)
        PV_MP4_FF_DELETE(NULL, ObjectDescriptorAtom, _pobjectDescriptorAtom);

    if (_pUserDataAtom != NULL)
        PV_MP4_FF_DELETE(NULL, UserDataAtom, _pUserDataAtom);

    // Delete audio tracks
    for (i = 0; i < _ptrackArray->size(); i++)
    {
        PV_MP4_FF_DELETE(NULL, TrackAtom, (*_ptrackArray)[i]);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, trackAtomVecType, Oscl_Vector, _ptrackArray);
    if (_pMovieExtendsAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, MovieExtendsAtom , _pMovieExtendsAtom);
    }
}

uint32 MovieAtom::getTimestampForCurrentSample(uint32 id)
{
    TrackAtom *track = getTrackForID(id);

    if (track != NULL)
    {
        return track->getTimestampForCurrentSample();
    }
    else
    {
        return 0;
    }
}

int32 MovieAtom::getOffsetByTime(uint32 id, uint32 ts, int32* sampleFileOffset)
{
    TrackAtom *track = getTrackForID(id);
    if (track == NULL)
    {
        return DEFAULT_ERROR;
    }
    return track->getOffsetByTime(ts, sampleFileOffset);
}

int32 MovieAtom::getMediaSample(uint32 id, uint32 sampleNumber, uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset)
{
    int32 nReturn = 0;

    TrackAtom *track = getTrackForID(id);
    if (track == NULL)
    {
        return READ_TRACK_ATOM_FAILED;
    }
    nReturn =  track->getMediaSample(sampleNumber, buf, size, index, SampleOffset);
    return (nReturn);
}

MP4_ERROR_CODE MovieAtom::getKeyMediaSampleNumAt(uint32 aTrackId,
        uint32 aKeySampleNum,
        GAU    *pgau)
{
    MP4_ERROR_CODE nReturn = READ_FAILED;

    TrackAtom *track = getTrackForID(aTrackId);
    if (track == NULL)
    {
        return nReturn;
    }
    nReturn =  track->getKeyMediaSampleNumAt(aKeySampleNum, pgau);
    return (nReturn);
}

int32 MovieAtom::getPrevKeyMediaSample(uint32 inputtimestamp,
                                       uint32 &aKeySampleNum,
                                       uint32 id,
                                       uint32 *n,
                                       GAU    *pgau)
{
    int32 nReturn = 0;

    TrackAtom *track = getTrackForID(id);

    if (track == NULL)
    {
        return READ_TRACK_ATOM_FAILED;
    }
    nReturn =  track->getPrevKeyMediaSample(inputtimestamp, aKeySampleNum, n, pgau);
    return (nReturn);
}

int32 MovieAtom::getNextKeyMediaSample(uint32 inputtimestamp,
                                       uint32 &aKeySampleNum,
                                       uint32 id,
                                       uint32 *n,
                                       GAU    *pgau)
{
    int32 nReturn = 0;

    TrackAtom *track = getTrackForID(id);

    if (track == NULL)
    {
        return READ_TRACK_ATOM_FAILED;
    }
    nReturn =  track->getNextKeyMediaSample(inputtimestamp, aKeySampleNum, n, pgau);
    return (nReturn);
}

int32 MovieAtom::getNextMediaSample(uint32 id, uint8 *buf, uint32 &size, uint32 &index, uint32 &SampleOffset)
{
    int32 nReturn = 0;

    TrackAtom *track = getTrackForID(id);
    if (track == NULL)
    {
        return READ_TRACK_ATOM_FAILED;
    }

    int32 buf_size =  size;
    nReturn =  track->getNextMediaSample(buf, buf_size, index, SampleOffset);
    size = buf_size;
    return (nReturn);
}

int32
MovieAtom::getNextBundledAccessUnits(uint32 id,
                                     uint32 *n,
                                     GAU    *pgau)
{
    int32 nReturn = -1;

    TrackAtom *track = getTrackForID(id);

    if (track != NULL)
    {
        nReturn =  track->getNextBundledAccessUnits(n, pgau);
    }

    return (nReturn);
}

int32
MovieAtom::peekNextBundledAccessUnits(uint32 id,
                                      uint32 *n,
                                      MediaMetaInfo *mInfo)
{
    int32 nReturn = -1;

    TrackAtom *track = getTrackForID(id);

    if (track != NULL)
    {
        nReturn = track->peekNextBundledAccessUnits(n, mInfo);
    }

    return (nReturn);
}


uint64 MovieAtom::getTrackDuration(uint32 id)
{
    TrackAtom *trackAtom = getTrackforID(id);

    if (trackAtom != NULL)
    {
        return trackAtom->getTrackDuration();
    }
    else
    {
        return 0;
    }
}

// From TrackReference
int32 MovieAtom::trackDependsOn(uint32 id)
{
    TrackAtom *trackAtom = getTrackforID(id);

    if (trackAtom != NULL)
    {
        return trackAtom->dependsOn();
    }
    else
    {
        return 0;
    }

}

OSCL_EXPORT_REF uint64 MovieAtom::getMovieFragmentDuration()
{
    if (_pMovieExtendsAtom != NULL)
    {
        return _pMovieExtendsAtom->getFragmentDuration();
    }
    else
        return 0;
}

uint64 MovieAtom::getTrackMediaDuration(uint32 id)
{
    TrackAtom *trackAtom = getTrackforID(id);

    if (trackAtom != NULL)
    {
        return trackAtom->getMediaDuration();
    }
    else
    {
        return 0;
    }
}

int32 MovieAtom::getTrackMediaTimescale(uint32 id)
{
    TrackAtom *trackAtom = getTrackforID(id);

    if (trackAtom != NULL)
    {
        return trackAtom->getMediaTimescale();
    }
    else
    {
        //RETURN SOME UNDEFINED VALUE
        return (0xFFFFFFFF);
    }
}

// From Handler
int32  MovieAtom::getTrackStreamType(uint32 id)
{
    TrackAtom *trackAtom = getTrackforID(id);

    if (trackAtom != NULL)
    {
        return trackAtom->getTrackStreamType();
    }
    else
    {
        //RETURN SOME UNDEFINED VALUE
        return (0xFFFFFFFF);
    }
}

// From SampleDescription
OSCL_EXPORT_REF int32 MovieAtom::getTrackNumSampleEntries(uint32 id)
{
    TrackAtom *trackAtom = getTrackforID(id);

    if (trackAtom != NULL)
    {
        return trackAtom->getNumSampleEntries();
    }
    else
    {
        return 0;
    }
}


// From DecoderConfigDescriptor
DecoderSpecificInfo *MovieAtom::getTrackDecoderSpecificInfo(uint32 id)
{
    TrackAtom *trackAtom = getTrackforID(id);

    if (trackAtom != NULL)
    {
        return trackAtom->getDecoderSpecificInfo();
    }
    else
    {
        return NULL;
    }
}

// From DecoderConfigDescriptor
DecoderSpecificInfo *
MovieAtom::getTrackDecoderSpecificInfoAtSDI(uint32 trackID, uint32 index)
{
    TrackAtom *trackAtom = getTrackforID(trackID);

    if (trackAtom != NULL)
    {
        return trackAtom->getDecoderSpecificInfoForSDI(index);
    }
    else
    {
        return NULL;
    }
}

void MovieAtom::getTrackMIMEType(uint32 id, OSCL_String& aMimeType)
{
    TrackAtom *trackAtom = getTrackforID(id);

    if (trackAtom != NULL)
    {
        trackAtom->getMIMEType(aMimeType);
    }
}


int32 MovieAtom::getTrackMaxBufferSizeDB(uint32 id)
{
    TrackAtom *trackAtom = getTrackforID(id);

    if (trackAtom != NULL)
    {
        return trackAtom->getMaxBufferSizeDB();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF int32 MovieAtom::getTrackHeight(uint32 id)
{
    TrackAtom *trackAtom = getTrackforID(id);

    if (trackAtom != NULL)
    {

        return trackAtom->getHeight();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF int32 MovieAtom::getTrackWidth(uint32 id)
{
    TrackAtom *trackAtom = getTrackforID(id);

    if (trackAtom != NULL)
    {
        return trackAtom->getWidth();
    }
    else
    {
        return 0;
    }
}

int32  MovieAtom::getTrackAverageBitrate(uint32 id)
{
    TrackAtom *trackAtom = getTrackforID(id);

    if (trackAtom != NULL)
    {
        return trackAtom->getAverageBitrate();
    }
    else
    {
        return 0;
    }
}


TrackAtom *
MovieAtom::getTrackforID(uint32 id)
{
    TrackAtom *trackAtom = NULL;
    uint32 i = 0;

    while (i < _ptrackArray->size())
    {
        trackAtom = (*_ptrackArray)[i];
        i++;
        if (trackAtom != NULL)
        {
            if (trackAtom->getTrackID() == id)
            {
                return trackAtom;
            }
        }
    }

    return NULL;
}

void MovieAtom::resetPlayback()
{
    uint32 i;
    TrackAtom *trackAtom;
    for (i = 0; i < _ptrackArray->size(); i++)
    {
        trackAtom = (*_ptrackArray)[i];

        if (trackAtom != NULL)
        {
            //only reset video and audio track
            if ((trackAtom->getMediaType() == MEDIA_TYPE_VISUAL) ||
                    (trackAtom->getMediaType() == MEDIA_TYPE_AUDIO)  ||
                    (trackAtom->getMediaType() == MEDIA_TYPE_TEXT))
            {
                trackAtom->resetPlayBack();
            }
        }
    }
}

void MovieAtom::resetTrackToEOT()
{
    uint32 i;
    TrackAtom *trackAtom;
    for (i = 0; i < _ptrackArray->size(); i++)
    {
        trackAtom = (*_ptrackArray)[i];

        if (trackAtom != NULL)
        {
            //only reset video and audio track
            if ((trackAtom->getMediaType() == MEDIA_TYPE_VISUAL) ||
                    (trackAtom->getMediaType() == MEDIA_TYPE_AUDIO)  ||
                    (trackAtom->getMediaType() == MEDIA_TYPE_TEXT))
            {
                trackAtom->resetTrackToEOT();
            }
        }
    }
}

OSCL_EXPORT_REF TrackAtom *
MovieAtom::getTrackForID(uint32 id)
{
    int32 i, trackArrayLength;
    uint32 trackID = 0;
    TrackAtom *trackAtom = NULL;
    trackArrayLength = _ptrackArray->size();

    for (i = 0; i < trackArrayLength; i++)
    {
        trackAtom = (TrackAtom*)(*_ptrackArray)[i];

        if (trackAtom != NULL)
        {
            trackID = trackAtom->getTrackID();
            if (trackID == id)
            {
                return trackAtom;
            }
        }
    }
    return NULL;

}


// Add a new Track (audio/video) only to the trackArray
void MovieAtom::addTrackAtom(TrackAtom *a)
{
    switch (a->getMediaType())
    {
        case MEDIA_TYPE_AUDIO:
            (*_ptrackArray).push_back(a);
            a->setParent(this);
            _fileType |= FILE_TYPE_AUDIO;
            break;
        case MEDIA_TYPE_VISUAL:
        {
            (*_ptrackArray).push_back(a);
            a->setParent(this);
            _fileType |= FILE_TYPE_VIDEO;
            _scalability += 1; // increment scalability with each video track read in
            // Preferred way would be to read in the VOL headers (DecoderSpecificInfo) and parse
            // to see the scalability and layer tags within the headers
            break;
        }

        case MEDIA_TYPE_TEXT:
        {
            (*_ptrackArray).push_back(a);
            a->setParent(this);
            _fileType |= FILE_TYPE_TEXT;
            break;
        }

        default:

            PV_MP4_FF_DELETE(NULL, TrackAtom, a);
            break;
    }

}

// Get timescale from movieheader atom for ENTIRE presentation
uint32
MovieAtom::getTimeScale() const
{
    if (_pmovieHeaderAtom != NULL)
    {
        return _pmovieHeaderAtom->getTimeScale();
    }
    else
    {
        //RETURN SOME UNDEFINED VALUE
        return 0xFFFFFFFF;
    }
}


uint32 MovieAtom::resetPlayback(uint32 time, uint16 numTracks, uint32 *trackList, bool bResetToIFrame)
{
    // Find base layer video track
    // Reset time on it, get ts
    // reset audio to that ts
    // reset enhance layer to ts
    //
    uint32 i, modifiedTimeStamp;
    uint32 timestamp, returnedTS;
    uint32 convertedTS = 0;
    TrackAtom *trackAtom;
    TrackAtom *IndependentTrackAtom;

    modifiedTimeStamp = time;

    if (!bResetToIFrame)
    {//if reset to I frame is not enforced, the FF will just move to rp time no matter
        //it is I or P frame.
        for (i = 0; i < numTracks; i++)
        {
            trackAtom = getTrackForID(*(trackList + i));

            if (trackAtom != NULL)
            {
                MediaClockConverter mcc1(1000);
                mcc1.update_clock(modifiedTimeStamp);
                convertedTS = mcc1.get_converted_ts(getTrackMediaTimescale(*(trackList + i)));

                returnedTS = trackAtom->resetPlayBack(convertedTS, true);
                // convert returnedTS (which is in media time scale) to the ms
                MediaClockConverter mcc(getTrackMediaTimescale(*(trackList + i)));
                mcc.update_clock(returnedTS);
                timestamp = mcc.get_converted_ts(1000);

                if (timestamp <= modifiedTimeStamp)
                {
                    modifiedTimeStamp = timestamp;
                }

            }
        }
        //this return value is meanlingless
        return modifiedTimeStamp;
    }

    for (i = 0; i < numTracks; i++)
    {
        trackAtom = getTrackForID(*(trackList + i));

        if (trackAtom != NULL)
        {

            //only reset video and audio track
            if (trackAtom->getMediaType() == MEDIA_TYPE_VISUAL)
            {
                _oVideoTrackPresent = true;

                if (trackAtom->dependsOn() != 0)
                {
                    IndependentTrackAtom = getTrackForID(trackAtom->dependsOn());

                    if (IndependentTrackAtom != NULL)
                    {
                        // convert modifiedTimeStamp (which is in ms) to the appropriate
                        // media time scale
                        MediaClockConverter mcc1(1000);
                        mcc1.update_clock(modifiedTimeStamp);
                        convertedTS = mcc1.get_converted_ts(getTrackMediaTimescale(*(trackList + i)));

                        returnedTS = IndependentTrackAtom->resetPlayBack(convertedTS);

                        // convert returnedTS (which is in media time scale) to the ms
                        MediaClockConverter mcc(getTrackMediaTimescale(*(trackList + i)));
                        mcc.update_clock(returnedTS);
                        timestamp = mcc.get_converted_ts(1000);

                        if (timestamp <= modifiedTimeStamp)
                        {
                            modifiedTimeStamp = timestamp;
                        }

                    }

                    bool oDependsOn = true;

                    trackAtom->resetPlayBack(convertedTS, oDependsOn);
                }
                else
                {
                    modifiedTimeStamp = time;

                    // convert modifiedTimeStamp (which is in ms) to the appropriate
                    // media time scale
                    MediaClockConverter mcc1(1000);
                    mcc1.update_clock(modifiedTimeStamp);
                    convertedTS = mcc1.get_converted_ts(getTrackMediaTimescale(*(trackList + i)));

                    returnedTS = trackAtom->resetPlayBack(convertedTS);

                    // convert returnedTS (which is in media time scale) to the ms
                    MediaClockConverter mcc(getTrackMediaTimescale(*(trackList + i)));
                    mcc.update_clock(returnedTS);
                    timestamp = mcc.get_converted_ts(1000);

                    if (timestamp <= modifiedTimeStamp)
                    {
                        modifiedTimeStamp = timestamp;
                    }

                }
            }
        }
        else
        {
            modifiedTimeStamp = 0;
        }
    }

    for (i = 0; i < numTracks; i++)
    {
        trackAtom = getTrackForID(*(trackList + i));

        if (trackAtom != NULL)
        {

            //only reset video and audio track
            if ((trackAtom ->getMediaType() == MEDIA_TYPE_AUDIO) ||
                    (trackAtom ->getMediaType() == MEDIA_TYPE_TEXT))
            {
                // convert modifiedTimeStamp (which is in ms) to the appropriate
                // media time scale
                MediaClockConverter mcc1(1000);
                mcc1.update_clock(modifiedTimeStamp);
                convertedTS = mcc1.get_converted_ts(getTrackMediaTimescale(*(trackList + i)));

                if (_oVideoTrackPresent)
                {
                    returnedTS = trackAtom->resetPlayBack(convertedTS, true);
                }
                else
                {
                    returnedTS = trackAtom->resetPlayBack(convertedTS);
                }

                // convert returnedTS (which is in media time scale) to the ms
                MediaClockConverter mcc(getTrackMediaTimescale(*(trackList + i)));
                mcc.update_clock(returnedTS);
                timestamp = mcc.get_converted_ts(1000);

                if (timestamp <= modifiedTimeStamp)
                {
                    modifiedTimeStamp = timestamp;
                }
            }
        }
        else
        {
            modifiedTimeStamp = 0;
        }
    }


    return modifiedTimeStamp;

}

int32 MovieAtom::queryRepositionTime(uint32 time,
                                     uint16 numTracks,
                                     uint32 *trackList,
                                     bool bResetToIFrame,
                                     bool bBeforeRequestedTime)
{
    // Find base layer video track
    // Reset time on it, get ts
    // reset audio to that ts
    // reset enhance layer to ts
    //
    uint32 i, modifiedTimeStamp;
    uint32 timestamp, returnedTS;
    uint32 convertedTS = 0, minTS = 0;
    TrackAtom *trackAtom;
    TrackAtom *IndependentTrackAtom;

    bool  oVideoTrackPresent = false;
    modifiedTimeStamp = time;
    minTS = 0x7FFFFFFF;

    if (!bResetToIFrame)
    {//if reset to I frame is not enforced, the FF will just move to rp time no matter
        //it is I or P frame.
        for (i = 0; i < numTracks; i++)
        {
            trackAtom = getTrackForID(*(trackList + i));

            if (trackAtom != NULL)
            {
                // convert modifiedTimeStamp (which is in ms) to the appropriate
                // media time scale
                MediaClockConverter mcc1(1000);
                mcc1.update_clock(modifiedTimeStamp);
                convertedTS = mcc1.get_converted_ts(trackAtom->getMediaTimescale());

                returnedTS = trackAtom->queryRepositionTime(convertedTS, true, bBeforeRequestedTime);

                // convert returnedTS (which is in media time scale) to the ms
                MediaClockConverter mcc(trackAtom->getMediaTimescale());
                mcc.update_clock(returnedTS);
                timestamp = mcc.get_converted_ts(1000);
                if (timestamp < minTS)
                    minTS = timestamp;
            }
        }
        return minTS;

    }
    for (i = 0; i < numTracks; i++)
    {
        trackAtom = getTrackForID(*(trackList + i));

        if (trackAtom != NULL)
        {
            //only reset video and audio track
            if (trackAtom->getMediaType() == MEDIA_TYPE_VISUAL)
            {
                oVideoTrackPresent = true;

                if (trackAtom->dependsOn() != 0)
                {
                    // convert modifiedTimeStamp (which is in ms) to the appropriate
                    // media time scale
                    MediaClockConverter mcc1(1000);
                    mcc1.update_clock(modifiedTimeStamp);
                    convertedTS = mcc1.get_converted_ts(trackAtom->getMediaTimescale());

                    IndependentTrackAtom = getTrackForID(trackAtom->dependsOn());

                    if (IndependentTrackAtom != NULL)
                    {
                        returnedTS =
                            IndependentTrackAtom->queryRepositionTime(convertedTS,
                                    false,
                                    bBeforeRequestedTime);

                        // convert returnedTS (which is in media time scale) to the ms
                        MediaClockConverter mcc(trackAtom->getMediaTimescale());
                        mcc.update_clock(returnedTS);
                        timestamp = mcc.get_converted_ts(1000);

                        if (timestamp <= modifiedTimeStamp)
                        {
                            modifiedTimeStamp = timestamp;
                        }
                    }

                    trackAtom->queryRepositionTime(convertedTS, false, bBeforeRequestedTime);
                }
                else
                {
                    modifiedTimeStamp = time;

                    // convert modifiedTimeStamp (which is in ms) to the appropriate
                    // media time scale
                    MediaClockConverter mcc1(1000);
                    mcc1.update_clock(modifiedTimeStamp);
                    convertedTS = mcc1.get_converted_ts(trackAtom->getMediaTimescale());

                    returnedTS = trackAtom->queryRepositionTime(convertedTS, false, bBeforeRequestedTime);

                    // convert returnedTS (which is in media time scale) to the ms
                    MediaClockConverter mcc(trackAtom->getMediaTimescale());
                    mcc.update_clock(returnedTS);
                    timestamp = mcc.get_converted_ts(1000);

                    modifiedTimeStamp = timestamp;
                }
            }
        }
        else
        {
            modifiedTimeStamp = 0;
        }
    }

    for (i = 0; i < numTracks; i++)
    {
        trackAtom = getTrackForID(*(trackList + i));

        if (trackAtom != NULL)
        {
            //only reset video and audio track
            if ((trackAtom ->getMediaType() == MEDIA_TYPE_AUDIO) ||
                    (trackAtom ->getMediaType() == MEDIA_TYPE_TEXT))
            {
                // convert modifiedTimeStamp (which is in ms) to the appropriate
                // media time scale
                MediaClockConverter mcc1(1000);
                mcc1.update_clock(modifiedTimeStamp);
                convertedTS = mcc1.get_converted_ts(trackAtom->getMediaTimescale());

                returnedTS = trackAtom->queryRepositionTime(convertedTS, false, bBeforeRequestedTime);

                // convert returnedTS (which is in media time scale) to the ms
                MediaClockConverter mcc(trackAtom->getMediaTimescale());
                mcc.update_clock(returnedTS);
                timestamp = mcc.get_converted_ts(1000);

                if (!oVideoTrackPresent)
                {
                    if (trackAtom ->getMediaType() == MEDIA_TYPE_AUDIO)
                    {
                        if (timestamp <= modifiedTimeStamp)
                        {
                            modifiedTimeStamp = timestamp;
                        }
                    }
                }
            }
        }
        else
        {
            modifiedTimeStamp = 0;
        }
    }
    return modifiedTimeStamp;
}

int32	MovieAtom::querySyncFrameBeforeTime(uint32 time, uint16 numTracks, uint32 *trackList)
{
    TrackAtom *trackAtom;
    for (uint16 i = 0; i < numTracks; i++)
    {
        trackAtom = getTrackForID(*(trackList + i));
        if (trackAtom != NULL)
        {
            if (trackAtom->getMediaType() == MEDIA_TYPE_VISUAL)
            {
                if (!trackAtom->dependsOn())
                {	//base layer
                    return trackAtom->IsResetNeeded(time);
                }
            }
        }
    }
    return EVERYTHING_FINE;	//resetplayback is needed
}

uint32 MovieAtom::getTrackWholeIDList(uint32 *ids)
{
    int32 i = 0, totalTrackNumber;
    TrackAtom *trackAtom;
    totalTrackNumber = getNumTracks();

    if (ids == NULL)
    {
        return 0;
    }

    for (i = 0; i < totalTrackNumber; i++)
    {
        trackAtom = (TrackAtom *)(*_ptrackArray)[i];

        if (trackAtom != NULL)
        {
            (*ids) = trackAtom->getTrackID();
            ids++;
        }
    }
    return i;
}


int32 MovieAtom::updateFileSize(uint32	filesize)
{
    if (NULL == _ptrackArray)
    {
        return DEFAULT_ERROR;
    }
    TrackAtom *trackAtom;
    int32	returnVal = EVERYTHING_FINE;//success
    for (uint32 i = 0; i < _ptrackArray->size(); i++)
    {
        trackAtom = (TrackAtom *)(*_ptrackArray)[i];
        if (trackAtom != NULL)
        {
            if (EVERYTHING_FINE != trackAtom->updateFileSize(filesize))
            {
                returnVal = DEFAULT_ERROR;
            }
        }

    }
    return returnVal;	//success
}

OSCL_EXPORT_REF uint32 MovieAtom::getTrackIDList(uint32 *ids, int size)
{
    int32 i = 0, totalTrackNumber;
    TrackAtom *trackAtom;
    totalTrackNumber = getNumTracks();

    if (ids == NULL)
    {
        return 0;
    }

    while ((i < size) && (i < totalTrackNumber))
    {
        trackAtom = (TrackAtom *)(*_ptrackArray)[i];
        if (trackAtom != NULL)
        {
            (*ids) = trackAtom->getTrackID();
            ids++;
        }
        i++;
    }
    return i;
}

OSCL_EXPORT_REF uint32 MovieAtom::getSampleCountInTrack(uint32 id)
{
    TrackAtom *track = getTrackForID(id);

    if (track != NULL)
    {
        return track->getSampleCount();
    }
    else
    {
        // BY DEFAULT NO RANDOM ACCESS
        return 0;
    }
}

bool MovieAtom::checkMMP4()
{
    uint32 i;
    TrackAtom *trackAtom;

    int32 numAudioTracks = 0;
    int32 numVideoTracks = 0;
    int32 numTextTracks  = 0;

    for (i = 0; i < _ptrackArray->size(); i++)
    {
        trackAtom = (*_ptrackArray)[i];

        if (trackAtom != NULL)
        {
            //only reset video and audio track
            if (trackAtom->getMediaType() == MEDIA_TYPE_VISUAL)
            {
                numVideoTracks += 1;
            }

            if (trackAtom->getMediaType() == MEDIA_TYPE_AUDIO)
            {
                numAudioTracks += 1;
            }

            if (trackAtom->getMediaType() == MEDIA_TYPE_TEXT)
            {
                numTextTracks += 1;
            }
        }
    }

    if ((numAudioTracks > 1) || (numVideoTracks > 1) || (numTextTracks > 1))
    {
        return false;
    }
    else
    {
        return true;
    }
}
uint16 MovieAtom::getAssetInfoTitleLangCode(int32 index)
{
    AssetInfoTitleAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoTitleAtomAt(index);
    }
    else
    {
        return (0xFFFF);
    }

    if (pAtom != NULL)
    {
        return pAtom->getTitleLangCode();
    }
    else
    {
        return (0xFFFF);
    }
}

OSCL_wString& MovieAtom::getAssetInfoTitleNotice(MP4FFParserOriginalCharEnc &charType, int32 index)
{
    AssetInfoTitleAtom *pAtom = NULL;
    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoTitleAtomAt(index);
    }
    else
    {
        return _emptyString;
    }

    if (pAtom != NULL)
    {
        return pAtom->getTitleNotice(charType);
    }
    else
    {
        return _emptyString;
    }
}
uint16 MovieAtom::getAssetInfoDescLangCode(int32 index)
{
    AssetInfoDescAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoDescAtomAt(index);
    }
    else
    {
        return (0xFFFF);
    }

    if (pAtom != NULL)
    {
        return pAtom->getDescLangCode();
    }
    else
    {
        return (0xFFFF);
    }
}

OSCL_wString& MovieAtom::getAssetInfoDescNotice(MP4FFParserOriginalCharEnc &charType, int32 index)
{
    AssetInfoDescAtom *pAtom = NULL;
    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoDescAtomAt(index);
    }
    else
    {
        return _emptyString;
    }

    if (pAtom != NULL)
    {
        return pAtom->getDescNotice(charType);
    }
    else
    {
        return _emptyString;
    }
}


OSCL_wString& MovieAtom::getCopyRightString(MP4FFParserOriginalCharEnc &charType, int32 index)
{
    charType = ORIGINAL_CHAR_TYPE_UNKNOWN;

    CopyRightAtom *patom = NULL;
    if (_pUserDataAtom != NULL)
    {
        patom = _pUserDataAtom->getCopyRightAtomAt(index);
    }
    else
    {
        return _emptyString; // return empty string
    }

    if (patom != NULL)
    {
        return patom->getCopyRightString();
    }
    else
    {
        return _emptyString; // return empty string
    }
}

uint16 MovieAtom::getCopyRightLanguageCode(int32 index)
{
    CopyRightAtom *patom = NULL;

    if (_pUserDataAtom != NULL)
    {
        patom = _pUserDataAtom->getCopyRightAtomAt(index);
    }
    else
    {
        return 0xFFFF;
    }

    if (patom != NULL)
    {
        return patom->getLanguageCode();
    }
    else
    {
        return 0xFFFF;
    }
}

uint16 MovieAtom::getAssetInfoPerformerLangCode(int32 index)
{
    AssetInfoPerformerAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoPerformerAtomAt(index);
    }
    else
    {
        return (0xFFFF);
    }

    if (pAtom != NULL)
    {
        return pAtom->getPerfLangCode();
    }
    else
    {
        return (0xFFFF);
    }
}

OSCL_wString& MovieAtom::getAssetInfoPerformerNotice(MP4FFParserOriginalCharEnc &charType, int32 index)
{
    AssetInfoPerformerAtom *pAtom = NULL;
    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoPerformerAtomAt(index);
    }
    else
    {
        return _emptyString;
    }

    if (pAtom != NULL)
    {
        return pAtom->getPerfNotice(charType);
    }
    else
    {
        return _emptyString;
    }
}

uint16 MovieAtom::getAssetInfoAuthorLangCode(int32 index)
{
    AssetInfoAuthorAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoAuthorAtomAt(index);
    }
    else
    {
        return (0xFFFF);
    }

    if (pAtom != NULL)
    {
        return pAtom->getAuthorLangCode();
    }
    else
    {
        return (0xFFFF);
    }
}

OSCL_wString& MovieAtom::getAssetInfoAuthorNotice(MP4FFParserOriginalCharEnc &charType, int32 index)
{
    AssetInfoAuthorAtom *pAtom = NULL;
    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoAuthorAtomAt(index);
    }
    else
    {
        return _emptyString;
    }

    if (pAtom != NULL)
    {
        return pAtom->getAuthorNotice(charType);
    }
    else
    {
        return _emptyString;
    }
}

uint16 MovieAtom::getAssetInfoGenreLangCode(int32 index)
{
    AssetInfoGenreAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoGenreAtomAt(index);
    }
    else
    {
        return (0xFFFF);
    }

    if (pAtom != NULL)
    {
        return pAtom->getGenreLangCode();
    }
    else
    {
        return (0xFFFF);
    }
}

OSCL_wString& MovieAtom::getAssetInfoGenreNotice(MP4FFParserOriginalCharEnc &charType, int32 index)
{
    AssetInfoGenreAtom *pAtom = NULL;
    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoGenreAtomAt(index);
    }
    else
    {
        return _emptyString;
    }

    if (pAtom != NULL)
    {
        return pAtom->getGenreNotice(charType);
    }
    else
    {
        return _emptyString;
    }
}

uint32 MovieAtom::getAssetInfoRatingCriteria(int32 index)
{
    AssetInfoRatingAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoRatingAtomAt(index);
    }
    else
    {
        return (0);
    }

    if (pAtom != NULL)
    {
        return pAtom->getRatingCriteria();
    }
    else
    {
        return 0;
    }
}

uint32 MovieAtom::getAssetInfoRatingEntity(int32 index)
{
    AssetInfoRatingAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoRatingAtomAt(index);
    }
    else
    {
        return (0);
    }

    if (pAtom != NULL)
    {
        return pAtom->getRatingEntity();
    }
    else
    {
        return (0);
    }
}

uint16 MovieAtom::getAssetInfoRatingLangCode(int32 index)
{
    AssetInfoRatingAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoRatingAtomAt(index);
    }
    else
    {
        return (0xFFFF);
    }

    if (pAtom != NULL)
    {
        return pAtom->getRatingLangCode();
    }
    else
    {
        return (0xFFFF);
    }
}

OSCL_wString& MovieAtom::getAssetInfoRatingNotice(MP4FFParserOriginalCharEnc &charType, int32 index)
{
    AssetInfoRatingAtom *pAtom = NULL;
    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoRatingAtomAt(index);
    }
    else
    {
        return _emptyString;
    }

    if (pAtom != NULL)
    {
        return pAtom->getRatingNotice(charType);
    }
    else
    {
        return _emptyString;
    }
}

uint32 MovieAtom::getAssetInfoClassificationEntity(int32 index)
{
    AssetInfoClassificationAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoClassificationAtomAt(index);
    }
    else
    {
        return (0);
    }

    if (pAtom != NULL)
    {
        return pAtom->getClassificationEntity();
    }
    else
    {
        return 0;
    }
}

uint16 MovieAtom::getAssetInfoClassificationTable(int32 index)
{
    AssetInfoClassificationAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoClassificationAtomAt(index);
    }
    else
    {
        return (0);
    }

    if (pAtom != NULL)
    {
        return pAtom->getClassificationTable();
    }
    else
    {
        return (0);
    }
}

uint16 MovieAtom::getAssetInfoClassificationLangCode(int32 index)
{
    AssetInfoClassificationAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoClassificationAtomAt(index);
    }
    else
    {
        return (0xFFFF);
    }

    if (pAtom != NULL)
    {
        return pAtom->getClassificationLangCode();
    }
    else
    {
        return (0xFFFF);
    }
}

OSCL_wString& MovieAtom::getAssetInfoClassificationNotice(MP4FFParserOriginalCharEnc &charType, int32 index)
{
    AssetInfoClassificationAtom *pAtom = NULL;
    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoClassificationAtomAt(index);
    }
    else
    {
        return _emptyString;
    }

    if (pAtom != NULL)
    {
        return pAtom->getClassificationNotice(charType);
    }
    else
    {
        return _emptyString;
    }
}

uint16 MovieAtom::getAssetInfoNumKeyWords(int32 index)
{
    AssetInfoKeyWordAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoKeyWordAtomAt(index);
    }
    else
    {
        return (0);
    }

    if (pAtom != NULL)
    {
        return (uint16)pAtom->getNumKeyWords();
    }
    else
    {
        return (0);
    }
}

uint16 MovieAtom::getAssetInfoKeyWordLangCode(int32 index)
{
    AssetInfoKeyWordAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoKeyWordAtomAt(index);
    }
    else
    {
        return (0xFFFF);
    }

    if (pAtom != NULL)
    {
        return pAtom->getKeyWordLangCode();
    }
    else
    {
        return (0xFFFF);
    }
}

OSCL_wString& MovieAtom::getAssetInfoKeyWord(int32 atomIndex, int32 keyWordIndex)
{
    AssetInfoKeyWordAtom *pAtom = NULL;
    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoKeyWordAtomAt(atomIndex);
    }
    else
    {
        return _emptyString;
    }

    if (pAtom != NULL)
    {
        return pAtom->getKeyWordAt(keyWordIndex);
    }
    else
    {
        return _emptyString;
    }
}

PvmfAssetInfo3GPPLocationStruct* MovieAtom::getAssetInfoLocationStruct(int32 index) const
{
    AssetInfoLocationAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoLocationAtomAt(index);
    }
    else
    {
        return NULL;
    }

    if (pAtom != NULL)
    {
        return pAtom->getAssetInfoLocationStruct();
    }
    else
    {
        return NULL;
    }

}


uint16 MovieAtom::getAssetInfoAlbumLangCode(int32 index)
{
    AssetInfoAlbumAtom *pAtom = NULL;

    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoAlbumAtomAt(index);
    }
    else
    {
        return (0xFFFF);
    }

    if (pAtom != NULL)
    {
        return pAtom->getAlbumLangCode();
    }
    else
    {
        return (0xFFFF);
    }
}

OSCL_wString& MovieAtom::getAssetInfoAlbumNotice(MP4FFParserOriginalCharEnc &charType, int32 index)
{
    AssetInfoAlbumAtom *pAtom = NULL;
    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoAlbumAtomAt(index);
    }
    else
    {
        return _emptyString;
    }

    if (pAtom != NULL)
    {
        return pAtom->getAlbumNotice(charType);
    }
    else
    {
        return _emptyString;
    }
}

uint8 MovieAtom::getAssetInfoAlbumTrackNumber(int32 index)
{
    AssetInfoAlbumAtom *pAtom = NULL;
    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoAlbumAtomAt(index);
    }
    else
    {
        return 0;
    }

    if (pAtom != NULL)
    {
        return pAtom->getTrackNumber();
    }
    else
    {
        return 0;
    }
}

uint16 MovieAtom::getAssetInfoRecordingYear(int32 index)
{
    AssetInfoRecordingYearAtom *pAtom = NULL;
    if (_pUserDataAtom != NULL)
    {
        pAtom = _pUserDataAtom->getAssetInfoRecordingYearAtomAt(index);
    }
    else
    {
        return 0;
    }

    if (pAtom != NULL)
    {
        return pAtom->getRecordingYear();
    }
    else
    {
        return 0;
    }
}

int16 MovieAtom::getLayer(uint32 id)
{
    TrackAtom *track = getTrackForID(id);

    if (track != NULL)
    {
        return track->getLayer();
    }
    else
    {
        return 0;
    }
}

uint16 MovieAtom::getAlternateGroup(uint32 id)
{
    TrackAtom *track = getTrackForID(id);

    if (track != NULL)
    {
        return track->getAlternateGroup();
    }
    else
    {
        return 0;
    }
}

int32 MovieAtom::getTextTrackWidth(uint32 id)
{
    TrackAtom *track = getTrackForID(id);

    if (track != NULL)
    {
        return track->getTextTrackWidth();
    }
    else
    {
        return (-1);
    }
}

int32 MovieAtom::getTextTrackHeight(uint32 id)
{
    TrackAtom *track = getTrackForID(id);

    if (track != NULL)
    {
        return track->getTextTrackHeight();
    }
    else
    {
        return (-1);
    }
}

int32 MovieAtom::getTextTrackXOffset(uint32 id)
{
    TrackAtom *track = getTrackForID(id);

    if (track != NULL)
    {
        return track->getTextTrackXOffset();
    }
    else
    {
        return (-1);
    }
}


int32 MovieAtom::getTextTrackYOffset(uint32 id)
{
    TrackAtom *track = getTrackForID(id);

    if (track != NULL)
    {
        return track->getTextTrackYOffset();
    }
    else
    {
        return (-1);
    }
}

SampleEntry*
MovieAtom::getTextSampleEntryAt(uint32 id, uint32 index)
{
    TrackAtom *track = getTrackForID(id);

    if (track != NULL)
    {
        return track->getTextSampleEntryAt(index);
    }
    else
    {
        return NULL;
    }
}

int32 MovieAtom::getTrackTSStartOffset(uint32& aTSOffset, uint32 aTrackID)
{
    aTSOffset = 0;
    TrackAtom *track = getTrackForID(aTrackID);

    uint32 movieTimeScale =	_pmovieHeaderAtom->getTimeScale();
    if (track != NULL)
    {
        return track->getTrackTSOffset(aTSOffset, movieTimeScale);
    }
    else
    {
        return READ_FAILED;
    }
}



int32 MovieAtom::getNumAMRFramesPerSample(uint32 trackID)
{
    TrackAtom *trackAtom;
    trackAtom = getTrackForID(trackID);

    if (trackAtom != NULL)
    {
        return (trackAtom->getNumAMRFramesPerSample());
    }
    else
    {
        return 0;
    }
}


MP4_ERROR_CODE MovieAtom::getMaxTrackTimeStamp(uint32 trackID,
        uint32 fileSize,
        uint32& timeStamp)
{
    TrackAtom *trackAtom;
    trackAtom = getTrackForID(trackID);

    if (trackAtom != NULL)
    {
        return (trackAtom->getMaxTrackTimeStamp(fileSize,
                                                timeStamp));
    }
    else
    {
        return DEFAULT_ERROR;
    }
}

MP4_ERROR_CODE MovieAtom::getSampleNumberClosestToTimeStamp(uint32 trackID,
        uint32 &sampleNumber,
        uint32 timeStamp,
        uint32 sampleOffset)
{
    TrackAtom *trackAtom;
    trackAtom = getTrackForID(trackID);

    if (trackAtom != NULL)
    {
        return
            (trackAtom->getSampleNumberClosestToTimeStamp(sampleNumber,
                    timeStamp,
                    sampleOffset));
    }
    else
    {
        return (READ_FAILED);
    }
}


AVCSampleEntry* MovieAtom::getAVCSampleEntry(uint32 trackID, uint32 index)
{
    TrackAtom *trackAtom;
    trackAtom = getTrackForID(trackID);

    if (trackAtom != NULL)
    {
        return (trackAtom->getAVCSampleEntry(index));
    }
    return (NULL);
}


uint32 MovieAtom::getAVCNALLengthSize(uint32 trackID, uint32 index)
{
    TrackAtom *trackAtom;
    trackAtom = getTrackForID(trackID);

    if (trackAtom != NULL)
    {
        return (trackAtom->getAVCNALLengthSize(index));
    }
    return 0;
}



uint32 MovieAtom::getNumAVCSampleEntries(uint32 trackID)
{
    TrackAtom *trackAtom;
    trackAtom = getTrackForID(trackID);

    if (trackAtom != NULL)
    {
        return (trackAtom->getNumAVCSampleEntries());
    }
    return 0;
}


OSCL_EXPORT_REF bool MovieAtom::isMultipleSampleDescriptionAvailable(uint32 trackID)
{
    TrackAtom *trackAtom;
    trackAtom = getTrackForID(trackID);

    if (trackAtom != NULL)
    {
        return (trackAtom->isMultipleSampleDescriptionAvailable());
    }
    return 0;
}

int32 MovieAtom::getTimestampForRandomAccessPoints(uint32 id, uint32 *num, uint32 *tsBuf, uint32* numBuf, uint32* offsetBuf)
{
    TrackAtom *trackAtom;
    trackAtom = getTrackForID(id);
    if (trackAtom != NULL)
    {
        return trackAtom->getTimestampForRandomAccessPoints(num, tsBuf, numBuf, offsetBuf);
    }
    else
    {
        return 0;
    }
}

int32 MovieAtom::getTimestampForRandomAccessPointsBeforeAfter(uint32 id, uint32 ts, uint32 *tsBuf, uint32* numBuf,
        uint32& numsamplestoget,
        uint32 howManyKeySamples)
{
    TrackAtom *trackAtom;
    trackAtom = getTrackForID(id);
    if (trackAtom != NULL)
    {
        return trackAtom->getTimestampForRandomAccessPointsBeforeAfter(ts, tsBuf, numBuf, numsamplestoget, howManyKeySamples);
    }
    else
    {
        return 0;
    }

}

