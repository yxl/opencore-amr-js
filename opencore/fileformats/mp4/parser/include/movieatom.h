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


#ifndef MOVIEATOM_H_INCLUDED
#define MOVIEATOM_H_INCLUDED

#define PV_ERROR -1

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef MOVIEHEADERATOM_H_INCLUDED
#include "movieheaderatom.h"
#endif

#ifndef OBJECTDESCRIPTORATOM_H_INCLUDED
#include "objectdescriptoratom.h"
#endif

#ifndef TRACKATOM_H_INCLUDED
#include "trackatom.h"
#endif

#ifndef DECODERSPECIFICINFO_H_INCLUDED
#include "decoderspecificinfo.h"
#endif


#ifndef SAMPLEENTRY_H_INCLUDED
#include "sampleentry.h"
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


#ifndef MOVIEEXTENDSATOM_HINCLUDED
#include "movieextendsatom.h"
#endif


class AVCSampleEntry;
/*
Class Movie Atom
*/
class MovieAtom : public Atom
{

    public:

        OSCL_IMPORT_REF MovieAtom(MP4_FF_FILE *fp,
                                  OSCL_wString& filename,
                                  uint32 size,
                                  uint32 type,
                                  bool oPVContent = false,
                                  bool oPVContentDownloadable = false,
                                  uint32 parsingMode = 0);

        OSCL_IMPORT_REF virtual ~MovieAtom();


        int32 updateFileSize(uint32	filesize);

        int32 getNextMediaSample(uint32 id, uint8 *buf, uint32 &size, uint32 &index, uint32 &SampleOffset);
        int32 getMediaSample(uint32 sampleNumber, uint32 id, uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset);

        MP4_ERROR_CODE getKeyMediaSampleNumAt(uint32 aTrackId,
                                              uint32 aKeySampleNum,
                                              GAU    *pgau);

        int32 getPrevKeyMediaSample(uint32 inputtimestamp,
                                    uint32 &aKeySampleNum,
                                    uint32 id,
                                    uint32 *n,
                                    GAU    *pgau);
        int32 getNextKeyMediaSample(uint32 inputtimestamp,
                                    uint32 &aKeySampleNum,
                                    uint32 id,
                                    uint32 *n,
                                    GAU    *pgau);


        uint32 getTimestampForCurrentSample(uint32 id);
        int32 getOffsetByTime(uint32 id, uint32 ts, int32* sampleFileOffset);

        uint32 getNumKeyFrames(uint32 trackid)
        {
            TrackAtom *track = getTrackForID(trackid);

            if (track != NULL)
            {
                return track->getNumKeyFrames();
            }
            else
            {
                return 0;
            }
        }

        uint64 getDuration()
        {
            if (_pmovieHeaderAtom != NULL)
            {
                return _pmovieHeaderAtom->getDuration();
            }
            else
                return 0;
        }// Get the duration of the movie

        OSCL_IMPORT_REF uint64 getMovieFragmentDuration();

        uint64 getCreationTime()
        {
            if (_pmovieHeaderAtom != NULL)
            {
                return _pmovieHeaderAtom->getCreationTime();
            }
            else
            {
                return 0;
            }
        }

        OSCL_wHeapString<OsclMemAllocator> getCreationDate()
        {
            if (_pmovieHeaderAtom != NULL)
            {
                return _pmovieHeaderAtom->getCreationDate();
            }
            else
            {
                return _emptyString;
            }
        }


        uint64 getModificationTime()
        {
            if (_pmovieHeaderAtom != NULL)
            {
                return _pmovieHeaderAtom->getModificationTime();
            }
            else
            {
                return 0;
            }
        }

        uint32 getTimeScale() const;    // Gets the timescale of the mp4 movie

        OSCL_IMPORT_REF TrackAtom *getTrackForID(uint32 id);
        int32 getScalability() const
        {
            return _scalability;
        }
        OSCL_IMPORT_REF int32 getFileType() const
        {
            return _fileType;
        }

        int32 getTimestampForRandomAccessPoints(uint32 id, uint32 *num, uint32 *tsBuf, uint32* numBuf, uint32* offsetBuf);

        int32 getTimestampForRandomAccessPointsBeforeAfter(uint32 id, uint32 ts, uint32 *tsBuf, uint32* numBuf,
                uint32& numsamplestoget,
                uint32 howManyKeySamples);
        int32 getNumTracks()
        {
            return _ptrackArray->size();
        }

        OSCL_IMPORT_REF uint32 getTrackIDList(uint32 *ids, int size) ;
        uint32 getTrackWholeIDList(uint32 *ids);

        // From TrackHeader
        uint64 getTrackDuration(uint32 id);

        // From TrackReference
        int32 trackDependsOn(uint32 id);

        // From MediaHeader
        uint64 getTrackMediaDuration(uint32 id);

        int32 getTrackMediaTimescale(uint32 id);

        // From Handler
        int32 getTrackStreamType(uint32 id);

        // From SampleDescription
        OSCL_IMPORT_REF int32 getTrackNumSampleEntries(uint32 id);

        // From DecoderConfigDescriptor
        DecoderSpecificInfo *getTrackDecoderSpecificInfo(uint32 id);

        DecoderSpecificInfo *getTrackDecoderSpecificInfoAtSDI(uint32 trackID, uint32 index);

        void getTrackMIMEType(uint32 id, OSCL_String& aMimeType);

        int32 getTrackMaxBufferSizeDB(uint32 id);

        int32  getTrackAverageBitrate(uint32 id);

        OSCL_IMPORT_REF int32 getTrackHeight(uint32 id);
        OSCL_IMPORT_REF int32 getTrackWidth(uint32 id);

        TrackAtom *getTrackforID(uint32 id);

        void resetPlayback();
        void resetTrackToEOT();

        uint32 resetPlayback(uint32 time, uint16 numTracks, uint32 *trackList, bool bResetToIFrame);

        int32 queryRepositionTime(uint32 time,
                                  uint16 numTracks,
                                  uint32 *trackList,
                                  bool bResetToIFrame,
                                  bool bBeforeRequestedTime);

        int32 querySyncFrameBeforeTime(uint32 time, uint16 numTracks, uint32 *trackList);

        int32 getNextBundledAccessUnits(const uint32 trackID,
                                        uint32 *n,
                                        GAU    *pgau);

        int32 peekNextBundledAccessUnits(const uint32 trackID,
                                         uint32 *n,
                                         MediaMetaInfo *mInfo);

        OSCL_IMPORT_REF uint32 getSampleCountInTrack(uint32 id);

        bool checkMMP4();
        int32 getNumAssetInfoTitleAtoms()
        {
            if (_pUserDataAtom != NULL)
            {
                return (_pUserDataAtom->getNumAssetInfoTitleAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoDescAtoms()
        {
            if (_pUserDataAtom != NULL)
            {
                return (_pUserDataAtom->getNumAssetInfoDescAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumCopyRightAtoms()
        {
            if (_pUserDataAtom != NULL)
            {
                return (_pUserDataAtom->getNumCopyRightAtoms());
            }
            else
            {
                return 0;
            }
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesDescription() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesDescription();
            else
                return temp;
        }

        int32 getNumAssetInfoPerformerAtoms()
        {
            if (_pUserDataAtom != NULL)
            {
                return (_pUserDataAtom->getNumAssetInfoPerformerAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoAuthorAtoms()
        {
            if (_pUserDataAtom != NULL)
            {
                return (_pUserDataAtom->getNumAssetInfoAuthorAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoGenreAtoms()
        {
            if (_pUserDataAtom != NULL)
            {
                return (_pUserDataAtom->getNumAssetInfoGenreAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoRatingAtoms()
        {
            if (_pUserDataAtom != NULL)
            {
                return (_pUserDataAtom->getNumAssetInfoRatingAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoClassificationAtoms()
        {
            if (_pUserDataAtom != NULL)
            {
                return (_pUserDataAtom->getNumAssetInfoClassificationAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoKeyWordAtoms()
        {
            if (_pUserDataAtom != NULL)
            {
                return (_pUserDataAtom->getNumAssetInfoKeyWordAtoms());
            }
            else
            {
                return 0;
            }
        }
        int32 getNumAssetInfoLocationAtoms()
        {
            if (_pUserDataAtom != NULL)
            {
                return (_pUserDataAtom->getNumAssetInfoLocationAtoms());
            }
            else
            {
                return 0;
            }
        }

        int32 getNumAssetInfoAlbumAtoms()
        {
            if (_pUserDataAtom != NULL)
            {
                return (_pUserDataAtom->getNumAssetInfoAlbumAtoms());
            }
            else
            {
                return 0;
            }
        }

        int32 getNumAssetInfoRecordingYearAtoms()
        {
            if (_pUserDataAtom != NULL)
            {
                return (_pUserDataAtom->getNumAssetInfoRecordingYearAtoms());
            }
            else
            {
                return 0;
            }
        }

        uint16 getAssetInfoTitleLangCode(int32 index) ;
        OSCL_wString& getAssetInfoTitleNotice(MP4FFParserOriginalCharEnc &charType, int32 index) ;
        uint16 getAssetInfoDescLangCode(int32 index) ;
        OSCL_wString& getAssetInfoDescNotice(MP4FFParserOriginalCharEnc &charType, int32 index) ;
        OSCL_wString& getCopyRightString(MP4FFParserOriginalCharEnc &charType, int32 index);
        uint16 getCopyRightLanguageCode(int32 index);
        uint16 getAssetInfoPerformerLangCode(int32 index) ;
        OSCL_wString& getAssetInfoPerformerNotice(MP4FFParserOriginalCharEnc &charType, int32 index) ;
        uint16 getAssetInfoAuthorLangCode(int32 index) ;
        OSCL_wString& getAssetInfoAuthorNotice(MP4FFParserOriginalCharEnc &charType, int32 index) ;
        uint16 getAssetInfoGenreLangCode(int32 index) ;
        OSCL_wString& getAssetInfoGenreNotice(MP4FFParserOriginalCharEnc &charType, int32 index) ;
        uint32 getAssetInfoRatingCriteria(int32 index) ;
        uint32 getAssetInfoRatingEntity(int32 index) ;
        uint16 getAssetInfoRatingLangCode(int32 index) ;
        OSCL_wString& getAssetInfoRatingNotice(MP4FFParserOriginalCharEnc &charType, int32 index) ;
        uint32 getAssetInfoClassificationEntity(int32 index) ;
        uint16 getAssetInfoClassificationTable(int32 index) ;
        uint16 getAssetInfoClassificationLangCode(int32 index) ;
        OSCL_wString& getAssetInfoClassificationNotice(MP4FFParserOriginalCharEnc &charType, int32 index) ;
        uint16 getAssetInfoNumKeyWords(int32 index) ;
        uint16 getAssetInfoKeyWordLangCode(int32 index) ;
        OSCL_wString& getAssetInfoKeyWord(int32 atomIndex, int32 keyWordIndex) ;

        PvmfAssetInfo3GPPLocationStruct *getAssetInfoLocationStruct(int32 index) const;

        uint16 getAssetInfoAlbumLangCode(int32 index);
        OSCL_wString& getAssetInfoAlbumNotice(MP4FFParserOriginalCharEnc &charType, int32 index);
        uint8 getAssetInfoAlbumTrackNumber(int32 index);

        uint16 getAssetInfoRecordingYear(int32 index);

        int16 getLayer(uint32 id);
        uint16 getAlternateGroup(uint32 id);
        int32 getTextTrackWidth(uint32 id);
        int32 getTextTrackHeight(uint32 id);
        int32 getTextTrackXOffset(uint32 id);
        int32 getTextTrackYOffset(uint32 id);

        SampleEntry *getTextSampleEntryAt(uint32 id, uint32 index);

        int32 getNumAMRFramesPerSample(uint32 trackID);


        MP4_ERROR_CODE getMaxTrackTimeStamp(uint32 trackID,
                                            uint32 fileSize,
                                            uint32& timeStamp);

        MP4_ERROR_CODE getSampleNumberClosestToTimeStamp(uint32 trackID,
                uint32 &sampleNumber,
                uint32 timeStamp,
                uint32 sampleOffset = 0);


        AVCSampleEntry* getAVCSampleEntry(uint32 trackID, uint32 index);

        uint32 getAVCNALLengthSize(uint32 trackID, uint32 index);

        uint32 getNumAVCSampleEntries(uint32 trackID);

        int32 getTrackTSStartOffset(uint32& aTSOffset, uint32 aTrackID);


        OSCL_EXPORT_REF bool isMultipleSampleDescriptionAvailable(uint32 trackID);

        OSCL_EXPORT_REF bool IsMovieFragmentPresent()
        {
            return _isMovieFragmentPresent;
        }

        Oscl_Vector<TrackExtendsAtom*, OsclMemAllocator> *getTrackExtendsAtomVec()
        {
            if (_pMovieExtendsAtom != NULL)
                return _pMovieExtendsAtom->getTrackExtendsAtomVec();

            return NULL;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesTitle() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesTitle();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesTrackSubTitle() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesTrackSubTitle();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesArtist() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesArtist();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesAlbumArtist() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesAlbumArtist();
            else
                return temp;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesAlbum() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesAlbum();
            else
                return temp;
        }

        // Gnre ** Starts **
        uint16 getITunesGnreID() const
        {
            if (_pUserDataAtom)
            {
                if (_pUserDataAtom->getITunesGnreVersion() == INTEGER_GENRE)
                {
                    return _pUserDataAtom->getITunesGnreID();
                }
                else
                    return 0;
            }
            else
                return 0;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesGnreString() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
            {
                if (_pUserDataAtom->getITunesGnreVersion() == STRING_GENRE)
                {
                    return _pUserDataAtom->getITunesGnreString();
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
            if (_pUserDataAtom)
            {
                return _pUserDataAtom->getITunesGnreVersion();
            }
            else
                // By-default return INTEGER_GENRE
                return INTEGER_GENRE;
        }
        // Gnre ** Ends **


        // Returns the 4-byte YEAR when the song was recorded
        OSCL_wHeapString<OsclMemAllocator> getITunesYear() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesYear();
            else
                return temp;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesTool() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
            {
                return _pUserDataAtom->getITunesTool();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesEncodedBy() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
            {
                return _pUserDataAtom->getITunesEncodedBy();
            }
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesWriter() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesWriter();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesGroup() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesGroup();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesComment() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesComment();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCopyright() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesCopyright();
            else
                return temp;
        }


        uint16 getITunesThisTrackNo() const
        {
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesThisTrackNo();
            else
                return 0;
        }

        uint16 getITunesTotalTracks() const
        {
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesTotalTracks();
            else
                return 0;
        }

        bool IsITunesCompilationPart() const
        {
            if (_pUserDataAtom)
                return _pUserDataAtom->IsITunesCompilationPart();
            else
                return false;
        }

        bool IsITunesContentRating() const
        {
            if (_pUserDataAtom)
                return _pUserDataAtom->IsITunesContentRating();
            else
                return false;
        }

        uint16 getITunesBeatsPerMinute() const
        {
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesBeatsPerMinute();
            else
                return 0;
        }


        PvmfApicStruct* getITunesImageData() const
        {
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesImageData();
            else
                return NULL;
        }

        uint16 getITunesThisDiskNo() const
        {
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesThisDiskNo();
            else
                return 0;
        }

        uint16 getITunesTotalDisks() const
        {
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesTotalDisks();
            else
                return 0;
        }



        OSCL_wHeapString<OsclMemAllocator> getITunesNormalizationData() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesNormalizationData();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCDIdentifierData(uint8 index) const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesCDIdentifierData(index);
            else
                return temp;
        }


        uint8 getITunesTotalCDIdentifierData() const
        {

            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesTotalCDIdentifierData();
            else
                return 0;
        }


        OSCL_wHeapString<OsclMemAllocator> getITunesCDTrackNumberData() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesCDTrackNumberData();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesCDDB1Data() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesCDDB1Data();
            else
                return temp;
        }

        OSCL_wHeapString<OsclMemAllocator> getITunesLyrics() const
        {
            OSCL_wHeapString<OsclMemAllocator> temp;
            if (_pUserDataAtom)
                return _pUserDataAtom->getITunesLyrics();
            else
                return temp;
        }

    private:
        void addTrackAtom(TrackAtom *a);

        MovieHeaderAtom       *_pmovieHeaderAtom;
        ObjectDescriptorAtom  *_pobjectDescriptorAtom;
        UserDataAtom		  *_pUserDataAtom;


        MovieExtendsAtom	  *_pMovieExtendsAtom;

        int32 _scalability;
        int32 _fileType;

        Oscl_Vector<TrackAtom*, OsclMemAllocator> *_ptrackArray;

        bool _isMovieFragmentPresent;
        bool _oVideoTrackPresent;

        OSCL_wHeapString<OsclMemAllocator> _emptyString;


};

#endif // MOVIEATOM_H_INCLUDED


