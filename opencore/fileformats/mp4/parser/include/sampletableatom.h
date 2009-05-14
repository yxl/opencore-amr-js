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
/*                         MPEG-4 SampleTableAtom Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This SampleTableAtom Class contains all the time and data indexing of the
    media samples in a track.
*/


#ifndef SAMPLETABLEATOM_H_INCLUDED
#define SAMPLETABLEATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef TIMETOSAMPLEATOM_H_INCLUDED
#include "timetosampleatom.h"
#endif

#ifndef COMPOSITIONOFFSETATOM_H_INCLUDED
#include "compositionoffsetatom.h"
#endif

#ifndef SAMPLESIZEATOM_H_INCLUDED
#include "samplesizeatom.h"
#endif

#ifndef CHUNKOFFSETATOM_H_INCLUDED
#include "chunkoffsetatom.h"
#endif

#ifndef SAMPLETOCHUNKATOM_H_INCLUDED
#include "sampletochunkatom.h"
#endif

#ifndef SAMPLEDESCRIPTIONATOM_H_INCLUDED
#include "sampledescriptionatom.h"
#endif

#ifndef SYNCSAMPLEATOM_H_INCLUDED
#include "syncsampleatom.h"
#endif

#ifndef ESDESCRIPTOR_H_INCLUDED
#include "esdescriptor.h"
#endif

#ifndef ATOMDEFS_H_INCLUDED
#include "atomdefs.h"
#endif

#ifndef OSCL_MEDIA_DATA_H_INCLUDED
#include "oscl_media_data.h"
#endif

#ifndef PV_GAU_H_INCLUDED
#include "pv_gau.h"
#endif

#ifndef AVCSAMPLEDEPENDENCYTYPE_H_INCLUDED
#include "avcsampledependencytype.h"
#endif

#ifndef AVCSAMPLETOGROUPBOX_H_INCLUDED
#include "avcsampletogroupbox.h"
#endif

#ifndef AVCSAMPLEDEPENDENCY_H_INCLUDED
#include "avcsampledependency.h"
#endif

//Macro to give number of frames that one must go back
//in order to retrieve sample number corresponding to requested timestamp
#define BACK_TRAVERSE_FRAME_COUNT 10

class AVCSampleEntry;

class SampleTableAtom : public Atom
{

    public:
        SampleTableAtom(MP4_FF_FILE *fp,
                        uint32 mediaType,
                        OSCL_wString& filename,
                        uint32 size,
                        uint32 type,
                        bool oPVContentDownloadable = false,
                        uint32 parsingMode = 0);

        virtual ~SampleTableAtom();

        // Member gets and sets
        TimeToSampleAtom &getTimeToSampleAtom()
        {
            return *_ptimeToSampleAtom;
        }
        TimeToSampleAtom &getMutableTimeToSampleAtom()
        {
            return *_ptimeToSampleAtom;
        }

        const SampleDescriptionAtom &getSampleDescriptionAtom() const
        {
            return *_psampleDescriptionAtom;
        }
        SampleDescriptionAtom &getMutableSampleDescriptionAtom()
        {
            return *_psampleDescriptionAtom;
        }

        SampleSizeAtom &getSampleSizeAtom()
        {
            return *_psampleSizeAtom;
        }
        SampleSizeAtom &getMutableSampleSizeAtom()
        {
            return *_psampleSizeAtom;
        }

        const SampleToChunkAtom &getSampleToChunkAtom() const
        {
            return *_psampleToChunkAtom;
        }
        SampleToChunkAtom &getMutableSampleToChunkAtom()
        {
            return *_psampleToChunkAtom;
        }

        const ChunkOffsetAtom &getChunkOffsetAtom() const
        {
            return *_pchunkOffsetAtom;
        }
        ChunkOffsetAtom &getMutableChunkOffsetAtom()
        {
            return *_pchunkOffsetAtom;
        }

        void  resetPlayBack();
        void  resetTrackToEOT();
        int32 resetPlayBackbyTime(int32 time, bool oDependsOn);
        int32 queryRepositionTime(int32 time, bool oDependsOn, bool bBeforeRequestedTime);

        int32 IsResetNeeded(int32 time);

        uint32 getTimestampForSampleNumber(uint32 sampleNumber);
        int32 getCttsOffsetForSampleNumber(uint32 sampleNumber);
        int32 getCttsOffsetForSampleNumberPeek(uint32 sampleNumber);
        int32 getCttsOffsetForSampleNumberGet(uint32 sampleNumber);
        int32 getSampleSizeAt(int32 sampleNum)
        {
            return getSampleSizeAtom().getSampleSizeAt(sampleNum);
        }

        // Get the type of SampleEntry atom (i.e. MPEG_SAMPLE_ENTRY, ... '<protocol>', etc.)
        uint32 getSampleProtocol()
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return _psampleDescriptionAtom->getSampleProtocol(_SDIndex);
            }
            else
            {
                return 0;
            }
        }

        // Return the ESID for the track - undefined if a HINT track
        uint32 getESID() const
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return _psampleDescriptionAtom->getESID(_SDIndex);
            }
            else
            {
                return 0;
            }
        }

        // Return the first ESDescriptor for the track - undefined if a HINT track
        const ESDescriptor *getESDescriptor() const
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return _psampleDescriptionAtom->getESDescriptor(_SDIndex);
            }
            else
            {
                return NULL;
            }
        }

        uint32 getNumSampleEntries()
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return _psampleDescriptionAtom->getNumSampleEntries();
            }
            else
            {
                return 0;
            }
        }

        uint32 getMaxBufferSizeDB() const
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return _psampleDescriptionAtom->getMaxBufferSizeDB();
            }
            else
            {
                return 0;
            }
        }

        // Getting and setting the Mpeg4 VOL header
        DecoderSpecificInfo *getDecoderSpecificInfo() const
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return _psampleDescriptionAtom->getDecoderSpecificInfo(_SDIndex);
            }
            else
            {
                return NULL;
            }
        }

        DecoderSpecificInfo *getDecoderSpecificInfoForSDI(uint32 index) const
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return _psampleDescriptionAtom->getDecoderSpecificInfo(index);
            }
            else
            {
                return NULL;
            }
        }

        void getMIMEType(OSCL_String& aMimeType)
        {
            if (_psampleDescriptionAtom != NULL)
            {
                _psampleDescriptionAtom->getMIMEType(aMimeType);
            }
        }

        uint8  getObjectTypeIndication()
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return _psampleDescriptionAtom->getObjectTypeIndication();
            }
            else
            {
                return 0;
            }
        }


        int32 getAverageBitrate()
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return _psampleDescriptionAtom->getAverageBitrate();
            }
            else
            {
                return 0;
            }
        }

        int32 getHeight()
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return _psampleDescriptionAtom->getHeight();
            }
            else
            {
                return 0;
            }

        }

        int32 getWidth()
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return _psampleDescriptionAtom->getWidth();
            }
            else
            {
                return 0;
            }
        }

        //PASP
        uint32 getHspacing()
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return _psampleDescriptionAtom->getHspacing();
            }
            else
            {
                return 0;
            }
        }

        uint32 getVspacing()
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return _psampleDescriptionAtom->getVspacing();
            }
            else
            {
                return 0;
            }
        }

        // Gets for optional member atoms
        int32 getTimestampForRandomAccessPoints(uint32 *num, uint32 *tsBuf, uint32* numBuf, uint32* offsetBuf);
        const SyncSampleAtom *getSyncSampleAtom() const
        {
            return _psyncSampleAtom;
        }

        // Returns next video frame
        int32 getNextSample(uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset);

        int32 getMediaSample(uint32 sampleNumber, uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset);

        // Returns next I-frame at time ts (in milliseconds) - or the very next I-frame in the stream
        int32 getNextSampleAtTime(uint32 ts, uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset);

        MP4_ERROR_CODE getKeyMediaSampleNumAt(uint32 aKeySampleNum,
                                              GAU    *pgau);
        int32 getPrevKeyMediaSample(uint32 inputtimestamp, uint32 &aKeySampleNum, uint32 *n, GAU    *pgau);
        int32 getNextKeyMediaSample(uint32 inputtimestamp, uint32 &aKeySampleNum, uint32 *n, GAU    *pgau);

        // Returns the timestamp (in milliseconds) for the last sample returned
        // This is mainly to be used when seeking in the bitstream - you request a frame at timestamp
        // X, but the actual frame you get is Y, this method returns the timestamp for Y so you know which
        // audio sample to request.
        int32 getTimestampForCurrentSample();

        // Returns the sample number of the last samplle returned
        // Used when requesting a hint sample for a specific randomly accessed sample
        int32 getSampleNumberForCurrentSample()
        {
            return _currentPlaybackSampleNumber;
        }
        int32 getSampleNumberAdjustedWithCTTS(uint32 aTs, int32 aSampleNumber);

        void advance()
        {
            _currentPlaybackSampleNumber++;    // Advances the currentPlayback number
        }
        void advance(int32 sampleNum)
        {
            _currentPlaybackSampleNumber = sampleNum;    // Advances the currentPlayback number to sampleNum
        }
        void seek(int32 baseLayerSampleNum); // Seeks to the base-layer sample in the hint track


        // TS offset value for the start of the media track.  The STTS
        // Atom only holds TS deltas.  For a track that does not begin at 0s, we need to hold an
        // offset timestamp value.
        void setTrackTSOffset(uint32 ts)
        {
            _trackStartTSOffset = ts;
        }

        int32 updateFileSize(uint32	filesize);

        uint32 getSampleDescriptionIndex()
        {
            return _SDIndex;
        }

        int32 getNextBundledAccessUnits(uint32 *n,
                                        GAU    *pgau);

        int32 peekNextBundledAccessUnits(uint32 *n,
                                         MediaMetaInfo *mInfo);

        int32 getNextNSamples(uint32 startSampleNum,
                              uint32 *n,
                              GAU *pgau);


        int32 peekNextNSamples(uint32 startSampleNum,
                               uint32 *n,
                               MediaMetaInfo    *mInfo);

        uint32 getSampleCount()
        {
            if (_psampleSizeAtom != NULL)
            {
                return (_psampleSizeAtom->getSampleCount());
            }
            else
            {
                return 0;
            }
        }

        bool IsSyncSample(uint32 sampleNum)
        {
            if (_psyncSampleAtom != NULL)
            {
                return (_psyncSampleAtom->IsSyncSample(sampleNum));
            }
            else
            {
                return false;
            }
        }

        uint32 getNumKeyFrames()
        {
            if (_psyncSampleAtom != NULL)
            {
                return (_psyncSampleAtom->getEntryCount());
            }
            else
            {
                if (_psampleSizeAtom != NULL)
                {
                    return (_psampleSizeAtom->getSampleCount());
                }
                else
                    return 0;
            }
        }
        SampleEntry *getTextSampleEntryAt(uint32 index)
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return (_psampleDescriptionAtom->getTextSampleEntryAt(index));
            }
            else
            {
                return NULL;
            }
        }

        int32 getOffsetByTime(uint32 ts, int32* sampleFileOffset);

        int32 getNumAMRFramesPerSample()
        {
            return _numAMRFramesPerSample;
        }

        MP4_ERROR_CODE getMaxTrackTimeStamp(uint32 fileSize, uint32& timeStamp);

        MP4_ERROR_CODE getSampleNumberClosestToTimeStamp(uint32 &sampleNumber,
                uint32 timeStamp,
                uint32 sampleOffset = 0);

        AVCSampleEntry* getAVCSampleEntry(uint32 index)
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return (_psampleDescriptionAtom->getAVCSampleEntry(index));
            }
            return (NULL);
        }

        uint32 getAVCNALLengthSize(uint32 index)
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return (_psampleDescriptionAtom->getAVCNALLengthSize(index));
            }
            return 0;
        }

        uint32 getNumAVCSampleEntries()
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return (_psampleDescriptionAtom->getNumAVCSampleEntries());
            }
            return 0;
        }
        bool isMultipleSampleDescriptionAvailable()
        {
            return _oMultipleSampleDescription;
        }

        int32 getSample(uint32 sampleNum, uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset);
        int32 getTimestampForRandomAccessPointsBeforeAfter(uint32 ts, uint32 *tsBuf, uint32* numBuf,
                uint32& numsamplestoget,
                uint32 howManyKeySamples);

        uint32 getTrackLevelOMA2DRMInfoSize()
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return (_psampleDescriptionAtom->getTrackLevelOMA2DRMInfoSize());
            }
            return 0;
        }

        uint8* getTrackLevelOMA2DRMInfo()
        {
            if (_psampleDescriptionAtom != NULL)
            {
                return (_psampleDescriptionAtom->getTrackLevelOMA2DRMInfo());
            }
            return NULL;
        }

    private:

        TimeToSampleAtom		*_ptimeToSampleAtom;
        CompositionOffsetAtom	*_pcompositionOffsetAtom;
        SampleDescriptionAtom *_psampleDescriptionAtom;
        SampleSizeAtom        *_psampleSizeAtom;
        SampleToChunkAtom     *_psampleToChunkAtom;
        ChunkOffsetAtom       *_pchunkOffsetAtom;
        AVCSampleDependencyType *_pavcSampleDependencyType;
        AVCSampleToGroup		*_pavcSampleToGroup;
        AVCSampleDependency		*_pavcSampleDependency;

        // Optional member atoms
        SyncSampleAtom        *_psyncSampleAtom;

        uint32 _currentPlaybackSampleTimestamp;
        int32 _currentPlaybackSampleNumber;

        // TS offset value for the start of the media track.  The STTS
        // Atom only holds TS deltas.  For a track that does not begin at 0s, we need to hold an
        // offset timestamp value.
        uint32 _trackStartTSOffset;

        uint32 _SDIndex;

        OSCL_wHeapString<OsclMemAllocator> _filename;
        MP4_FF_FILE *_pinput;

        uint32	_fileSize;
        uint32	_IsUpdateFileSize;

        int32 _numAMRFramesPerSample;

        uint8 *_pAMRTempBuffer;
        bool  _oResidualSample;
        int32 _remainingFramesInSample;
        uint32 _amrTempBufferOffset;
        int32  _amrFrameDelta;
        int32  _amrFrameTimeStamp;
        int32  _amrSampleSize;

        uint32 SamplesCount;
        MP4_FF_FILE *_commonFilePtr;

        bool _oPVContentDownloadable;
        uint32 _parsingMode;
        PVLogger *iLogger, *iStateVarLogger, *iParsedDataLogger, *iDiagnosticsLogger;
        bool _oMultipleSampleDescription;

        OSCL_wStackString<16> _defaultMimeType;
        uint32 _currChunkOffset;

};

#endif // SAMPLETABLEATOM_H_INCLUDED

