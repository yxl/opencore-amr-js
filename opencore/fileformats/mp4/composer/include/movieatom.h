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
    This PVA_FF_MovieAtom Class is the main atom class in the MPEG-4 File that stores
    all the meta data about the MPEG-4 presentation.
*/


#ifndef __MovieAtom_H__
#define __MovieAtom_H__

#define PV_ERROR -1

#include "atom.h"
#include "a_isucceedfail.h"

#include "pv_mp4ffcomposer_config.h"

#include "movieheaderatom.h"
#include "trackatom.h"

#include "userdataatom.h"


#include "assetinfoatoms.h"

// movie fragment
#include "movieextendsatom.h"
#include "moviefragmentatom.h"
#include "moviefragmentrandomaccessatom.h"
#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif


class PVA_FF_MovieAtom : public PVA_FF_Atom, public PVA_FF_ISucceedFail
{

    public:
        PVA_FF_MovieAtom(uint32 fileAuthoringFlags); // Constructor

        virtual ~PVA_FF_MovieAtom();

        // Get the duration of the movie
        uint32 getDuration()
        {
            return _pmovieHeaderAtom->getDuration();
        }

        int32 getScalability() const
        {
            return _scalability;
        }
        int32 getFileType() const
        {
            return _fileType;
        }

        // Member gets and sets
        const PVA_FF_MovieHeaderAtom &getMovieHeaderAtom()
        {
            return *_pmovieHeaderAtom;
        }
        PVA_FF_MovieHeaderAtom &getMutableMovieHeaderAtom()
        {
            return *_pmovieHeaderAtom;
        }
        void setMovieHeaderAtom(PVA_FF_MovieHeaderAtom *header)
        {
            _pmovieHeaderAtom = header;
        }

        // Track gets and adds
        void addTrackAtom(PVA_FF_TrackAtom *a); // Adds to appropriate vector

        const Oscl_Vector<PVA_FF_TrackAtom*, OsclMemAllocator>& getMpeg4TrackVec()
        {
            return *_pmpeg4TrackVec;
        }
        PVA_FF_TrackAtom *getMpeg4Track(int32 index);

        // Return a pointer to a track with ES_ID value of esid
        //PVA_FF_TrackAtom *getTrackWithESID(uint32 esid) const;

        // Sets the timescale of the mp4 movie - gets reflected in the movieHeader
        // and is used for the duration computation in the track headers
        void setTimeScale(uint32 ts);

        // Gets the timescale of the mp4 movie
        uint32 getTimeScale() const;

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        // Getting and setting the Mpeg4 VOL header
        void addDecoderSpecificInfo(PVA_FF_DecoderSpecificInfo *pinfo, int32 trackID);

        // Getting and setting the Mpeg4 VOL header for timed text
        void addTextDecoderSpecificInfo(PVA_FF_TextSampleDescInfo *pinfo, int32 trackID);

        PVA_FF_TrackAtom *getMediaTrack(uint32 trackID);
        void addSampleToTrack(uint32 trackID, uint8 *psample,
                              uint32 size, uint32 ts, uint8 flags,
                              uint32 baseOffset = 0, bool _oChunkStart = false);

        void addTextSampleToTrack(uint32 trackID, uint8 *psample,
                                  uint32 size, uint32 ts, uint8 flags, int32 index,
                                  uint32 baseOffset = 0, bool _oChunkStart = false);

        void addSampleToTrack(uint32 trackID,
                              Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                              uint32 size, uint32 ts, uint8 flags,
                              uint32 baseOffset = 0, bool _oChunkStart = false);
        void addTextSampleToTrack(uint32 trackID,
                                  Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                                  uint32 size, uint32 ts, uint8 flags, int32 index,
                                  uint32 baseOffset = 0, bool _oChunkStart = false);

        bool reAuthorFirstSampleInTrack(uint32 trackID,
                                        uint32 size,
                                        uint32 baseOffset);

        void setTargetBitRate(uint32 trackID, uint32 bitrate);
        void setTimeScale(uint32 trackID, uint32 rate);
        int32 getCodecType(uint32 trackID);

        void setMaxBufferSizeDB(uint32 trackID, uint32 max);

        void prepareToRender();

        void populateCommonMetadataAtoms();


        void createAssetInfoAtoms();


        void setLanguage(uint32 language)
        {
            if (_pMediaTrackVec != NULL)
            {
                for (uint32 i = 0; i < _pMediaTrackVec->size(); i++)
                {
                    (*_pMediaTrackVec)[i]->setLanguage(language);
                }
            }
        }

        bool setCopyRightInfo(PVA_FF_UNICODE_STRING_PARAM cprt, uint16 langCode)
        {
            if (_pAssetInfoCopyRightAtom != NULL)
            {
                _pAssetInfoCopyRightAtom->setCopyRightLangCode(langCode);
                _pAssetInfoCopyRightAtom->setCopyRightString(cprt);
                setLanguage(langCode);
                return true;
            }

            return false;
        }

        bool setAuthorInfo(PVA_FF_UNICODE_STRING_PARAM author, uint16 langCode)
        {
            if (_pAssetInfoAuthorAtom != NULL)
            {
                _pAssetInfoAuthorAtom->setAuthorLangCode(langCode);
                _pAssetInfoAuthorAtom->setAuthorNotice(author);
                setLanguage(langCode);
                return true;
            }

            return false;
        }

        bool setTitleInfo(PVA_FF_UNICODE_STRING_PARAM title, uint16 langCode)
        {
            if (_pAssetInfoTitleAtom != NULL)
            {
                _pAssetInfoTitleAtom->setTitleLangCode(langCode);
                _pAssetInfoTitleAtom->setTitleNotice(title);
                setLanguage(langCode);
                return true;
            }

            return false;
        }

        bool setDescription(PVA_FF_UNICODE_STRING_PARAM desc, uint16 langCode)
        {
            if (_pAssetInfoDescAtom != NULL)
            {
                _pAssetInfoDescAtom->setDescLangCode(langCode);
                _pAssetInfoDescAtom->setDescNotice(desc);
                setLanguage(langCode);
                return true;
            }

            return false;
        }


        bool setPerformerInfo(PVA_FF_UNICODE_STRING_PARAM performer, uint16 langCode)
        {
            if (_pAssetInfoPerformerAtom != NULL)
            {
                _pAssetInfoPerformerAtom->setPerfLangCode(langCode);
                _pAssetInfoPerformerAtom->setPerfNotice(performer);
                setLanguage(langCode);
                return true;
            }

            return false;
        }


        bool setGenreInfo(PVA_FF_UNICODE_STRING_PARAM genre, uint16 langCode)
        {
            if (_pAssetInfoGenreAtom != NULL)
            {
                _pAssetInfoGenreAtom->setGenreLangCode(langCode);
                _pAssetInfoGenreAtom->setGenreNotice(genre);
                setLanguage(langCode);
                return true;
            }

            return false;
        }

        bool setRatingInfo(PVA_FF_UNICODE_STRING_PARAM ratingInfo, uint32 ratingEntity,
                           uint32 ratingCriteria, uint16 langCode)
        {
            if (_pAssetInfoRatingAtom != NULL)
            {
                _pAssetInfoRatingAtom->setRatingLangCode(langCode);
                _pAssetInfoRatingAtom->setRatingEntity(ratingEntity);
                _pAssetInfoRatingAtom->setRatingCriteria(ratingCriteria);
                _pAssetInfoRatingAtom->setRatingInfoNotice(ratingInfo);
                setLanguage(langCode);
                return true;
            }

            return false;
        }


        bool setClassificationInfo(PVA_FF_UNICODE_STRING_PARAM classificationInfo,
                                   uint32 classificationEntity,
                                   uint16 classificationTable, uint16 langCode)
        {
            if (_pAssetInfoClassificationAtom != NULL)
            {
                _pAssetInfoClassificationAtom->setClassificationLangCode(langCode);
                _pAssetInfoClassificationAtom->setClassificationEntity(classificationEntity);
                _pAssetInfoClassificationAtom->setClassificationTable(classificationTable);
                _pAssetInfoClassificationAtom->setClassificationInfoNotice(classificationInfo);
                setLanguage(langCode);
                return true;
            }

            return false;
        }

        bool setKeyWordsInfo(uint8 keyWordSize, PVA_FF_UNICODE_HEAP_STRING keyWordInfo,
                             uint16 langCode)
        {
            if (_pAssetInfoKeyWordsAtom != NULL)
            {
                _pAssetInfoKeyWordsAtom->setKeyWordsLangCode(langCode);
                _pAssetInfoKeyWordsAtom->setKeyWord(keyWordSize, keyWordInfo);
                setLanguage(langCode);
                return true;
            }

            return false;
        }

        bool setLocationInfo(PvmfAssetInfo3GPPLocationStruct *ptr_loc_struct)
        {
            if (_pAssetInfoLocationInfoAtom != NULL)
            {
                _pAssetInfoLocationInfoAtom->setLocationInfoLangCode(ptr_loc_struct->_langCode);
                _pAssetInfoLocationInfoAtom->setLocationInfoRole(ptr_loc_struct->_role);
                _pAssetInfoLocationInfoAtom->setLocationInfoLongitude(ptr_loc_struct->_longitude);
                _pAssetInfoLocationInfoAtom->setLocationInfoLatitude(ptr_loc_struct->_latitude);
                _pAssetInfoLocationInfoAtom->setLocationInfoAltitude(ptr_loc_struct->_altitude);

                _pAssetInfoLocationInfoAtom->setLocationInfoName(ptr_loc_struct->_location_name);
                _pAssetInfoLocationInfoAtom->setLocationInfoAstrBody(ptr_loc_struct->_astronomical_body);
                _pAssetInfoLocationInfoAtom->setLocationInfoAddNotes(ptr_loc_struct->_additional_notes);
                setLanguage(ptr_loc_struct->_langCode);
                return true;
            }

            return false;
        }

        bool setAlbumInfo(PVA_FF_UNICODE_STRING_PARAM albumtitle, uint16 langCode)
        {
            if (_pAssetInfoKeyAlbumAtom != NULL)
            {
                _pAssetInfoKeyAlbumAtom->setAlbumLangCode(langCode);
                _pAssetInfoKeyAlbumAtom->setAlbumInfo(albumtitle);
                setLanguage(langCode);
                return true;
            }

            return false;
        }

        bool setRecordingYearInfo(uint16 recordingYear)
        {
            if (_pAssetInfoKeyRecordingYearAtom != NULL)
            {
                _pAssetInfoKeyRecordingYearAtom->setRecordingYear(recordingYear);
                return true;
            }

            return false;
        }

        // Movie Fragment : add movie extend atom usage APIs
        void	setMovieFragmentDuration();
        void	updateMovieFragmentDuration(uint32 trackID, uint32 ts);
        void	writeMovieFragmentDuration(MP4_AUTHOR_FF_FILE_IO_WRAP* fp);
        void    SetMaxSampleSize(uint32, uint32);
        void	writeMaxSampleSize(MP4_AUTHOR_FF_FILE_IO_WRAP*);
    private:
        virtual void recomputeSize();

        PVA_FF_MovieHeaderAtom *_pmovieHeaderAtom;

        Oscl_Vector<PVA_FF_TrackAtom*, OsclMemAllocator> *_pmpeg4TrackVec;

        PVA_FF_UserDataAtom *_puserDataAtom;
        Oscl_Vector<PVA_FF_TrackAtom*, OsclMemAllocator> *_pMediaTrackVec; // Vector of tracks
        int32 _scalability;
        int32 _fileType;


        PVA_FF_AssetInfoTitleAtom			*_pAssetInfoTitleAtom;
        PVA_FF_AssetInfoDescAtom			*_pAssetInfoDescAtom;
        PVA_FF_AssetInfoCopyRightAtom		*_pAssetInfoCopyRightAtom;
        PVA_FF_AssetInfoPerformerAtom		*_pAssetInfoPerformerAtom;
        PVA_FF_AssetInfoAuthorAtom			*_pAssetInfoAuthorAtom;
        PVA_FF_AssetInfoGenreAtom			*_pAssetInfoGenreAtom;
        PVA_FF_AssetInfoRatingAtom			*_pAssetInfoRatingAtom;
        PVA_FF_AssetInfoClassificationAtom	*_pAssetInfoClassificationAtom;
        PVA_FF_AssetInfoKeyWordsAtom		*_pAssetInfoKeyWordsAtom;
        PVA_FF_AssetInfoLocationInfoAtom	*_pAssetInfoLocationInfoAtom;
        PVA_FF_AssetInfoAlbumAtom			*_pAssetInfoKeyAlbumAtom;
        PVA_FF_AssetInfoRecordingYearAtom	*_pAssetInfoKeyRecordingYearAtom;

        // Movie Fragment : Atoms needed in movie fragment mode
        PVA_FF_MovieExtendsAtom				*_pMovieExtendsAtom;
        bool								_oMovieFragmentEnabled;
};



#endif

