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
/*                            MPEG-4 TrackAtom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This TrackAtom Class is the container for a single track in the MPEG-4
    presentation.
*/


#ifndef TRACKATOM_H_INCLUDED
#define TRACKATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef TRACKHEADERATOM_H_INCLUDED
#include "trackheaderatom.h"
#endif

#ifndef MEDIAATOM_H_INCLUDED
#include "mediaatom.h"
#endif

#ifndef TRACKREFERENCEATOM_H_INCLUDED
#include "trackreferenceatom.h"
#endif

#ifndef EDITATOM_H_INCLUDED
#include "editatom.h"
#endif

#ifndef DECODERSPECIFICINFO_H_INCLUDED
#include "decoderspecificinfo.h"
#endif


#ifndef USERDATAATOM_H_INCLUDED
#include "userdataatom.h"
#endif

#ifndef OSCL_MEDIA_DATA_H_INCLUDED
#include "oscl_media_data.h"
#endif

#ifndef PV_GAU_H_INCLUDED
#include "pv_gau.h"
#endif

#ifndef OMA2BOXES_H_INCLUDED
#include "oma2boxes.h"
#endif

class AVCSampleEntry;

class TrackAtom : public Atom
{

    public:
        TrackAtom(MP4_FF_FILE *fp,
                  OSCL_wString& filename,
                  uint32 size,
                  uint32 type,
                  bool oPVContent = false,
                  bool oPVContentDownloadable = false,
                  uint32 parsingMode = 0); // Stream-in Constructor

        virtual ~TrackAtom();

        /* Returns media samples
        buf:	A pointer to the data buffer into which to place the media sample.
        size:	The size of the data buffer
        index:	An output parameter which is the index of the sample entry to which the returned sample refers.  If zero, will return based on the index set by the previous call to getNextMediaSample().
        return:	The size in bytes of the data placed into the provided buffer.  If the buffer is not large enough, the return value is the negative of the size that is needed.
        */
        int32 getNextMediaSample(uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset)
        {
            if (_pmediaAtom == NULL)
            {
                return READ_MEDIA_ATOM_FAILED;
            }
            return _pmediaAtom->getNextSample(buf, size, index, SampleOffset);
        }

        MP4_ERROR_CODE getKeyMediaSampleNumAt(uint32 aKeySampleNum,
                                              GAU    *pgau)
        {
            if (_pmediaAtom == NULL)
            {
                return READ_MEDIA_ATOM_FAILED;
            }
            return _pmediaAtom->getKeyMediaSampleNumAt(aKeySampleNum, pgau);
        }

        uint32 getNumKeyFrames()
        {
            if (_pmediaAtom != NULL)
            {
                return (_pmediaAtom->getNumKeyFrames());
            }
            else
            {
                return 0;
            }
        }

        int32 getPrevKeyMediaSample(uint32 inputtimestamp,
                                    uint32  &aKeySampleNum,
                                    uint32 *n,
                                    GAU    *pgau)
        {
            if (_pmediaAtom == NULL)
            {
                return READ_MEDIA_ATOM_FAILED;
            }
            return _pmediaAtom->getPrevKeyMediaSample(inputtimestamp, aKeySampleNum, n, pgau);
        }

        int32 getNextKeyMediaSample(uint32 inputtimestamp,
                                    uint32 &aKeySampleNum,
                                    uint32 *n,
                                    GAU    *pgau)
        {
            if (_pmediaAtom == NULL)
            {
                return READ_MEDIA_ATOM_FAILED;
            }
            return _pmediaAtom->getNextKeyMediaSample(inputtimestamp, aKeySampleNum, n, pgau);
        }


        int32 getMediaSample(uint32 sampleNumber, uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset)
        {
            if (_pmediaAtom == NULL)
            {
                return READ_MEDIA_ATOM_FAILED;
            }
            return _pmediaAtom->getMediaSample(sampleNumber, buf, size, index, SampleOffset);
        }

        int32 updateFileSize(uint32	filesize)
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->updateFileSize(filesize);
            }
            return DEFAULT_ERROR;
        }

        int32 getOffsetByTime(uint32 ts, int32* sampleFileOffset)
        {
            if (_pmediaAtom == NULL)
            {
                return DEFAULT_ERROR;
            }
            return _pmediaAtom->getOffsetByTime(ts, sampleFileOffset);
        }

        uint32 getTrackID() const
        {
            if (_ptrackHeader != NULL)
            {
                return _ptrackHeader->getTrackID();
            }
            else
            {
                return 0;
            }
        }

        uint64 getTrackDuration() const
        {
            if (_ptrackHeader != NULL)
            {
                return _ptrackHeader->getDuration();
            }
            else
            {
                return 0;
            }
        }

        // Get the max size buffer needed to retrieve the media samples
        uint32 getMaxBufferSizeDB() const
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getMaxBufferSizeDB();
            }
            else
            {
                return 0;
            }
        }

        uint32 getMediaType() const
        {
            return _pMediaType;
        }

        void  resetPlayBack()
        {
            if (_pmediaAtom != NULL)
            {
                _pmediaAtom->resetPlayBack();
            }
        }

        void resetTrackToEOT()
        {
            if (_pmediaAtom != NULL)
            {
                _pmediaAtom->resetTrackToEOT();
            }
        }

        int32  resetPlayBack(int32 time, bool oDependsOn = false)
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->resetPlayBack(time, oDependsOn);
            }
            else
            {
                return 0;
            }
        }

        int32  queryRepositionTime(int32 time, bool oDependsOn = false, bool bBeforeRequestedTime = true)
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->queryRepositionTime(time, oDependsOn, bBeforeRequestedTime);
            }
            else
            {
                return 0;
            }
        }

        int32 IsResetNeeded(int32 time)
        {
            if (_pmediaAtom == NULL)
                return READ_MEDIA_ATOM_FAILED;
            return _pmediaAtom->IsResetNeeded((int32)((float)time*(float)_pmediaAtom->getMediaTimescale() / (float)1000));
        }

        uint32 getTimestampForSampleNumber(uint32 sampleNumber)
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getTimestampForSampleNumber(sampleNumber);
            }
            else
            {
                return 0;
            }
        }

        int32 getSampleSizeAt(int32 sampleNum)
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getSampleSizeAt(sampleNum);
            }
            else
            {
                return 0;
            }
        }


        // Returns the timestamp  from the last video sample
        // This is mainly to be used when seeking in the bitstream - you request a video frame at timestamp
        // X, but the actual frame you get is Y, this method returns the timestamp for Y so you know which
        // audio sample to request.
        int32 getTimestampForCurrentSample()
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getTimestampForCurrentSample();
            }
            else
            {
                return 0;
            }
        }

        // Returns the sample number of the last samplle returned
        // Used when requesting a hint sample for a specific randomly accessed sample
        int32 getSampleNumberForCurrentSample()
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getSampleNumberForCurrentSample();
            }
            else
            {
                return 0;
            }
        }

        // Getting and setting the Mpeg4 VOL header
        DecoderSpecificInfo *getDecoderSpecificInfo() const
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getDecoderSpecificInfo();
            }
            else
            {
                return NULL;
            }
        }

        DecoderSpecificInfo *getDecoderSpecificInfoForSDI(uint32 index) const
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getDecoderSpecificInfoForSDI(index);
            }
            else
            {
                return NULL;
            }
        }

        // TS offset value for the start of the media track.  The STTS
        // Atom only holds TS deltas.  For a track that does not begin at 0s, we need to hold an
        // offset timestamp value.
        int32 NEWsetTrackTSOffset(uint32 ts);

        int32 getTrackTSOffset(uint32& aTSOffset, uint32 aMovieTimeScale);

        void setTrackTSOffset(uint32 ts)
        {
            _trackStartOffset = ts;

            if (_pmediaAtom != NULL)
            {
                _pmediaAtom->setTrackTSOffset(ts);
            }
        }


        //From TrackHeader
        uint64 getTrackDuration()
        {
            if (_ptrackHeader != NULL)
            {
                return _ptrackHeader->getDuration();
            }
            else
            {
                return 0;
            }
        } // in terms of the movie timescale

        // From TrackReference
        uint32 dependsOn()
        {
            if (_ptrackReference != NULL)
            {
                return _ptrackReference->getTrackReference();
            }
            else
            {
                return 0;
            }
        }

        // From MediaAtom -> MediaHeader
        uint64 getMediaDuration()
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getMediaDuration();
            }
            else
            {
                return 0;
            }
        } // in terms of the media timescale

        uint32 getMediaTimescale()
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getMediaTimescale();
            }
            else
            {
                //RETURN SOME UNDEFINED VALUE
                return (0xFFFFFFFF);
            }
        }

        uint16 getLanguageCode()
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getLanguageCode();
            }
            else
            {
                //RETURN SOME UNDEFINED VALUE
                return (0xFFFF);
            }

        }


        // From mediaAtom -> Handler
        uint32 getTrackStreamType()
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getTrackStreamType();
            }
            else
            {
                return 0xFFFFFFFF;
            }
        }

        // From mediaAtom -> SampleDescription
        uint32 getNumSampleEntries()
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getNumSampleEntries();
            }
            else
            {
                return 0xFFFFFFFF;
            }
        }

        void getMIMEType(OSCL_String& aMimeType)
        {
            if (_pmediaAtom != NULL)
            {
                _pmediaAtom->getMIMEType(aMimeType);
            }
        } // Based on OTI value

        OSCL_IMPORT_REF uint8  getObjectTypeIndication();

        int getMaxBufferSizeDB()
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getMaxBufferSizeDB();
            }
            else
            {
                return 0;
            }
        }

        int getAverageBitrate()
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getAverageBitrate();
            }
            else
            {
                return 0;
            }
        }
        int32 getHeight()
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getHeight();
            }
            else
            {
                return 0;
            }

        }

        int32 getWidth()
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getWidth();
            }
            else
            {
                return 0;
            }
        }

        //PASP box
        uint32 getHspacing()
        {

            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getHspacing();
            }
            else
            {
                return 0;
            }
        }

        uint32 getVspacing()
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getVspacing();
            }
            else
            {
                return 0;
            }
        }

        int32 getNextBundledAccessUnits(uint32 *n,
                                        GAU    *pgau)
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getNextBundledAccessUnits(n, pgau);
            }
            else
            {
                return -1;
            }
        }

        int32 peekNextBundledAccessUnits(uint32 *n,
                                         MediaMetaInfo *mInfo)
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->peekNextBundledAccessUnits(n, mInfo);
            }
            else
            {
                return -1;
            }
        }

        uint32 getSampleCount()
        {
            if (_pmediaAtom != NULL)
            {
                return (_pmediaAtom->getSampleCount());
            }
            else
            {
                return 0;
            }
        }

        int16 getLayer()
        {
            if (_ptrackHeader != NULL)
            {
                return _ptrackHeader->getLayer();
            }
            else
            {
                return (-1);
            }
        }
        uint16 getAlternateGroup()
        {
            if (_ptrackHeader != NULL)
            {
                return _ptrackHeader->getAlternateGroup();
            }
            else
            {
                return (0xFFFF);
            }
        }
        int32 getTextTrackWidth()
        {
            if (_ptrackHeader != NULL)
            {
                return _ptrackHeader->getTextTrackWidth();
            }
            else
            {
                return (-1);
            }
        }

        int32 getTextTrackHeight()
        {
            if (_ptrackHeader != NULL)
            {
                return _ptrackHeader->getTextTrackHeight();
            }
            else
            {
                return (-1);
            }
        }

        int32 getTextTrackXOffset()
        {
            if (_ptrackHeader != NULL)
            {
                return _ptrackHeader->getTextTrackXOffset();
            }
            else
            {
                return (-1);
            }
        }

        int32 getTextTrackYOffset()
        {
            if (_ptrackHeader != NULL)
            {
                return _ptrackHeader->getTextTrackYOffset();
            }
            else
            {
                return (-1);
            }
        }

        SampleEntry *getTextSampleEntryAt(uint32 index)
        {
            if (_pmediaAtom != NULL)
            {
                return (_pmediaAtom-> getTextSampleEntryAt(index));
            }
            else
            {
                return NULL;
            }
        }

        int32 getTimestampForRandomAccessPoints(uint32 *num, uint32 *tsBuf, uint32* numBuf, uint32* offsetBuf)
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getTimestampForRandomAccessPoints(num, tsBuf, numBuf, offsetBuf);
            }
            else
            {
                return 0;
            }
        }

        int32 getTimestampForRandomAccessPointsBeforeAfter(uint32 ts, uint32 *tsBuf, uint32* numBuf,
                uint32& numsamplestoget,
                uint32 howManyKeySamples)
        {
            if (_pmediaAtom != NULL)
            {
                return _pmediaAtom->getTimestampForRandomAccessPointsBeforeAfter(ts, tsBuf, numBuf, numsamplestoget, howManyKeySamples);
            }
            else
            {
                return 0;
            }

        }
        int32 getNumAMRFramesPerSample()
        {
            if (_pmediaAtom != NULL)
            {
                return (_pmediaAtom->getNumAMRFramesPerSample());
            }
            else
            {
                return 0;
            }
        }

        MP4_ERROR_CODE getMaxTrackTimeStamp(uint32 fileSize, uint32& timeStamp)
        {
            if (_pmediaAtom != NULL)
            {
                return (_pmediaAtom->getMaxTrackTimeStamp(fileSize, timeStamp));
            }
            else
            {
                return DEFAULT_ERROR;
            }
        }

        MP4_ERROR_CODE getSampleNumberClosestToTimeStamp(uint32 &sampleNumber,
                uint32 timeStamp,
                uint32 sampleOffset = 0)
        {
            if (_pmediaAtom != NULL)
            {
                return
                    (_pmediaAtom->getSampleNumberClosestToTimeStamp(sampleNumber,
                            timeStamp,
                            sampleOffset));
            }
            else
            {
                return (READ_FAILED);
            }
        }

        AVCSampleEntry* getAVCSampleEntry(uint32 index)
        {
            if (_pmediaAtom != NULL)
            {
                return (_pmediaAtom->getAVCSampleEntry(index));
            }
            return (NULL);
        }

        uint32 getAVCNALLengthSize(uint32 index)
        {
            if (_pmediaAtom != NULL)
            {
                return (_pmediaAtom->getAVCNALLengthSize(index));
            }
            return 0;
        }

        uint32 getNumAVCSampleEntries()
        {
            if (_pmediaAtom != NULL)
            {
                return (_pmediaAtom->getNumAVCSampleEntries());
            }
            return 0;
        }

        uint32 getTrackLevelOMA2DRMInfoSize()
        {
            if (_pmediaAtom != NULL)
            {
                return (_pmediaAtom->getTrackLevelOMA2DRMInfoSize());
            }
            return 0;
        }

        uint8* getTrackLevelOMA2DRMInfo()
        {
            if (_pmediaAtom != NULL)
            {
                return (_pmediaAtom->getTrackLevelOMA2DRMInfo());
            }
            return NULL;
        }

        bool isMultipleSampleDescriptionAvailable()
        {
            if (_pmediaAtom != NULL)
            {
                return (_pmediaAtom->isMultipleSampleDescriptionAvailable());
            }
            return 0;
        }

    private:
        UserDataAtom         *_puserdataatom;
        EditAtom             *_pEditAtom;
        TrackHeaderAtom      *_ptrackHeader;
        TrackReferenceAtom   *_ptrackReference;
        MediaAtom            *_pmediaAtom;
        uint32 _trackStartOffset;
        int32  _pMediaType;

        OMADRMKMSBox* _pOMADRMKMSBox;
};

#endif // TRACKATOM_H_INCLUDED


