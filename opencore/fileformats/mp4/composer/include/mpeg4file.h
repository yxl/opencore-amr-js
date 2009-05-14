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
    The PVA_FF_Mpeg4File Class is the class that will construct and maintain all the
    mecessary data structures to be able to render a valid MP4 file to disk.
    Format.
*/


#ifndef __Mpeg4File_H__
#define __Mpeg4File_H__

#include "pv_mp4ffcomposer_config.h"

#include "filetypeatom.h"
#include "a_impeg4file.h"
#include "userdataatom.h"

#include "movieatom.h"
#include "mediadataatom.h"
#include "a_atomdefs.h"
#include "a_isucceedfail.h"

#include "interleavebuffer.h"

// movie fragment
#include "moviefragmentatom.h"
#include "moviefragmentrandomaccessatom.h"
#include "assetinfoatoms.h"
//HEADER FILES REQD FOR MULTIPLE SAMPLE API
#include "oscl_media_data.h"
#include "pv_gau.h"
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#include "textsampledescinfo.h"

class PVA_FF_Mpeg4File : public PVA_FF_IMpeg4File, public PVA_FF_Parentable
{

    public:
        PVA_FF_Mpeg4File(int32 mediaType = FILE_TYPE_VIDEO); // Constructor

        virtual ~PVA_FF_Mpeg4File();

        // 03/21/01 Multiple instances support, commented out moved FROM private
        bool init(int32 mediaType,
                  void* osclFileServerSession = NULL,
                  uint32 fileAuthoringFlags = PVMP4FF_3GPP_DOWNLOAD_MODE);

        // Member get methods
        PVA_FF_MovieAtom &getMutableMovieAtom()
        {
            return *_pmovieAtom;
        }
        const PVA_FF_MovieAtom &getMovieAtom()
        {
            return *_pmovieAtom;
        }

        void addMediaDataAtom(PVA_FF_MediaDataAtom* atom);

        Oscl_Vector<PVA_FF_MediaDataAtom*, OsclMemAllocator>& getMediaDataAtomVec() const
        {
            return *_pmediaDataAtomVec;
        }

        uint32 getSize() const
        {
            return _size;
        }

        virtual void setVersion(PVA_FF_UNICODE_STRING_PARAM version,
                                uint16 langCode = LANGUAGE_CODE_UNKNOWN);
        virtual void setTitle(PVA_FF_UNICODE_STRING_PARAM title,
                              uint16 langCode = LANGUAGE_CODE_UNKNOWN);
        virtual void setAuthor(PVA_FF_UNICODE_STRING_PARAM author,
                               uint16 langCode = LANGUAGE_CODE_UNKNOWN);
        virtual void setCopyright(PVA_FF_UNICODE_STRING_PARAM copyright,
                                  uint16 langCode = LANGUAGE_CODE_UNKNOWN);
        virtual void setDescription(PVA_FF_UNICODE_STRING_PARAM description,
                                    uint16 langCode = LANGUAGE_CODE_UNKNOWN);

        //AssetInfo specific
        virtual void setPerformer(PVA_FF_UNICODE_STRING_PARAM performer,
                                  uint16 langCode = LANGUAGE_CODE_UNKNOWN);

        virtual void setGenre(PVA_FF_UNICODE_STRING_PARAM genre,
                              uint16 langCode = LANGUAGE_CODE_UNKNOWN);

        virtual void setRating(PVA_FF_UNICODE_STRING_PARAM ratingInfo,
                               uint16 langCode = LANGUAGE_CODE_UNKNOWN,
                               uint32 ratingEntity = RATING_ENTITY_UNKNOWN,
                               uint32 ratingCriteria = RATING_CRITERIA_UNKNOWN);

        virtual void setClassification(PVA_FF_UNICODE_STRING_PARAM classificationInfo,
                                       uint32 classificationEntity,
                                       uint16 classificationTable,
                                       uint16 langCode = LANGUAGE_CODE_UNKNOWN);

        virtual void setKeyWord(uint8 keyWordSize,
                                PVA_FF_UNICODE_HEAP_STRING keyWordInfo,
                                uint16 langCode = LANGUAGE_CODE_UNKNOWN);

        virtual void setLocationInfo(PvmfAssetInfo3GPPLocationStruct*);

        virtual void setAlbumInfo(PVA_FF_UNICODE_STRING_PARAM albumInfo,
                                  uint16 langCode = LANGUAGE_CODE_UNKNOWN);

        virtual void setRecordingYear(uint16 recordingYear);


        virtual void setCreationDate(PVA_FF_UNICODE_STRING_PARAM creationDate);

        // Methods to set the sample rate (i.e. timescales) for the streams and
        // the overall Mpeg-4 presentation
        virtual void setPresentationTimescale(uint32 timescale);

        virtual int32 getFileType() const
        {
            return _fileType;
        }

        // MPEG4 header retrieval methods
        virtual void setDecoderSpecificInfo(uint8 * header, int32 size, int32 trackID);

        // MPEG4 header retrieval methods for timed text
        virtual void setTextDecoderSpecificInfo(PVA_FF_TextSampleDescInfo *header, int32 trackID);

        // Render to file with filename
        virtual bool renderToFile(PVA_FF_UNICODE_STRING_PARAM filename);
        // Rendering the PVA_FF_Mpeg4File in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        // An access function to set the postfix string for PVA_FF_MediaDataAtom objects
        virtual void SetTempFilePostFix(PVA_FF_UNICODE_STRING_PARAM postFix);

        virtual void setMajorBrand(uint32 brand)
        {
            if (_pFileTypeAtom != NULL)
            {
                _pFileTypeAtom->setMajorBrand(brand);
            }
        }

        // 16.16 NOTATATION AND THE VERSION OF WMF SPEC is 1.1, hence the default
        // value 0x00010001
        virtual void setMajorBrandVersion(uint32 version = 0x10001)
        {
            if (_pFileTypeAtom != NULL)
            {
                _pFileTypeAtom->setMajorBrandVersion(version);
            }
        }
        virtual void addCompatibleBrand(uint32 brand)
        {
            if (_pFileTypeAtom != NULL)
            {
                _pFileTypeAtom->addCompatibleBrand(brand);
            }
        }

        virtual void setVideoParams(uint32 trackID, float frate, uint16 interval,
                                    uint32 frame_width, uint32 frame_height);


        virtual void setH263ProfileLevel(uint32 trackID,
                                         uint8 profile,
                                         uint8 level);

        uint32 addTrack(int32 mediaType,
                        int32 codecType,
                        bool oDirectRender = false,
                        uint8 profile = 1,
                        uint8 profileComp = 0xFF,
                        uint8 level = 0xFF);

        bool addSampleToTrack(uint32 trackID,
                              Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                              uint32 ts, uint8 flags);
        //special API for Timed text
        bool addTextSampleToTrack(uint32 trackID,
                                  Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                                  uint32 ts, uint8 flags, int32 index, uint8* Textsamplemodifier);

        void addTrackReference(uint32 currtrackID, int32 reftrackID);
        void setTargetBitRate(uint32 trackID, uint32 bitrate);
        void setTimeScale(uint32 trackID, uint32 rate);

        // An access function to set the output path string for PVA_FF_MediaDataAtom objects
        virtual void SetTempOutputPath(PVA_FF_UNICODE_STRING_PARAM outputPath);

        PVA_FF_MediaDataAtom* getMediaDataAtomForTrack(uint32 trackID);

        void setMaxBufferSizeDB(uint32 trackID, uint32 max)
        {
            _pmovieAtom->setMaxBufferSizeDB(trackID, max);
        }

        bool addMultipleAccessUnitsToTrack(uint32 trackID, GAU *pgau);

        bool renderTruncatedFile(PVA_FF_UNICODE_STRING_PARAM filename);

        uint32 convertCreationTime(PVA_FF_UNICODE_STRING_PARAM creationDate);

        bool checkInterLeaveDuration(uint32 trackID, uint32 ts);

        bool flushInterLeaveBuffer(uint32 trackID);

        bool getTargetFileSize(uint32 &metaDataSize, uint32 &mediaDataSize);

        bool prepareToEncode();

        void populateUserDataAtom();


        virtual bool setInterLeaveInterval(int32 interval)
        {
            if (interval > 0)
            {
                _interLeaveDuration = (uint32)interval;
                return true;
            }
            return false;
        }

        bool addMediaSampleInterleave(uint32 trackID,
                                      Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                                      uint32 size, uint32 ts, uint8 flags);

        bool addTextMediaSampleInterleave(uint32 trackID,
                                          Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                                          uint32 size, uint32 ts, uint8 flags, int32 index);
        //interleave buffer vector functions
        void addInterLeaveBuffer(PVA_FF_InterLeaveBuffer	*pInterLeaveBuffer);
        PVA_FF_InterLeaveBuffer*	getInterLeaveBuffer(uint32	trackID);

        bool setOutputFileName(PVA_FF_UNICODE_STRING_PARAM outputFileName);
        bool setOutputFileHandle(MP4_AUTHOR_FF_FILE_HANDLE outputFileHandle);

        bool reAuthorFirstSampleInTrack(uint32 trackID,
                                        uint8 *psample,
                                        uint32 size);
        void setLanguage(uint32 language)
        {
            if (_pmovieAtom != NULL)
                _pmovieAtom->setLanguage(language);
        }

        void SetCacheSize(uint32 aCacheSize);

        // movie fragment : set duration of MOOF
        void setMovieFragmentDuration(uint32 duration);
        uint32 getMovieFragmentDuration();

        bool renderMoovAtom();
        bool renderMovieFragments();

    private:

        PVA_FF_UNICODE_HEAP_STRING _title;
        bool         _oSetTitleDone;

        PVA_FF_UNICODE_HEAP_STRING _author;
        bool         _oSetAuthorDone;

        PVA_FF_UNICODE_HEAP_STRING _copyright;
        bool         _oSetCopyrightDone;

        PVA_FF_UNICODE_HEAP_STRING _description;
        bool         _oSetDescriptionDone;

        PVA_FF_UNICODE_HEAP_STRING _performer;
        bool         _oSetPerformerDone;

        PVA_FF_UNICODE_HEAP_STRING _genre;
        bool         _oSetGenreDone;

        PVA_FF_UNICODE_HEAP_STRING _ratingInfo;
        uint32 _ratingEntity;
        uint32 _ratingCriteria;
        bool   _oSetRatingDone;


        PVA_FF_UNICODE_HEAP_STRING _classificationInfo;
        uint32 _classificationEntity;
        uint32 _classificationTable;
        bool         _oSetClassificationDone;

        PVA_FF_UNICODE_HEAP_STRING _keyWordInfo;
        uint8 _keyWordSize;

        PVA_FF_UNICODE_HEAP_STRING _locationName;
        PVA_FF_UNICODE_HEAP_STRING _locationInfoAstrBody;
        PVA_FF_UNICODE_HEAP_STRING _locationInfoAddNotes;
        uint8  _locationInfoRole;
        uint32 _locationInfoLongitude;
        uint32 _locationInfoLatitude;
        uint32 _locationInfoAltitude;
        bool         _oSetLocationInfoDone;

        PVA_FF_UNICODE_HEAP_STRING _albumInfo;
        bool         _oSetAlbumDone;

        uint16	_recordingYear;
        bool	_oSetRecordingYearDone;


        PVA_FF_UNICODE_HEAP_STRING _creationDate;
        bool         _oSetCreationDateDone;

        virtual void recomputeSize();

        PVA_FF_UserDataAtom *_puserDataAtom;

        PVA_FF_FileTypeAtom       *_pFileTypeAtom;
        PVA_FF_MovieAtom          *_pmovieAtom;

        Oscl_Vector<PVA_FF_MediaDataAtom*, OsclMemAllocator> *_pmediaDataAtomVec;

        // In movie fragment mode these parameters hold
        // the current MOOF and MDAT to add samples. They are destroyed once
        // they are written to file
        PVA_FF_MovieFragmentAtom								*_pCurrentMoofAtom;
        PVA_FF_MediaDataAtom									*_pCurrentMediaDataAtom;

        // This media data atom holds MDAT for movie fragments only

        // Movie Fragment random access atom in case of movie fragments
        PVA_FF_MovieFragmentRandomAccessAtom *_pMfraAtom;

        // Movie Fragment mode parameters
        bool		_oMovieFragmentEnabled;
        bool		_oComposeMoofAtom;			// flag when true, samples are added in MOOF
        uint32		_currentMoofOffset;			// contains the offset of current MOOF before MDAT,
        // actual offset is current offset + size of MDAT for this MOOF
        uint32		_movieFragmentDuration;		// always in milliseconds : decides total sample length in MOOF and MOOV
        bool		_oTrunStart;				// true when new TRUN is to be added in MOOF from interleave buffer
        // made true for 1st sample of interleave buffer
        uint32		_sequenceNumber;			// keep track of number of movie fragments added

        // interleave buffers and vectors
        Oscl_Vector<PVA_FF_InterLeaveBuffer*, OsclMemAllocator> *_pInterLeaveBufferVec;

        uint32 _size;
        bool _modifiable;

        int32 _scalability;
        int32 _fileType;

        bool _firstFrameInLayer0;
        bool _firstFrameInLayer1;

        // Postfix string to handle multiple instances of the output filter,
        // the temporary file names will be different for every instances
        PVA_FF_UNICODE_HEAP_STRING _tempFilePostfix;

        bool             _downLoadOnly;

        int32 _codecType;

        // an output path for generated temporary files (.mdat)
        PVA_FF_UNICODE_HEAP_STRING _tempOutputPath;

        bool _fileWriteFailed;
        bool _oFileRenderCalled;
        bool _oUserDataPopulated;
        bool _oFtypPopulated;

        bool _o3GPPTrack;
        bool _oWMFTrack;
        bool _oMPEGTrack;
        bool _oPVMMTrack;

        uint32 _fileAuthoringFlags;
        bool   _oInterLeaveEnabled;
        bool   _oMovieAtomUpfront;
        uint32 _interLeaveDuration; // Always in milliseconds


        bool   _oAuthorASSETINFOAtoms;
        uint32 _baseOffset;
        bool   _oChunkStart;

        uint16 _nextAvailableODID;
        int32  _tempFileIndex;

        void *_aFs;

        PVA_FF_UNICODE_HEAP_STRING _targetFileName;
        MP4_AUTHOR_FF_FILE_HANDLE  _targetFileHandle;

        bool        _oPartialTempFileRemoval;
        bool        _oDirectRenderEnabled;
        uint32      _initialUserDataSize;
        uint32      _directRenderFileOffset;

        PVA_FF_UNICODE_HEAP_STRING _outputFileName;
        MP4_AUTHOR_FF_FILE_HANDLE  _outputFileHandle;

        bool        _outputFileNameSet;
        bool        _totalTempFileRemoval;
        bool        _oUserDataUpFront;

        bool        _oFileOpenedOutsideAFFLib;

        bool        _oFirstSampleEditMode;
        uint32      iCacheSize;
        bool		_oIsFileOpen;
};



#endif

