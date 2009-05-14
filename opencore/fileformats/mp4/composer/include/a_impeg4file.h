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
/**
 * The PVA_FF_Mpeg4File Class is the class that will construct and maintain all the
 * mecessary data structures to be able to render a valid MP4 file to disk.
 */

#ifndef __A_IMpeg4File_H__
#define __A_IMpeg4File_H__

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#include "pv_mp4ffcomposer_config.h"
#include "a_isucceedfail.h"
#include "a_atomdefs.h"

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef TEXTSAMPLEDESCINFO_H
#include "textsampledescinfo.h"
#endif

#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif

typedef OSCL_HeapString<OsclMemAllocator> PVA_FF_UTF8_HEAP_STRING;
typedef const OSCL_wString& PVA_FF_UNICODE_STRING_PARAM;
typedef OSCL_wHeapString<OsclMemAllocator> PVA_FF_UNICODE_HEAP_STRING;
typedef Oscl_File* MP4_AUTHOR_FF_FILE_HANDLE;

struct GAU;

class PVA_FF_IMpeg4File : public PVA_FF_ISucceedFail
{
    public:
        /**
         * This factory method is the main entry point into the MP4 file composer library.
        */

        /* This factory API creates an instance of the interface object, based on arguments.
        * In that process, it creates and initializes all the classes that hold the meta data
        * (time stamp, sample size, chunk tables etc) for the presentation. This API also
        * initializes container classes that hold the media data, which could be the
        * target file in case of no temp file authoring mode.
        */
        /* @param mediaType   int32 that could be any of the following:
        *                    FILE_TYPE_AUDIO, FILE_TYPE_VIDEO, FILE_TYPE_AUDIO_VIDEO,
        *                    FILE_TYPE_STILL_IMAGE, FILE_TYPE_STILL_IMAGE_AUDIO.
        *
        * @param outputPath  string. This is the location at which the temp files generated
        *                    during authoring would be stored.
        *
        * @param postFix     string. This is a tag that is used to distinguish between multiple
        *                    instances of the fileformat lib, so that there is no confilict in
        *                    in the names of temp files, across multiple simultaneous authoring
        *                    sessions.
        *
        * @param osclFileServerSession void*. This is a pointer to the fileserver session, that
        *                              is required in platforms like Symbian, to open a file.
        *
        * @param outputFileName  string. This argument contains the target MP4/3GP file, that
        *                        would be the outcome of the authoring process.
        *
        * @param fileAuthoringFlags uint32. This argument specifies the file authoring mode.
        *                           It should be one of the following:
        *                           DEFAULT_AUTHORING_MODE - Authors PV Fastrack Downloadable
        *                           clips. No media interleaving, and meta data is authored
        *                           towards the end of the clip. Creates temp media files.
        *                           SET_3GPP_PROGRESSIVE_DOWNLOAD_MODE - Authors 3GPP PD content.
        *                           This implies that meta data is upfront, media data interleaved.
        *                           This mode requires the use of temp files as well.
        */

        /*							 SET_MOVIE_FRAGMENT_MODE - Authors Movie fragment clips. This implies
        *							 that file is interleaved, meta data is after the media data in moov
        *							 and movie fragments.
        *
        * @return PVA_FF_IMpeg4File*  Pointer to PVA_FF_IMpeg4File class. This is the interface
        *                             object, that the user of the library would use to invoke
        *                             the rest of the APIs, to control the authoring process.
        */
        OSCL_IMPORT_REF
        static PVA_FF_IMpeg4File *createMP4File(int32 mediaType,
                                                PVA_FF_UNICODE_STRING_PARAM outputPath,
                                                PVA_FF_UNICODE_STRING_PARAM postFix,
                                                void* osclFileServerSession = NULL,
                                                uint32 fileAuthoringFlags = PVMP4FF_3GPP_PROGRESSIVE_DOWNLOAD_MODE,
                                                PVA_FF_UNICODE_STRING_PARAM outputFileName = PVA_FF_UNICODE_HEAP_STRING(_STRLIT_WCHAR("")),
                                                uint32 aCacheSize = 0);

        /**
         * This overloaded factory method is an alternate entry point into the MP4 file
         * composer library, under certain usecase scenarios.
         *
         * This factory API creates an instance of the interface object, based on arguments.
         * In that process, it creates and initializes all the classes that hold the meta data
         * (time stamp, sample size, chunk tables etc) for the presentation. This API is to be
         * used only in case of the NO_TEMP_FILE authoring mode. The difference between the
         * overloaded version, and the version above is that, the former take a FILE HANDLE
         * (actual opening of the target file is done outside the FF lib), latter takes a
         * FILE NAME (the target file is opened inside the FF lib in this case)
         *
         * @param mediaType   int32 that could be any of the following:
         *                    FILE_TYPE_AUDIO, FILE_TYPE_VIDEO, FILE_TYPE_AUDIO_VIDEO,
         *                    FILE_TYPE_STILL_IMAGE, FILE_TYPE_STILL_IMAGE_AUDIO.
         *
         * @param fileAuthoringFlags uint32. This argument specifies the file authoring mode.
         *                           It should be one of the following:
         *                           SET_NO_TEMP_FILE_AUTHORING_MODE - Does not use any temp files.
         *                           Media data is interleaved and meta data is stored at the end
         *                           of the file.
         *
         * @param outputFileHandle  file pointer. This argument contains the target MP4/3GP file pointer,
         *                          that would be the outcome of the authoring process. Note that the
         *                          actual file open is done outside the fileformat library.
         *
         *
         * @return PVA_FF_IMpeg4File*  Pointer to PVA_FF_IMpeg4File class. This is the interface
         *                             object, that the user of the library would use to invoke
         *                             the rest of the APIs, to control the authoring process.
         */
        OSCL_IMPORT_REF
        static PVA_FF_IMpeg4File *createMP4File(int32 mediaType,
                                                uint32 fileAuthoringFlags = PVMP4FF_3GPP_DOWNLOAD_MODE,
                                                MP4_AUTHOR_FF_FILE_HANDLE outputFileHandle = NULL,
                                                uint32 aCacheSize = 0);

        virtual ~PVA_FF_IMpeg4File() {} // So the PVA_FF_Mpeg4File destructor gets called when delete the interface

        /**
         *
         * Following methods are used to populate the clip info strings.
         * These include, clip author, clip title, version (version of
         * authoring tool that was used to author this clip), clip rating,
         * clip copyright, clip description & clip creation date.
         */

        /* @param version   version string.
         * @param langCode  16 bit ISO-639-2/T Language code
         *
         * @return None.
         */
        virtual void setVersion(PVA_FF_UNICODE_STRING_PARAM version,
                                uint16 langCode = LANGUAGE_CODE_UNKNOWN) = 0;

        /**
         * This method populates the title string. Title string contains
         * the title of the authored clip.Currently only wide char strings are supported.
         */
        /* @param title   title string.
        * @param langCode  16 bit ISO-639-2/T Language code
        *
        * @return None.
        */
        virtual void setTitle(PVA_FF_UNICODE_STRING_PARAM title,
                              uint16 langCode = LANGUAGE_CODE_UNKNOWN) = 0;

        /**
         * This method populates the author string. Title string information about
         * the the author of the clip (actual user - like authored by grandma martha).
         * Currently only wide char strings are supported.
        */
        /* @param author   author string.
        * @param langCode  16 bit ISO-639-2/T Language code
        *
        * @return None.
        */
        virtual void setAuthor(PVA_FF_UNICODE_STRING_PARAM author,
                               uint16 langCode = LANGUAGE_CODE_UNKNOWN) = 0;


        /**
         * This method populates the copyright string.
         */

        /* @param copyright   copyright string.
        * @param langCode  16 bit ISO-639-2/T Language code
        *
        * @return None.
        */
        virtual void setCopyright(PVA_FF_UNICODE_STRING_PARAM copyright,
                                  uint16 langCode = LANGUAGE_CODE_UNKNOWN) = 0;

        /**
         * This method populates the description string. Description string contains
         * some brief description of the clip (viz. surfing on the beach).
         */
        /* @param description   description string.
        * @param langCode  16 bit ISO-639-2/T Language code
        *
        * @return None.
        */
        virtual void setDescription(PVA_FF_UNICODE_STRING_PARAM description,
                                    uint16 langCode = LANGUAGE_CODE_UNKNOWN) = 0;

        /**
         * This method populates the rating string. Rating string contains
         * some information about the clip rating (viz.PG-13).
         */

        /* @param rating   rating string.
        * @param langCode  16 bit ISO-639-2/T Language code
        *
        * @return None.
        */

        virtual void setRating(PVA_FF_UNICODE_STRING_PARAM ratingInfo,
                               uint16 langCode = LANGUAGE_CODE_UNKNOWN,
                               uint32 ratingEntity = RATING_ENTITY_UNKNOWN,
                               uint32 ratingCriteria = RATING_CRITERIA_UNKNOWN) = 0;

        /**
         * This method populates the creation date, both in PV user data atoms, as well as
         * in other standard atoms like movieheader, trackheader etc. Date is input as a
         * string. The format of date is as per ISO 8601 spec. Currently only
         * wide char strings are supported.
         *
         * @param creationDate   creationDate string.
         *
         * @return None.
         */


        virtual void setPerformer(PVA_FF_UNICODE_STRING_PARAM performer,
                                  uint16 langCode = LANGUAGE_CODE_UNKNOWN) = 0;


        virtual void setGenre(PVA_FF_UNICODE_STRING_PARAM genre,
                              uint16 langCode = LANGUAGE_CODE_UNKNOWN) = 0;

        virtual void setClassification(PVA_FF_UNICODE_STRING_PARAM classificationInfo, uint32 classificationEntity,
                                       uint16 classificationTable, uint16 langCode = LANGUAGE_CODE_UNKNOWN) = 0;

        virtual void setKeyWord(uint8 keyWordSize, PVA_FF_UNICODE_HEAP_STRING keyWordInfo, uint16 langCode = LANGUAGE_CODE_UNKNOWN) = 0;

        virtual void setLocationInfo(PvmfAssetInfo3GPPLocationStruct*) = 0;
        virtual void setAlbumInfo(PVA_FF_UNICODE_STRING_PARAM albumInfo, uint16 langCode = LANGUAGE_CODE_UNKNOWN) = 0;
        virtual void setRecordingYear(uint16 recordingYear) = 0;

        virtual void setCreationDate(PVA_FF_UNICODE_STRING_PARAM creationDate) = 0;

        // Methods to set the sample rate (i.e. timescales) for the streams and
        // the overall Mpeg-4 presentation
        virtual void setPresentationTimescale(uint32 timescale) = 0; // Timescale of overall MP4 presentation (i.e. milliseconds?)

        // Possible FILE TYPE return values (defined in AtomDefs.h):
        // FILE_TYPE_AUDIO = 1;
        // FILE_TYPE_VIDEO = 2;
        // FILE_TYPE_AUDIO_VIDEO = 3; // logical ORing of above
        // FILE_TYPE_STILL_IMAGE = 4;
        // FILE_TYPE_STILL_IMAGE_AUDIO = 5;
        virtual int32 getFileType() const = 0;


        // MPEG4 header retrieval methods
        virtual void setDecoderSpecificInfo(uint8 * header, int32 size, int32 trackID) = 0;
        // MPEG4 header retrieval methods for timed text
        virtual void setTextDecoderSpecificInfo(PVA_FF_TextSampleDescInfo *header, int32 trackID) = 0;

        ///////////////////////
        // RENDERING METHODS //
        ///////////////////////

        // Method to render the MP4 file to disk
        virtual bool renderToFile(PVA_FF_UNICODE_STRING_PARAM filename/*=PVA_FF_EMPTY_STRING*/) = 0;

        // Access function to set the postfix string for PVA_FF_MediaDataAtom objects
        virtual void SetTempFilePostFix(PVA_FF_UNICODE_STRING_PARAM postFix) = 0;

        virtual uint32  addTrack(int32 mediaType,
                                 int32 codecType,
                                 bool oDirectRender = false,
                                 uint8 profile = 1,
                                 uint8 profileComp = 0xFF,
                                 uint8 level = 0xFF) = 0;

        virtual bool addSampleToTrack(uint32 trackID,
                                      Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,	// vector which contains either NALs or samples
                                      uint32 ts, uint8 flags) = 0;
        virtual bool addTextSampleToTrack(uint32 trackID,
                                          Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,	// vector which contains either NALs or samples
                                          uint32 ts, uint8 flags, int32 index, uint8* modifierinfo) = 0;
        virtual void addTrackReference(uint32 currtrackID, int32 reftrackID) = 0;
        virtual void setTargetBitRate(uint32 trackID, uint32 bitrate) = 0;
        virtual void setTimeScale(uint32 trackID, uint32 rate) = 0;
        virtual void setMaxBufferSizeDB(uint32 trackID, uint32 max) = 0;

        virtual void setMajorBrand(uint32 brand = BRAND_3GPP4) = 0;
        virtual void setMajorBrandVersion(uint32 version = VERSION_3GPP4) = 0;
        virtual void addCompatibleBrand(uint32 brand) = 0;

        virtual void setVideoParams(uint32 trackID, float frate, uint16 interval,
                                    uint32 frame_width, uint32 frame_height) = 0;

        virtual void setH263ProfileLevel(uint32 trackID,
                                         uint8 profile,
                                         uint8 level) = 0;

        virtual bool addMultipleAccessUnitsToTrack(uint32 trackID, GAU *pgau) = 0;
        virtual bool renderTruncatedFile(PVA_FF_UNICODE_STRING_PARAM filename) = 0;

        virtual bool getTargetFileSize(uint32 &metaDataSize, uint32 &mediaDataSize) = 0;

        virtual bool prepareToEncode() = 0;


        virtual bool setInterLeaveInterval(int32 interval) = 0;

        virtual bool reAuthorFirstSampleInTrack(uint32 trackID,
                                                uint8 *psample,
                                                uint32 size) = 0;
        virtual void setLanguage(uint32 language) = 0;

        // movie fragment duration
        virtual void setMovieFragmentDuration(uint32 duration) = 0;
        virtual uint32 getMovieFragmentDuration() = 0;

        OSCL_IMPORT_REF static void DestroyMP4FileObject(PVA_FF_IMpeg4File* aMP4FileObject);
};

#endif


