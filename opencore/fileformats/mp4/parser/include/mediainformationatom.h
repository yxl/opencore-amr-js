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
/*                       MPEG-4 MediaInformationAtom Class                       */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MediaInformationAtom Class contains all the objects that declare
    characteristic information about the media data within the stream.
*/


#ifndef MEDIAINFORMATIONATOM_H_INCLUDED
#define MEDIAINFORMATIONATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef MEDIAINFORMATIONHEADERATOM_H_INCLUDED
#include "mediainformationheaderatom.h"
#endif

#ifndef DATAINFORMATIONATOM_H_INCLUDED
#include "datainformationatom.h"
#endif

#ifndef SAMPLETABLEATOM_H_INCLUDED
#include "sampletableatom.h"
#endif

#ifndef ESDESCRIPTOR_H_INCLUDED
#include "esdescriptor.h"
#endif

#ifndef OSCL_MEDIA_DATA_H_INCLUDED
#include "oscl_media_data.h"
#endif

#ifndef PV_GAU_H_INCLUDED
#include "pv_gau.h"
#endif

class AVCSampleEntry;

class MediaInformationAtom : public Atom
{

    public:
        MediaInformationAtom(MP4_FF_FILE *fp,
                             uint32 mediaType,
                             OSCL_wString& filename,
                             bool oPVContentDownloadable = false,
                             uint32 parsingMode = 0);

        virtual ~MediaInformationAtom();

        // Member gets and creates
        const MediaInformationHeaderAtom* getMediaInformationHeader() const
        {
            return _pmediaInformationHeader;
        }

        MediaInformationHeaderAtom* getMutableMediaInformationHeader()
        {
            return _pmediaInformationHeader;
        }

        const DataInformationAtom& getDataInformationAtom() const
        {
            return *_pdataInformationAtom;
        }

        const SampleTableAtom& getSampleTableAtom() const
        {
            return *_psampleTableAtom;
        }

        int32 getTimestampForRandomAccessPoints(uint32 *num, uint32 *tsBuf, uint32* numBuf, uint32* offsetBuf)
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getTimestampForRandomAccessPoints(num, tsBuf, numBuf, offsetBuf);
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
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getTimestampForRandomAccessPointsBeforeAfter(ts, tsBuf, numBuf, numsamplestoget, howManyKeySamples);
            }
            else
            {
                return 0;
            }

        }

        // Get the type of SampleEntry in this track (MPEG_SAMPLE_ENTRY, VIDEO_SAMPLE_ENTRY , etc...)
        uint32 getSampleProtocol()
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getSampleProtocol();
            }
            else
            {
                return 0;
            }
        }

        // Return the ESID for the track - undefined if a HINT track
        uint32 getESID() const
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getESID();
            }
            else
            {
                return 0;
            }
        }

        // Return the first ESDescriptor for the track - undefined if a HINT track
        const ESDescriptor *getESDescriptor() const
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getESDescriptor();
            }
            else
            {
                return NULL;
            }
        }


        int32 updateFileSize(uint32	filesize)
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->updateFileSize(filesize);
            }
            return DEFAULT_ERROR;
        }

        // Returns next video frame
        int32 getNextSample(uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset)
        {
            if (_psampleTableAtom == NULL)
            {
                return READ_SAMPLE_TABLE_ATOM_FAILED;
            }
            return _psampleTableAtom->getNextSample(buf, size, index, SampleOffset);
        }

        MP4_ERROR_CODE getKeyMediaSampleNumAt(uint32 aKeySampleNum,
                                              GAU    *pgau)
        {
            if (_psampleTableAtom == NULL)
            {
                return READ_SAMPLE_TABLE_ATOM_FAILED;
            }
            return _psampleTableAtom->getKeyMediaSampleNumAt(aKeySampleNum, pgau);
        }

        uint32 getNumKeyFrames()
        {
            if (_psampleTableAtom != NULL)
            {
                return (_psampleTableAtom->getNumKeyFrames());
            }
            else
            {
                return 0;
            }

        }

        int32 getPrevKeyMediaSample(uint32 inputtimestamp,
                                    uint32 &aKeySampleNum,
                                    uint32 *n,
                                    GAU    *pgau)
        {
            if (_psampleTableAtom == NULL)
            {
                return READ_SAMPLE_TABLE_ATOM_FAILED;
            }
            return _psampleTableAtom->getPrevKeyMediaSample(inputtimestamp, aKeySampleNum, n, pgau);
        }

        int32 getNextKeyMediaSample(uint32 inputtimestamp,
                                    uint32 &aKeySampleNum,
                                    uint32 *n,
                                    GAU    *pgau)
        {
            if (_psampleTableAtom == NULL)
            {
                return READ_SAMPLE_TABLE_ATOM_FAILED;
            }
            return _psampleTableAtom->getNextKeyMediaSample(inputtimestamp, aKeySampleNum, n, pgau);
        }

        int32 getMediaSample(uint32 sampleNumber, uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset)
        {
            if (_psampleTableAtom == NULL)
            {
                return READ_SAMPLE_TABLE_ATOM_FAILED;
            }
            return _psampleTableAtom->getSample(sampleNumber, buf, size, index, SampleOffset);
        }

        int32 getOffsetByTime(uint32 ts, int32* sampleFileOffset)
        {
            if (_psampleTableAtom == NULL)
            {
                return DEFAULT_ERROR;
            }
            return _psampleTableAtom->getOffsetByTime(ts, sampleFileOffset);
        }

        void resetPlayBack()
        {
            if (_psampleTableAtom != NULL)
            {
                _psampleTableAtom->resetPlayBack();
            }
        }

        void resetTrackToEOT()
        {
            if (_psampleTableAtom != NULL)
            {
                _psampleTableAtom->resetTrackToEOT();
            }
        }

        int32 resetPlayBack(int32 time, bool oDependsOn)
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->resetPlayBackbyTime(time, oDependsOn);
            }
            else
            {
                return 0;
            }
        }

        int32 queryRepositionTime(int32 time, bool oDependsOn, bool bBeforeRequestedTime)
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->queryRepositionTime(time, oDependsOn, bBeforeRequestedTime);
            }
            else
            {
                return 0;
            }
        }


        int32 IsResetNeeded(int32 time)
        {
            if (_psampleTableAtom == NULL)
                return READ_SAMPLE_TABLE_ATOM_FAILED;
            return _psampleTableAtom->IsResetNeeded(time);
        }

        uint32 getTimestampForSampleNumber(uint32 sampleNumber)
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getTimestampForSampleNumber(sampleNumber);
            }
            else
            {
                return 0;
            }
        }

        int32 getSampleSizeAt(int32 sampleNum)
        {
            if (_psampleTableAtom != NULL)
            {
                return   _psampleTableAtom->getSampleSizeAt(sampleNum);
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
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getTimestampForCurrentSample();
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
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getSampleNumberForCurrentSample();
            }
            else
            {
                return 0;
            }
        }

        void advance()
        {
            if (_psampleTableAtom != NULL)
            {
                _psampleTableAtom->advance();
            }
        } // Advances the currentPlayback number

        void advance(int32 sampleNum)
        {
            if (_psampleTableAtom != NULL)
            {
                _psampleTableAtom->advance(sampleNum);
            }
        } // Advances the currentPlayback number to sampleNum

        void seek(int32 baseLayerSampleNum)
        {
            if (_psampleTableAtom != NULL)
            {
                _psampleTableAtom->seek(baseLayerSampleNum);
            }
        } // Seeks to the base-layer sample in the hint track

        uint32 getNumSampleEntries()
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getNumSampleEntries();
            }
            else
            {
                return 0;
            }
        }

        // Getting and setting the Mpeg4 VOL header
        DecoderSpecificInfo *getDecoderSpecificInfo() const
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getDecoderSpecificInfo();
            }
            else
            {
                return NULL;
            }
        }

        DecoderSpecificInfo *getDecoderSpecificInfoForSDI(uint32 index) const
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getDecoderSpecificInfoForSDI(index);
            }
            else
            {
                return NULL;
            }
        }

        void getMIMEType(OSCL_String& aMimeType)
        {
            if (_psampleTableAtom != NULL)
            {
                _psampleTableAtom->getMIMEType(aMimeType);
            }
        }

        uint8  getObjectTypeIndication()
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getObjectTypeIndication();
            }
            else
            {
                return (0xFF);
            }
        }

        uint32 getMaxBufferSizeDB() const
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getMaxBufferSizeDB();
            }
            else
            {
                return 0;
            }
        }

        int32 getAverageBitrate()
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getAverageBitrate();
            }
            else
            {
                return 0;
            }
        }

        int32 getHeight()
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getHeight();
            }
            else
            {
                return 0;
            }

        }

        int32 getWidth()
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getWidth();
            }
            else
            {
                return 0;
            }
        }

        //PASP Box
        uint32 getHspacing()
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getHspacing();
            }
            else
            {
                return 0;
            }
        }

        uint32 getVspacing()
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getVspacing();
            }
            else
            {
                return 0;
            }
        }

        // TS offset value for the start of the media track.  The STTS
        // Atom only holds TS deltas.  For a track that does not begin at 0s, we need to hold an
        // offset timestamp value.
        void setTrackTSOffset(uint32 ts)
        {
            _trackStartOffset = ts;

            if (_psampleTableAtom != NULL)
            {
                _psampleTableAtom->setTrackTSOffset(ts);
            }
        }

        uint32 getSampleDescriptionIndex()
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getSampleDescriptionIndex();
            }
            else
            {
                return 0;
            }
        }

        int32 getNextBundledAccessUnits(uint32 *n,
                                        GAU    *pgau)
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->getNextBundledAccessUnits(n, pgau);
            }
            else
            {
                return -1;
            }
        }

        int32 peekNextBundledAccessUnits(uint32 *n,
                                         MediaMetaInfo *mInfo)
        {
            if (_psampleTableAtom != NULL)
            {
                return _psampleTableAtom->peekNextBundledAccessUnits(n, mInfo);
            }
            else
            {
                return -1;
            }
        }

        uint32 getSampleCount()
        {
            if (_psampleTableAtom != NULL)
            {
                return (_psampleTableAtom->getSampleCount());
            }
            else
            {
                return 0;
            }
        }

        SampleEntry *getTextSampleEntryAt(uint32 index)
        {
            if (_psampleTableAtom != NULL)
            {
                return (_psampleTableAtom-> getTextSampleEntryAt(index));
            }
            else
            {
                return NULL;
            }
        }

        int32 getNumAMRFramesPerSample()
        {
            if (_psampleTableAtom != NULL)
            {
                return (_psampleTableAtom->getNumAMRFramesPerSample());
            }
            else
            {
                return 0;
            }
        }

        MP4_ERROR_CODE getMaxTrackTimeStamp(uint32 fileSize, uint32& timeStamp)
        {
            if (_psampleTableAtom != NULL)
            {
                return (_psampleTableAtom->getMaxTrackTimeStamp(fileSize, timeStamp));
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
            if (_psampleTableAtom != NULL)
            {
                return
                    (_psampleTableAtom->getSampleNumberClosestToTimeStamp(sampleNumber,
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
            if (_psampleTableAtom != NULL)
            {
                return (_psampleTableAtom->getAVCSampleEntry(index));
            }
            return (NULL);
        }

        uint32 getAVCNALLengthSize(uint32 index)
        {
            if (_psampleTableAtom != NULL)
            {
                return (_psampleTableAtom->getAVCNALLengthSize(index));
            }
            return 0;
        }

        uint32 getNumAVCSampleEntries()
        {
            if (_psampleTableAtom != NULL)
            {
                return (_psampleTableAtom->getNumAVCSampleEntries());
            }
            return 0;
        }

        uint32 getTrackLevelOMA2DRMInfoSize()
        {
            if (_psampleTableAtom != NULL)
            {
                return (_psampleTableAtom->getTrackLevelOMA2DRMInfoSize());
            }
            return 0;
        }

        uint8* getTrackLevelOMA2DRMInfo()
        {
            if (_psampleTableAtom != NULL)
            {
                return (_psampleTableAtom->getTrackLevelOMA2DRMInfo());
            }
            return NULL;
        }

        bool isMultipleSampleDescriptionAvailable()
        {
            if (_psampleTableAtom != NULL)
            {
                return (_psampleTableAtom->isMultipleSampleDescriptionAvailable());
            }
            return 0;
        }

    private:

        // This is actually a pointer to the base class.  The possible
        // derived classes include VideoMediaHeaderAtom, SoundMediaHeaderAtom,
        // HintMediaHeaderAtom, and Mpeg4MediaHeaderAtom.
        MediaInformationHeaderAtom *_pmediaInformationHeader;
        DataInformationAtom        *_pdataInformationAtom;
        SampleTableAtom            *_psampleTableAtom;

        uint32 _trackStartOffset;

};

#endif // MEDIAINFORMATIONATOM_H_INCLUDED

