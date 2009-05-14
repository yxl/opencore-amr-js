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
/*                            MPEG-4 Mpeg4File Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    The Mpeg4File Class is the class that will construct and maintain all the
    mecessary data structures to be able to render a valid MP4 file to disk.
    Format.
*/


#ifndef MPEG4FILE_H_INCLUDED
#define MPEG4FILE_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef IMPEG4FILE_H_INCLUDED
#include "impeg4file.h"
#endif
#ifndef PARENTABLE_H_INCLUDED
#include "parentable.h"
#endif
#ifndef USERDATAATOM_H_INCLUDED
#include "userdataatom.h"
#endif
#ifndef MOVIEATOM_H_INCLUDED
#include "movieatom.h"
#endif
#ifndef MEDIADATAATOM_H_INCLUDED
#include "mediadataatom.h"
#endif
#ifndef ATOMDEFS_H_INCLUDED
#include "atomdefs.h"
#endif
#ifndef FILETYPEATOM_H_INCLUDED
#include "filetypeatom.h"
#endif
#ifndef COPYRIGHTATOM_H_INCLUDED
#include "copyrightatom.h"
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
#ifndef ATOMDEFS_H_INCLUDED
#include "atomdefs.h"
#endif

#ifndef OSCL_UTF8CONV_H_INCLUDED
#include "oscl_utf8conv.h"
#endif

#ifndef MOVIEFRAGMENTATOM_H_INCLUDED
#include "moviefragmentatom.h"
#endif

#ifndef MOVIEFRAGMENTRANDOMACCESSATOM_H_INCLUDED
#include "moviefragmentrandomaccess.h"
#endif

#ifndef MEDIA_CLOCK_CONVERTOR_H_INCLUDED
#include "media_clock_converter.h"
#endif

#ifndef PV_ID3_PARCOM_H_INCLUDED
#include "pv_id3_parcom.h"
#endif

#define ID3V1_STR_MAX_SIZE 64

class AVCSampleEntry;

class Mpeg4File : public IMpeg4File, public Parentable
{

    public:
        Mpeg4File(MP4_FF_FILE *fp,
                  OSCL_wString& filename,
                  uint32 parsingMode = 0); // Stream-in Constructor
        virtual ~Mpeg4File();

        int32 updateFileSize(uint32	filesize)
        {
            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->updateFileSize(filesize);
            }
            return DEFAULT_ERROR;
        }

        int32 getNextMediaSample(uint32 id, uint8 *buf, uint32 &size, uint32 &index, uint32 &SampleOffset)
        {
            if (_pmovieAtom == NULL)
            {
                return READ_MOVIE_ATOM_FAILED;
            }
            return _pmovieAtom->getNextMediaSample(id,  buf, size, index, SampleOffset);
        }

        MP4_ERROR_CODE getKeyMediaSampleNumAt(uint32 aTrackId,
                                              uint32 aKeySampleNum,
                                              GAU    *pgau);

        uint32 getNumKeyFrames(uint32 trackid)
        {
            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getNumKeyFrames(trackid);
            }
            else
            {
                return 0;
            }
        }

        int32 getPrevKeyMediaSample(uint32 inputtimestamp,
                                    uint32 &aKeySampleNum,
                                    uint32 id,
                                    uint32 *n,
                                    GAU    *pgau)
        {
            if (_pmovieAtom == NULL)
            {
                return READ_MOVIE_ATOM_FAILED;
            }
            return _pmovieAtom->getPrevKeyMediaSample(inputtimestamp, aKeySampleNum, id, n, pgau);
        }

        int32 getNextKeyMediaSample(uint32 inputtimestamp,
                                    uint32 &aKeySampleNum,
                                    uint32 id,
                                    uint32 *n,
                                    GAU    *pgau)
        {
            if (_pmovieAtom == NULL)
            {
                return READ_MOVIE_ATOM_FAILED;
            }
            return _pmovieAtom->getNextKeyMediaSample(inputtimestamp, aKeySampleNum, id, n, pgau);
        }

        int32 getMediaSample(uint32 id, uint32 sampleNumber, uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset)
        {
            if (_pmovieAtom == NULL)
            {
                return READ_MOVIE_ATOM_FAILED;
            }
            return _pmovieAtom->getMediaSample(id, sampleNumber, buf, size, index, SampleOffset);
        }

        int32 getOffsetByTime(uint32 id, uint32 ts, int32* sampleFileOffset , uint32 jitterbuffertimeinmillisec);


        uint32 getMediaTimestampForCurrentSample(uint32 id)
        {
            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getTimestampForCurrentSample(id);
            }
            else
            {
                return 0;
            }
        }

        int32 getTimestampForRandomAccessPoints(uint32 id, uint32 *num, uint32 *tsBuf, uint32* numBuf, uint32 *offsetBuf);


        int32 getTimestampForRandomAccessPointsBeforeAfter(uint32 id, uint32 ts, uint32 *tsBuf, uint32* numBuf,
                uint32& numsamplestoget,
                uint32 howManyKeySamples);


        //Return MPEG VOL header information
        virtual uint8* getTrackDecoderSpecificInfoContent(uint32 id);
        virtual uint32 getTrackDecoderSpecificInfoSize(uint32 id);
        uint32 getTimestampForSampleNumber(uint32 id, uint32 sampleNumber) ;
        int32 getSampleSizeAt(uint32 id, int32 sampleNum) ;

        virtual DecoderSpecificInfo* getTrackDecoderSpecificInfoAtSDI(uint32 trackID, uint32 index);

        virtual void resetPlayback();

        uint32 repositionFromMoof(uint32 time, uint32 trackID);

        void resetAllMovieFragments();


        virtual uint32 resetPlayback(uint32 time, uint16 numTracks, uint32 *trackList, bool bResetToIFrame);

        virtual int32 queryRepositionTime(uint32 time,
                                          uint16 numTracks,
                                          uint32 *trackList,
                                          bool bResetToIFrame,
                                          bool bBeforeRequestedTime);

        virtual int32	querySyncFrameBeforeTime(uint32 time, uint16 numTracks, uint32 *trackList)
        {
            if (_pmovieAtom == NULL)
            {
                return READ_MOVIE_ATOM_FAILED;
            }
            return _pmovieAtom->querySyncFrameBeforeTime(time, numTracks, trackList);
        }

        // pvmm
        virtual OSCL_wString& getPVTitle(MP4FFParserOriginalCharEnc &charType) ;
        virtual OSCL_wString& getPVAuthor(MP4FFParserOriginalCharEnc &charType) ;
        virtual OSCL_wString& getPVDescription(MP4FFParserOriginalCharEnc &charType) ;
        virtual OSCL_wString& getPVRating(MP4FFParserOriginalCharEnc &charType) ;
        virtual OSCL_wString& getPVCopyright(MP4FFParserOriginalCharEnc &charType) ;
        virtual OSCL_wString& getPVVersion(MP4FFParserOriginalCharEnc &charType) ;
        virtual OSCL_wHeapString<OsclMemAllocator> getCreationDate(MP4FFParserOriginalCharEnc &charType) ;

        // from 'ftyp' atom
        uint32 getCompatibiltyMajorBrand()
        {
            if (_pFileTypeAtom != NULL)
            {
                return	_pFileTypeAtom->getMajorBrand();
            }
            else
            {
                return 0;
            }
        }

        uint32 getCompatibiltyMajorBrandVersion()
        {
            if (_pFileTypeAtom != NULL)
            {
                return _pFileTypeAtom->getMajorBrandVersion();
            }
            else
            {
                return 0;
            }
        }

        Oscl_Vector<uint32, OsclMemAllocator> *getCompatibiltyList()
        {
            if (_pFileTypeAtom != NULL)
            {
                return _pFileTypeAtom->getCompatibleBrand();
            }
            else
            {
                return NULL;
            }
        }

        // From Movie
        int32 getNumTracks()
        {
            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getNumTracks();
            }
            else
            {
                return 0;
            }
        }

        int32 getTrackIDList(uint32 *ids, int size)
        {
            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getTrackIDList(ids, size);
            }
            else
            {
                return 0;
            }
        }

        uint32 getTrackWholeIDList(uint32 *ids)
        {
            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getTrackWholeIDList(ids);
            }
            else
            {
                return 0;
            }
        }


        // RETRIEVAL METHODS
        // Methods to get the sample rate (i.e. timescales) for the streams and
        // the overall Mpeg-4 presentation
        virtual uint64 getMovieDuration() const;
        virtual uint64 getMovieFragmentDuration() const;
        virtual uint32 getMovieTimescale() const;

        // From TrackHeader
        uint64 getTrackDuration(uint32 id);

        // From TrackReference
        uint32  trackDependsOn(uint32 id);

        // From MediaHeader
        uint64 getTrackMediaDurationForMovie(uint32 id);

        // From MediaHeader
        uint64 getTrackMediaDuration(uint32 id);
        uint32 getTrackMediaTimescale(uint32 id);
        uint16 getTrackLangCode(uint32 id);

        // From Handler
        uint32 getTrackMediaType(uint32 id);

        // From SampleDescription
        int32 getTrackNumSampleEntries(uint32 id);

        void getTrackMIMEType(uint32 id, OSCL_String& aMimeType); // Based on OTI value

        int32 getTrackMaxBufferSizeDB(uint32 id);
        int32 getTrackAverageBitrate(uint32 id);

        //From PASP atom
        uint32 getHspacing(uint32 id);
        uint32 getVspacing(uint32 id);



        // MPEG4 header retrieval methods
        int32 getFileType() const
        {
            return _fileType;
        }
        int32 getScalability() const
        {
            return _scalability;
        }
        int32 parseMFRA();

        virtual int32 peekNextBundledAccessUnits(const uint32 trackID,
                uint32 *n,
                MediaMetaInfo *mInfo);

        virtual int32 getNextBundledAccessUnits(const uint32 trackID, uint32 *n, GAU    *pgau);
        MovieFragmentAtom *getMovieFragmentForTrackId(uint32 id);

        uint32 getSampleCountInTrack(uint32 id);


        int16 getLayer(uint32 trackid)
        {
            return _pmovieAtom->getLayer(trackid);
        }
        uint16 getAlternateGroup(uint32 trackid)
        {
            return _pmovieAtom->getAlternateGroup(trackid);
        }
        int32 getVideoFrameHeight(uint32 trackid)
        {
            return _pmovieAtom->getTrackHeight(trackid);
        }

        int32 getVideoFrameWidth(uint32 trackid)
        {
            return _pmovieAtom->getTrackWidth(trackid);
        }

        int32 getTextTrackWidth(uint32 trackid)
        {
            return _pmovieAtom->getTextTrackWidth(trackid);
        }
        int32 getTextTrackHeight(uint32 trackid)
        {
            return _pmovieAtom->getTextTrackHeight(trackid);
        }
        int32 getTextTrackXOffset(uint32 trackid)
        {
            return _pmovieAtom->getTextTrackXOffset(trackid);
        }
        int32 getTextTrackYOffset(uint32 trackid)
        {
            return _pmovieAtom->getTextTrackYOffset(trackid);
        }

        SampleEntry *getTextSampleEntryAt(uint32 trackid, uint32 index)
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getTextSampleEntryAt(trackid, index));
            }
            else
            {
                return NULL;
            }
        }

        virtual bool IsMobileMP4();

        OSCL_wString& convertModificationTime(uint32 time);

//Metadata Retrieval Functions

        virtual PVMFStatus populateMetadataVectors();

        //Title Retrieval Functions
        virtual uint32 getNumTitle();
        virtual PVMFStatus getTitle(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType);
        virtual PVMFStatus populateTitleVector();

        //Author Retrieval Functions
        virtual uint32 getNumAuthor();
        virtual PVMFStatus getAuthor(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType);
        virtual PVMFStatus populateAuthorVector();

        //Album Retrieval Functions
        virtual uint32 getNumAlbum();
        virtual PVMFStatus getAlbum(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType);
        virtual PVMFStatus populateAlbumVector();

        //Artist Retrieval Functions
        virtual uint32 getNumArtist();
        virtual PVMFStatus getArtist(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType);
        virtual PVMFStatus populateArtistVector();

        //Genre Retrieval Functions
        virtual uint32 getNumGenre();
        virtual PVMFStatus getGenre(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType);
        virtual PVMFStatus populateGenreVector();

        //Year Retrieval Functions
        virtual uint32 getNumYear();
        virtual PVMFStatus getYear(uint32 index, uint32& aVal);
        virtual PVMFStatus populateYearVector();

        //Copyright Retrieval Functions
        virtual uint32 getNumCopyright();
        virtual PVMFStatus getCopyright(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType);
        virtual PVMFStatus populateCopyrightVector();

        //Comment Retrieval Functions
        virtual uint32 getNumComment();
        virtual PVMFStatus getComment(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType);
        virtual PVMFStatus populateCommentVector();

        //Description Retrieval Functions
        virtual uint32 getNumDescription();
        virtual PVMFStatus getDescription(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType);
        virtual PVMFStatus populateDescriptionVector();

        //Rating Retrieval Functions
        virtual uint32 getNumRating();
        virtual PVMFStatus getRating(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType);
        virtual PVMFStatus populateRatingVector();


        uint16 getAssetInfoTitleLangCode(int32 index) const
        {
            return (_pmovieAtom->getAssetInfoTitleLangCode(index));
        }
        OSCL_wString& getAssetInfoTitleNotice(MP4FFParserOriginalCharEnc &charType, int32 index) const
        {
            return (_pmovieAtom->getAssetInfoTitleNotice(charType, index));
        }
        uint16 getAssetInfoDescLangCode(int32 index) const
        {
            return (_pmovieAtom->getAssetInfoDescLangCode(index));
        }
        OSCL_wString& getAssetInfoDescNotice(MP4FFParserOriginalCharEnc &charType, int32 index) const
        {
            return (_pmovieAtom->getAssetInfoDescNotice(charType, index));
        }
        OSCL_wString& getCopyRightString(MP4FFParserOriginalCharEnc &charType, int32 index)
        {
            return (_pmovieAtom->getCopyRightString(charType, index));
        }
        uint16 getCopyRightLanguageCode(int32 index)
        {
            return (_pmovieAtom->getCopyRightLanguageCode(index));
        }
        uint16 getAssetInfoPerformerLangCode(int32 index) const
        {
            return (_pmovieAtom->getAssetInfoPerformerLangCode(index));
        }
        OSCL_wString& getAssetInfoPerformerNotice(MP4FFParserOriginalCharEnc &charType, int32 index) const
        {
            return (_pmovieAtom->getAssetInfoPerformerNotice(charType, index));
        }
        uint16 getAssetInfoAuthorLangCode(int32 index) const
        {
            return (_pmovieAtom->getAssetInfoAuthorLangCode(index));
        }
        OSCL_wString& getAssetInfoAuthorNotice(MP4FFParserOriginalCharEnc &charType, int32 index) const
        {
            return (_pmovieAtom->getAssetInfoAuthorNotice(charType, index));
        }
        uint16 getAssetInfoGenreLangCode(int32 index) const
        {
            return (_pmovieAtom->getAssetInfoGenreLangCode(index));
        }
        OSCL_wString& getAssetInfoGenreNotice(MP4FFParserOriginalCharEnc &charType, int32 index) const
        {
            return (_pmovieAtom->getAssetInfoGenreNotice(charType, index));
        }
        uint32 getAssetInfoRatingCriteria(int32 index) const
        {
            return (_pmovieAtom->getAssetInfoRatingCriteria(index));
        }
        uint32 getAssetInfoRatingEntity(int32 index) const
        {
            return (_pmovieAtom->getAssetInfoRatingEntity(index));
        }
        uint16 getAssetInfoRatingLangCode(int32 index) const
        {
            return (_pmovieAtom->getAssetInfoRatingLangCode(index));
        }
        OSCL_wString& getAssetInfoRatingNotice(MP4FFParserOriginalCharEnc &charType, int32 index) const
        {
            return (_pmovieAtom->getAssetInfoRatingNotice(charType, index));
        }
        uint32 getAssetInfoClassificationEntity(int32 index) const
        {
            return (_pmovieAtom->getAssetInfoClassificationEntity(index));
        }
        uint16 getAssetInfoClassificationTable(int32 index) const
        {
            return (_pmovieAtom->getAssetInfoClassificationTable(index));
        }
        uint16 getAssetInfoClassificationLangCode(int32 index) const
        {
            return (_pmovieAtom->getAssetInfoClassificationLangCode(index));
        }
        OSCL_wString& getAssetInfoClassificationNotice(MP4FFParserOriginalCharEnc &charType, int32 index) const
        {
            return (_pmovieAtom->getAssetInfoClassificationNotice(charType, index));
        }
        uint16 getAssetInfoNumKeyWords(int32 index) const
        {
            return (_pmovieAtom->getAssetInfoNumKeyWords(index));
        }
        uint16 getAssetInfoKeyWordLangCode(int32 index) const
        {
            return (_pmovieAtom->getAssetInfoKeyWordLangCode(index));
        }
        OSCL_wString& getAssetInfoKeyWord(int32 atomIndex, int32 keyWordIndex) const
        {
            return (_pmovieAtom->getAssetInfoKeyWord(atomIndex, keyWordIndex));
        }

        int32 getNumAssetInfoTitleAtoms()
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumAssetInfoTitleAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoDescAtoms()
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumAssetInfoDescAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumCopyRightAtoms()
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumCopyRightAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoPerformerAtoms()
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumAssetInfoPerformerAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoAuthorAtoms()
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumAssetInfoAuthorAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoGenreAtoms()
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumAssetInfoGenreAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoRatingAtoms()
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumAssetInfoRatingAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoClassificationAtoms()
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumAssetInfoClassificationAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoKeyWordAtoms()
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumAssetInfoKeyWordAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoLocationAtoms()
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumAssetInfoLocationAtoms());
            }
            else
            {
                return 0;
            }
        }

        int32 getNumAssetInfoAlbumAtoms()
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumAssetInfoAlbumAtoms());
            }
            else
            {
                return 0;
            }
        }

        int32 getNumAssetInfoRecordingYearAtoms()
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumAssetInfoRecordingYearAtoms());
            }
            else
            {
                return 0;
            }
        }
        PvmfAssetInfo3GPPLocationStruct *getAssetInfoLocationStruct(int32 index = 0) const
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getAssetInfoLocationStruct(index));
            }
            else
            {
                return NULL;
            }
        }
        uint16 getAssetInfoAlbumLangCode(int32 index)
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getAssetInfoAlbumLangCode(index));
            }
            else
            {
                return 0;
            }
        }
        OSCL_wString& getAssetInfoAlbumNotice(MP4FFParserOriginalCharEnc &charType, int32 index)
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getAssetInfoAlbumNotice(charType, index));
            }
            else
            {
                return _emptyString;
            }
        }
        uint8 getAssetInfoAlbumTrackNumber(int32 index)
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getAssetInfoAlbumTrackNumber(index));
            }
            else
            {
                return 0;
            }
        }
        uint16 getAssetInfoRecordingYear(int32 index)
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getAssetInfoRecordingYear(index));
            }
            else
            {
                return 0;
            }
        }

        int32 getNumAMRFramesPerSample(uint32 trackID)
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumAMRFramesPerSample(trackID));
            }
            else
            {
                return 0;
            }
        }

        uint8 parseBufferAndGetNumAMRFrames(uint8* buffer, uint32 size);


        MP4_ERROR_CODE getMaxTrackTimeStamp(uint32 trackID,
                                            uint32 fileSize,
                                            uint32& timeStamp)
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getMaxTrackTimeStamp(trackID,
                        fileSize,
                        timeStamp));
            }
            else
            {
                return DEFAULT_ERROR;
            }
        }

        MP4_ERROR_CODE getSampleNumberClosestToTimeStamp(uint32 trackID,
                uint32 &sampleNumber,
                uint32 timeStamp,
                uint32 sampleOffset = 0)
        {
            if (_pmovieAtom != NULL)
            {
                return
                    (_pmovieAtom->getSampleNumberClosestToTimeStamp(trackID,
                            sampleNumber,
                            timeStamp,
                            sampleOffset));
            }
            else
            {
                return (READ_FAILED);
            }
        }

        AVCSampleEntry* getAVCSampleEntry(uint32 trackID, uint32 index)
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getAVCSampleEntry(trackID, index));
            }
            return (NULL);
        }

        uint32 getAVCNALLengthSize(uint32 trackID, uint32 index)
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getAVCNALLengthSize(trackID, index));
            }
            return 0;
        }

        uint32 getNumAVCSampleEntries(uint32 trackID)
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getNumAVCSampleEntries(trackID));
            }
            return 0;
        }

        void ResetVideoTrackPresentFlag(void)
        {
            _oVideoTrackPresent = false;
        }

        uint32 getTrackLevelOMA2DRMInfoSize(uint32 trackID);
        uint8* getTrackLevelOMA2DRMInfo(uint32 trackID);

        MP4_ERROR_CODE RequestReadCapacityNotification(PvmiDataStreamObserver& aObserver,
                uint32 aFileOffset,
                OsclAny* aContextData = NULL);

        MP4_ERROR_CODE CancelNotificationSync();

        MP4_ERROR_CODE GetCurrentFileSize(uint32& aFileSize);

        int32 getTrackTSStartOffset(uint32& aTSOffset, uint32 aTrackID)
        {
            if (_pmovieAtom != NULL)
            {
                return (_pmovieAtom->getTrackTSStartOffset(aTSOffset, aTrackID));
            }
            else
            {
                return READ_FAILED;
            }
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesArtist() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));
            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getITunesArtist();

            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesAlbumArtist() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));
            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getITunesAlbumArtist();

            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesTitle() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));
            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getITunesTitle();

            }
            else
                return temp;

        }

        OSCL_wHeapString<OsclMemAllocator> getITunesTrackSubTitle() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));
            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getITunesTrackSubTitle();

            }
            else
                return temp;

        }


        OSCL_wHeapString<OsclMemAllocator> getITunesAlbum() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getITunesAlbum();
            }
            else
                return temp;
        }

        // Gnre ** Starts **
        uint16 getITunesGnreID() const
        {
            if (_pmovieAtom != NULL)
            {

                if (_pmovieAtom->getITunesGnreVersion() == INTEGER_GENRE)
                {
                    return _pmovieAtom->getITunesGnreID();
                }
                else
                    return 0;
            }
            else
                return 0;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesGnreString() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {

                if (_pmovieAtom->getITunesGnreVersion() == STRING_GENRE)
                {
                    return _pmovieAtom->getITunesGnreString();
                }
                else
                    return temp;
            }
            else
                return temp;
        }

        //This function will tell the type of Genre--
        GnreVersion getITunesGnreVersion() const
        {
            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getITunesGnreVersion();
            }
            else
                return INTEGER_GENRE;

        }
        // Gnre ** Ends **


        // Returns the 4-byte YEAR when the song was recorded
        OSCL_wHeapString<OsclMemAllocator> getITunesYear() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->getITunesYear();
            }
            else
                return temp;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesTool() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->getITunesTool();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesEncodedBy() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->getITunesEncodedBy();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesWriter() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->getITunesWriter();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesGroupData() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->getITunesGroup();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesComment() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->getITunesComment();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCopyright() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->getITunesCopyright();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesDescription() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->getITunesDescription();
            }
            else
                return temp;
        }

        uint16 getITunesThisTrackNo() const
        {
            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->getITunesThisTrackNo();
            }
            else
                return 0;
        }

        uint16 getITunesTotalTracks() const
        {
            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->getITunesTotalTracks();
            }
            else
                return 0;
        }

        bool IsITunesCompilationPart() const
        {
            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->IsITunesCompilationPart();
            }
            else
                return false;
        }

        bool IsITunesContentRating() const
        {
            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->IsITunesContentRating();
            }
            else
                return false;
        }

        uint16 getITunesBeatsPerMinute() const
        {
            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getITunesBeatsPerMinute();

            }
            else
                return 0;
        }


        PvmfApicStruct* getITunesImageData() const
        {
            if (_pmovieAtom)
                return _pmovieAtom ->getITunesImageData();
            else
                return NULL;
        }

        uint16 getITunesThisDiskNo() const
        {
            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->getITunesThisDiskNo();

            }
            else
                return 0;
        }

        uint16 getITunesTotalDisks() const
        {
            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->getITunesTotalDisks();
            }
            else
                return 0;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesNormalizationData() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getITunesNormalizationData();

            }
            else
                return temp;

        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCDIdentifierData(uint8 index)
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getITunesCDIdentifierData(index);

            }
            else
                return temp;
        }


        uint8 getITunesTotalCDIdentifierData() const
        {

            if (_pmovieAtom != NULL)
                return _pmovieAtom->getITunesTotalCDIdentifierData();
            else
                return 0;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesCDTrackNumberData() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getITunesCDTrackNumberData();

            }
            else
                return temp;

        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCDDB1Data() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {
                return _pmovieAtom->getITunesCDDB1Data();

            }
            else
                return temp;

        }

        OSCL_wHeapString<OsclMemAllocator> getITunesLyrics() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp(_STRLIT(""));

            if (_pmovieAtom != NULL)
            {

                return _pmovieAtom->getITunesLyrics();
            }
            else
                return temp;
        }

        // For DRM Atom.
        bool IsTFRAPresentForTrack(uint32 TrackId, bool oVideoAudioTextTrack);

        /*
        This function has been modified to check the entry count in TFRA for all tracks are equal.
        The code change is under macro DISABLE_REPOS_ON_CLIPS_HAVING_UNEQUAL_TFRA_ENTRY_COUNT
        */
        bool IsTFRAPresentForAllTrack(uint32 numTracks, uint32 *trackList);


        bool IsMovieFragmentsPresent() const
        {
            return  _isMovieFragmentsPresent;
        }
        uint32 GetByteOffsetToStartOfAudioFrames();
        void GetID3MetaData(PvmiKvpSharedPtrVector &id3Frames);
        bool IsID3Frame(const OSCL_String& frameType);
        void GetID3Frame(const OSCL_String& aFrameType, PvmiKvpSharedPtrVector& aFrame);
        PVID3Version GetID3Version() const;
        void parseID3Header(MP4_FF_FILE * aFile);

        uint32 getContentType();
        bool CreateDataStreamSessionForExternalDownload(OSCL_wString& aFilename,
                PVMFCPMPluginAccessInterfaceFactory* aCPMAccessFactory,
                OsclFileHandle* aHandle,
                Oscl_FileServer* aFileServSession);
        void DestroyDataStreamForExternalDownload();


    private:
        void ReserveMemoryForLangCodeVector(Oscl_Vector<uint16, OsclMemAllocator> &iLangCode, int32 capacity, int32 &leavecode);
        void ReserveMemoryForValuesVector(Oscl_Vector<OSCL_wHeapString<OsclMemAllocator>, OsclMemAllocator> &iValues, int32 capacity, int32 &leavecode);
        OSCL_wHeapString<OsclMemAllocator> _emptyString;
        UserDataAtom *_puserDataAtom;
        FileTypeAtom *_pFileTypeAtom;
        MovieAtom    *_pmovieAtom;
        MovieFragmentAtom *_pMovieFragmentAtom;
        PVID3ParCom* _pID3Parser;

        Oscl_Vector<TrackAtom*, OsclMemAllocator> *_pTrackAtomVec;

        int32 _scalability;
        int32 _fileType;

        // From DecoderConfigDescriptor
        DecoderSpecificInfo *getTrackDecoderSpecificInfo(uint32 id);

        bool _oPVContent;
        bool _oPVContentDownloadable;

        MP4_FF_FILE *_commonFilePtr;

        bool _isMovieFragmentsPresent;
        uint32 _pointerMovieAtomEnd;
        MP4_FF_FILE *_movieFragmentFilePtr;
        Oscl_Vector<MovieFragmentAtom*, OsclMemAllocator> *_pMovieFragmentAtomVec;
        Oscl_Vector<MovieFragmentRandomAccessAtom*, OsclMemAllocator> *_pMovieFragmentRandomAccessAtomVec;
        MfraOffsetAtom *_pMfraOffsetAtom;
        uint32 _ptrMoofEnds;
        uint32 _parsing_mode;
        uint32 _movieFragmentIdx[256];
        uint32 _peekMovieFragmentIdx[256];
        TrackDurationContainer *_pTrackDurationContainer;
        Oscl_Vector<TrackExtendsAtom*, OsclMemAllocator> *_pTrackExtendsAtomVec;
        Oscl_Vector<uint32, OsclMemAllocator> *_pMoofOffsetVec;
        void populateTrackDurationVec();
        MP4_FF_FILE tempfptr;

        PVLogger *iLogger, *iStateVarLogger, *iParsedDataLogger;
        bool oMfraFound;
        bool parseMoofCompletely;
        bool moofParsingCompleted;
        uint32 moofType;
        uint32 moofSize;
        uint32 moofCount;
        uint32 moofPtrPos;
        uint32 currMoofNum;
        bool _oVideoTrackPresent;
        uint32 _movieFragmentSeqIdx[256];
        uint32 _peekMovieFragmentSeqIdx[256];
        bool isResetPlayBackCalled;
        uint32 countOfTrunsParsed;

        //Master Title List
        Oscl_Vector<OSCL_wHeapString<OsclMemAllocator>, OsclMemAllocator> titleValues;
        Oscl_Vector<uint16, OsclMemAllocator> iTitleLangCode;
        Oscl_Vector<MP4FFParserOriginalCharEnc, OsclMemAllocator> iTitleCharType;

        //Master Author List
        Oscl_Vector<OSCL_wHeapString<OsclMemAllocator>, OsclMemAllocator> authorValues;
        Oscl_Vector<uint16, OsclMemAllocator> iAuthorLangCode;
        Oscl_Vector<MP4FFParserOriginalCharEnc, OsclMemAllocator> iAuthorCharType;

        //Master Album List
        Oscl_Vector<OSCL_wHeapString<OsclMemAllocator>, OsclMemAllocator> albumValues;
        Oscl_Vector<uint16, OsclMemAllocator> iAlbumLangCode;
        Oscl_Vector<MP4FFParserOriginalCharEnc, OsclMemAllocator> iAlbumCharType;

        //Master Artist List
        Oscl_Vector<OSCL_wHeapString<OsclMemAllocator>, OsclMemAllocator> artistValues;
        Oscl_Vector<uint16, OsclMemAllocator> iArtistLangCode;
        Oscl_Vector<MP4FFParserOriginalCharEnc, OsclMemAllocator> iArtistCharType;

        //Master Genre List
        Oscl_Vector<OSCL_wHeapString<OsclMemAllocator>, OsclMemAllocator> genreValues;
        Oscl_Vector<uint16, OsclMemAllocator> iGenreLangCode;
        Oscl_Vector<MP4FFParserOriginalCharEnc, OsclMemAllocator> iGenreCharType;

        //Master Year List
        Oscl_Vector<uint32, OsclMemAllocator> yearValues;

        //Master Copyright List
        Oscl_Vector<OSCL_wHeapString<OsclMemAllocator>, OsclMemAllocator> copyrightValues;
        Oscl_Vector<uint16, OsclMemAllocator> iCopyrightLangCode;
        Oscl_Vector<MP4FFParserOriginalCharEnc, OsclMemAllocator> iCopyrightCharType;

        //Master Comment List
        Oscl_Vector<OSCL_wHeapString<OsclMemAllocator>, OsclMemAllocator> commentValues;
        Oscl_Vector<uint16, OsclMemAllocator> iCommentLangCode;
        Oscl_Vector<MP4FFParserOriginalCharEnc, OsclMemAllocator> iCommentCharType;

        //Master Description List
        Oscl_Vector<OSCL_wHeapString<OsclMemAllocator>, OsclMemAllocator> descriptionValues;
        Oscl_Vector<uint16, OsclMemAllocator> iDescriptionLangCode;
        Oscl_Vector<MP4FFParserOriginalCharEnc, OsclMemAllocator> iDescriptionCharType;

        //Master Rating List
        Oscl_Vector<OSCL_wHeapString<OsclMemAllocator>, OsclMemAllocator> ratingValues;
        Oscl_Vector<uint16, OsclMemAllocator> iRatingLangCode;
        Oscl_Vector<MP4FFParserOriginalCharEnc, OsclMemAllocator> iRatingCharType;

        oscl_wchar _id3v1Title[ID3V1_STR_MAX_SIZE];
        oscl_wchar _id3v1Album[ID3V1_STR_MAX_SIZE];
        oscl_wchar _id3v1Artist[ID3V1_STR_MAX_SIZE];
        oscl_wchar _id3v1Comment[ID3V1_STR_MAX_SIZE];
        uint32 _id3v1Year;
        uint32 _fileSize;

};

#endif // MPEG4FILE_H_INCLUDED



