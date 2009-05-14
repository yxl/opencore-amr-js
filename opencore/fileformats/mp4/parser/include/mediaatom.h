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
/*                            MPEG-4 MediaAtom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MediaAtom Class contains all the objects that declare information
    about the media data within the stream.
*/


#ifndef MEDIAATOM_H_INCLUDED
#define MEDIAATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef MEDIAHEADERATOM_H_INCLUDED
#include "mediaheaderatom.h"
#endif

#ifndef HANDLERATOM_H_INCLUDED
#include "handleratom.h"
#endif

#ifndef MEDIAINFORMATIONATOM_H_INCLUDED
#include "mediainformationatom.h"
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

class MediaAtom : public Atom
{

    public:
        MediaAtom(MP4_FF_FILE *fp,
                  OSCL_wString& filename,
                  uint32 size,
                  uint32 type,
                  bool oPVContentDownloadable = false,
                  uint32 parsingMode = 0);

        virtual ~MediaAtom();


        uint32 getMediaHandlerType() const
        {
            if (_phandler != NULL)
            {
                return _phandler->getHandlerType();
            }
            else
            {
                return 0;
            }
        }

        uint32 getSampleProtocol() const
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getSampleProtocol();
            }
            else
            {
                return 0;
            }
        }


        // Return the ESID for the track - undefined if a HINT track
        uint32 getESID() const
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getESID();
            }
            else
            {
                return 0;
            }
        }

        // Return the first ESDescriptor for the track - undefined if a HINT track
        const ESDescriptor *getESDescriptor() const
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getESDescriptor();
            }
            else
            {
                return NULL;
            }
        }

        uint32 getTimeScale() const
        {
            if (_pmediaHeader != NULL)
            {
                return _pmediaHeader->getTimeScale();
            }
            else
            {
                // UNDEFINED VALUE
                return 0xFFFFFFFF;
            }
        }

        int32 updateFileSize(uint32	filesize)
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->updateFileSize(filesize);
            }
            return DEFAULT_ERROR;
        }

        // Returns next video frame
        int32 getNextSample(uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset)
        {
            if (_pmediaInformation == NULL)
            {
                return READ_MEDIA_INFORMATION_ATOM_FAILED;
            }
            return _pmediaInformation->getNextSample(buf, size, index, SampleOffset);
        }

        MP4_ERROR_CODE getKeyMediaSampleNumAt(uint32 aKeySampleNum,
                                              GAU    *pgau)
        {
            if (_pmediaInformation == NULL)
            {
                return READ_MEDIA_INFORMATION_ATOM_FAILED;
            }
            return _pmediaInformation->getKeyMediaSampleNumAt(aKeySampleNum, pgau);
        }

        uint32 getNumKeyFrames()
        {
            if (_pmediaInformation != NULL)
            {
                return (_pmediaInformation->getNumKeyFrames());
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
            if (_pmediaInformation == NULL)
            {
                return READ_MEDIA_INFORMATION_ATOM_FAILED;
            }
            return _pmediaInformation->getPrevKeyMediaSample(inputtimestamp, aKeySampleNum, n, pgau);
        }

        int32 getNextKeyMediaSample(uint32 inputtimestamp,
                                    uint32 &aKeySampleNum,
                                    uint32 *n,
                                    GAU    *pgau)
        {
            if (_pmediaInformation == NULL)
            {
                return READ_MEDIA_INFORMATION_ATOM_FAILED;
            }
            return _pmediaInformation->getNextKeyMediaSample(inputtimestamp, aKeySampleNum, n, pgau);
        }

        int32 getMediaSample(uint32 sampleNumber, uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset)
        {
            if (_pmediaInformation == NULL)
            {
                return READ_MEDIA_INFORMATION_ATOM_FAILED;
            }
            return _pmediaInformation->getMediaSample(sampleNumber, buf, size, index, SampleOffset);
        }

        int32 getOffsetByTime(uint32 ts, int32* sampleFileOffset)
        {
            if (_pmediaInformation == NULL)
            {
                return DEFAULT_ERROR;
            }
            return _pmediaInformation->getOffsetByTime(ts, sampleFileOffset);
        }

        void resetPlayBack()
        {
            if (_pmediaInformation != NULL)
            {
                _pmediaInformation->resetPlayBack();
            }
        }

        void resetTrackToEOT()
        {
            if (_pmediaInformation != NULL)
            {
                _pmediaInformation->resetTrackToEOT();
            }
        }

        int32 resetPlayBack(int32 time, bool oDependsOn)
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->resetPlayBack(time, oDependsOn);
            }
            else
            {
                return 0;
            }
        }

        int32 queryRepositionTime(int32 time, bool oDependsOn, bool bBeforeRequestedTime)
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->queryRepositionTime(time, oDependsOn, bBeforeRequestedTime);
            }
            else
            {
                return 0;
            }
        }

        int32 IsResetNeeded(int32 time)
        {
            if (_pmediaInformation == NULL)
                return READ_MEDIA_INFORMATION_ATOM_FAILED;
            return _pmediaInformation->IsResetNeeded(time);
        }

        uint32 getTimestampForSampleNumber(uint32 sampleNumber)
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getTimestampForSampleNumber(sampleNumber);
            }
            else
            {
                return 0;
            }
        }

        int32 getSampleSizeAt(int32 sampleNum)
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getSampleSizeAt(sampleNum);
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
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getTimestampForCurrentSample();
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
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getSampleNumberForCurrentSample();
            }
            else
            {
                return 0;
            }
        }

        void advance()
        {
            if (_pmediaInformation != NULL)
            {
                _pmediaInformation->advance();
            }
        } // Advances the currentPlayback number

        void advance(int32 sampleNum)
        {
            if (_pmediaInformation != NULL)
            {
                _pmediaInformation->advance(sampleNum);
            }
        } // Advances the currentPlayback number to sampleNum

        void seek(int32 baseLayerSampleNum)
        {
            if (_pmediaInformation != NULL)
            {
                _pmediaInformation->seek(baseLayerSampleNum);
            }
        } // Seeks to the base-layer sample in the hint track



        // From MediaHeader
        uint64 getMediaDuration()
        {
            if (_pmediaHeader != NULL)
            {
                return _pmediaHeader->getDuration();
            }
            else
            {
                return 0;
            }
        } // in terms of the media timescale

        uint32 getMediaTimescale()
        {
            if (_pmediaHeader != NULL)
            {
                return _pmediaHeader->getTimeScale();
            }
            else
            {
                return 0;
            }
        }

        uint16 getLanguageCode()
        {
            if (_pmediaHeader != NULL)
            {
                return _pmediaHeader->getLanguage();
            }
            else
            {
                return (0xFFFF);
            }

        }


        // From Handler
        uint32 getTrackStreamType()
        {
            if (_phandler != NULL)
            {
                return _phandler->getHandlerType();
            }
            else
            {
                return 0;
            }
        }

        // From SampleDescription
        uint32 getNumSampleEntries()
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getNumSampleEntries();
            }
            else
            {
                return 0;
            }
        }

        //From DecoderConfigDescriptor
        // Getting and setting the Mpeg4 VOL header
        DecoderSpecificInfo *getDecoderSpecificInfo() const
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getDecoderSpecificInfo();
            }
            else
            {
                return NULL;
            }
        }

        DecoderSpecificInfo *getDecoderSpecificInfoForSDI(uint32 index) const
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getDecoderSpecificInfoForSDI(index);
            }
            else
            {
                return NULL;
            }
        }

        void getMIMEType(OSCL_String& aMimeType)
        {
            if (_pmediaInformation != NULL)
            {
                _pmediaInformation->getMIMEType(aMimeType);
            }
        }

        uint8  getObjectTypeIndication()
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getObjectTypeIndication();
            }
            else
            {
                return 0;
            }
        }

        // Get the max size buffer needed to retrieve the media samples
        uint32 getMaxBufferSizeDB() const
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getMaxBufferSizeDB();
            }
            else
            {
                return 0;
            }
        }

        int32 getAverageBitrate()
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getAverageBitrate();
            }
            else
            {
                return 0;
            }
        }

        int32 getHeight()
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getHeight();
            }
            else
            {
                return 0;
            }

        }

        int32 getWidth()
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getWidth();
            }
            else
            {
                return 0;
            }
        }

        //PASP box
        uint32 getHspacing()
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getHspacing();
            }
            else
            {
                return 0;
            }
        }

        uint32 getVspacing()
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getVspacing();
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
            if (_pmediaInformation != NULL)
            {
                _pmediaInformation->setTrackTSOffset(ts);
            }
        }

        uint32 getSampleDescriptionIndex()
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getSampleDescriptionIndex();
            }
            else
            {
                return (0xFFFFFFFF);
            }
        }

        int32 getNextBundledAccessUnits(uint32 *n,
                                        GAU    *pgau)
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getNextBundledAccessUnits(n, pgau);
            }
            else
            {
                return -1;
            }
        }

        int32 peekNextBundledAccessUnits(uint32 *n,
                                         MediaMetaInfo *mInfo)
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->peekNextBundledAccessUnits(n, mInfo);
            }
            else
            {
                return -1;
            }
        }

        uint32 getSampleCount()
        {
            if (_pmediaInformation != NULL)
            {
                return (_pmediaInformation->getSampleCount());
            }
            else
            {
                return 0;
            }
        }

        SampleEntry *getTextSampleEntryAt(uint32 index)
        {
            if (_pmediaInformation != NULL)
            {
                return (_pmediaInformation-> getTextSampleEntryAt(index));
            }
            else
            {
                return NULL;
            }
        }

        int32 getTimestampForRandomAccessPoints(uint32 *num, uint32 *tsBuf, uint32* numBuf, uint32 *offsetBuf)
        {
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getTimestampForRandomAccessPoints(num, tsBuf, numBuf, offsetBuf);
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
            if (_pmediaInformation != NULL)
            {
                return _pmediaInformation->getTimestampForRandomAccessPointsBeforeAfter(ts, tsBuf, numBuf, numsamplestoget, howManyKeySamples);
            }
            else
            {
                return 0;
            }

        }


        int32 getNumAMRFramesPerSample()
        {
            if (_pmediaInformation != NULL)
            {
                return (_pmediaInformation->getNumAMRFramesPerSample());
            }
            else
            {
                return 0;
            }
        }

        MP4_ERROR_CODE getMaxTrackTimeStamp(uint32 fileSize, uint32& timeStamp)
        {
            if (_pmediaInformation != NULL)
            {
                return (_pmediaInformation->getMaxTrackTimeStamp(fileSize, timeStamp));
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
            if (_pmediaInformation != NULL)
            {
                return
                    (_pmediaInformation->getSampleNumberClosestToTimeStamp(sampleNumber,
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
            if (_pmediaInformation != NULL)
            {
                return (_pmediaInformation->getAVCSampleEntry(index));
            }
            return (NULL);
        }

        uint32 getAVCNALLengthSize(uint32 index)
        {
            if (_pmediaInformation != NULL)
            {
                return (_pmediaInformation->getAVCNALLengthSize(index));
            }
            return 0;
        }

        uint32 getNumAVCSampleEntries()
        {
            if (_pmediaInformation != NULL)
            {
                return (_pmediaInformation->getNumAVCSampleEntries());
            }
            return 0;
        }

        uint32 getTrackLevelOMA2DRMInfoSize()
        {
            if (_pmediaInformation != NULL)
            {
                return (_pmediaInformation->getTrackLevelOMA2DRMInfoSize());
            }
            return 0;
        }

        uint8* getTrackLevelOMA2DRMInfo()
        {
            if (_pmediaInformation != NULL)
            {
                return (_pmediaInformation->getTrackLevelOMA2DRMInfo());
            }
            return NULL;
        }

        bool isMultipleSampleDescriptionAvailable()
        {
            if (_pmediaInformation != NULL)
            {
                return (_pmediaInformation->isMultipleSampleDescriptionAvailable());
            }
            return 0;
        }

    private:
        MediaHeaderAtom *_pmediaHeader;
        HandlerAtom *_phandler;
        MediaInformationAtom *_pmediaInformation;

        uint32 _trackStartOffset;
};

#endif // MEDIAATOM_H_INCLUDED

