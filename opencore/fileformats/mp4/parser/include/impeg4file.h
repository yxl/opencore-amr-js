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

#ifndef IMPEG4FILE_H_INCLUDED
#define IMPEG4FILE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

//HEADER FILES REQD FOR MULTIPLE SAMPLE RETRIEVAL API
#ifndef OSCL_MEDIA_DATA_H_INCLUDED
#include "oscl_media_data.h"
#endif
#ifndef PV_GAU_H_INCLUDED
#include "pv_gau.h"
#endif

#ifndef TEXTSAMPLEENTRY_H_INCLUDED
#include "textsampleentry.h"
#endif
#ifndef FONTRECORD_H_INCLUDED
#include "fontrecord.h"
#endif

#ifndef ATOMDEFS_H_INCLUDED
#include "atomdefs.h"
#endif

#ifndef PV_MP4FFPARSER_CONFIG_H_INCLUDED
#include "pv_mp4ffparser_config.h"
#endif

/* CPM Related Header Files */
#ifndef CPM_H_INCLUDED
#include "cpm.h"
#endif
#include "pv_id3_parcom_types.h"

class OsclFileHandle;
class PvmiDataStreamObserver;
class AVCSampleEntry;
/*------------- Interface of Class Mpeg4 File ----------------*/
class IMpeg4File : public ISucceedFail
{
    public:
        virtual ~IMpeg4File() {} // So the Mpeg4File destructor gets called when delete the interface

        // MEDIA DATA APIS

        /* Returns media samples for the requested tracks
           id:	The track ID of the track from which the method is to retrieve the samples.
           buf:	A pointer to the buffer into which to place the sample.
           size:	The size of the data buffer
           index:	An output parameter which is the index of the sample entry to which the returned sample refers.
           return:	The size in bytes of the data placed into the provided buffer.  If the buffer is not large enough, the return value is the negative of the size that is needed.
        */
        virtual int32 getNextMediaSample(uint32 id, uint8 *buf, uint32 &size, uint32 &index, uint32 &SampleOffset) = 0;

        virtual int32 getMediaSample(uint32 id, uint32 sampleNumber, uint8 *buf, int32 &size, uint32 &index, uint32 &SampleOffset) = 0;

        virtual	int32 getOffsetByTime(uint32 id, uint32 ts, int32* sampleFileOffset, uint32 jitterbuffersize) = 0;

        virtual int32 updateFileSize(uint32	filesize) = 0;

        virtual MP4_ERROR_CODE getKeyMediaSampleNumAt(uint32 aTrackId,
                uint32 aKeySampleNum,
                GAU    *pgau) = 0;
        virtual int32 getPrevKeyMediaSample(uint32 inputtimestamp,
                                            uint32 &aKeySampleNum,
                                            uint32 id,
                                            uint32 *n,
                                            GAU    *pgau) = 0;
        virtual int32 getNextKeyMediaSample(uint32 inputtimestamp,
                                            uint32 &aKeySampleNum,
                                            uint32 id,
                                            uint32 *n,
                                            GAU    *pgau) = 0;

        /* Returns the timestamp for the previously returned media samples from the requested track
           id:	The track ID of the track from which the method is to retrieve the sample timestamp.
           return:	The timestamp of the most recently return media sample in the "media timescale"
        */
        virtual uint32 getMediaTimestampForCurrentSample(uint32 id) = 0;


        // META DATA APIS

        // From PVUserDataAtom 'pvmm'
        virtual OSCL_wString& getPVVersion(MP4FFParserOriginalCharEnc &charType) = 0;		// return _version string
        virtual OSCL_wHeapString<OsclMemAllocator> getCreationDate(MP4FFParserOriginalCharEnc &charType) = 0;	// return _creationDate string

        // from 'ftyp' atom
        virtual uint32 getCompatibiltyMajorBrand() = 0;
        virtual uint32 getCompatibiltyMajorBrandVersion() = 0;
        virtual Oscl_Vector<uint32, OsclMemAllocator> *getCompatibiltyList() = 0;

        // From Movie
        virtual int32 getNumTracks() = 0;
        virtual int32 getTrackIDList(uint32 *ids, int size) = 0;
        virtual uint32 getTrackWholeIDList(uint32 *ids) = 0;

        // From MovieHeader
        virtual uint64 getMovieDuration() const = 0;
        virtual uint32 getMovieTimescale() const = 0;


        // From TrackHeader
        virtual uint64 getTrackDuration(uint32 id) = 0; // in movie timescale

        // From TrackReference
        // Returns the track ID of the track on which this track depends
        virtual uint32 trackDependsOn(uint32 id) = 0;

        // From MediaHeader
        virtual uint64 getTrackMediaDuration(uint32 id) = 0;
        virtual uint32 getTrackMediaTimescale(uint32 id) = 0;
        virtual uint16 getTrackLangCode(uint32 id) = 0;

        // From Handler
        // Returns the 4CC of the track media type (i.e. 'vide' for video)
        virtual uint32 getTrackMediaType(uint32 id) = 0;

        // From SampleDescription
        // Returns the number of sample entries stored in the sample description
        virtual int32 getTrackNumSampleEntries(uint32 id) = 0;

        // From DecoderConfigDescriptor
        virtual void getTrackMIMEType(uint32 id, OSCL_String& aMimeType) = 0; // Based on OTI and string tables

        virtual int32  getTrackMaxBufferSizeDB(uint32 id) = 0;
        virtual int32  getTrackAverageBitrate(uint32 id) = 0;

        virtual uint8 *getTrackDecoderSpecificInfoContent(uint32 id) = 0;
        virtual uint32 getTrackDecoderSpecificInfoSize(uint32 id) = 0;
        virtual DecoderSpecificInfo *getTrackDecoderSpecificInfoAtSDI(uint32 trackID, uint32 index) = 0;

        virtual uint32 getTimestampForSampleNumber(uint32 id, uint32 sampleNumber) = 0;
        virtual int32 getSampleSizeAt(uint32 id, int32 sampleNum) = 0;

        //From PASP atom
        virtual uint32 getHspacing(uint32 id) = 0;
        virtual uint32 getVspacing(uint32 id) = 0;



        // MPEG4 header retrieval methods
        virtual int32 getFileType() const = 0;
        virtual int32 getScalability() const = 0;

        virtual int32 getTimestampForRandomAccessPoints(uint32 id, uint32 *num, uint32 *tsBuf, uint32* numBuf, uint32* offsetBuf = NULL) = 0;
        virtual int32 getTimestampForRandomAccessPointsBeforeAfter(uint32 id, uint32 ts, uint32 *tsBuf, uint32* numBuf,
                uint32 &numsamplestoget,
                uint32 howManyKeySamples = 1) = 0;

        // Static method to read in an MP4 file from disk and return the IMpeg4File interface
        OSCL_IMPORT_REF static IMpeg4File *readMP4File(OSCL_wString& aFilename,
                PVMFCPMPluginAccessInterfaceFactory* aCPMAccessFactory,
                OsclFileHandle* aHandle = NULL,
                uint32 aParsingMode = 0,
                Oscl_FileServer* aFileServSession = NULL);

        OSCL_IMPORT_REF static void DestroyMP4FileObject(IMpeg4File* aMP4FileObject);

        virtual bool CreateDataStreamSessionForExternalDownload(OSCL_wString& aFilename,
                PVMFCPMPluginAccessInterfaceFactory* aCPMAccessFactory,
                OsclFileHandle* aHandle = NULL,
                Oscl_FileServer* aFileServSession = NULL) = 0;

        virtual void DestroyDataStreamForExternalDownload() = 0;

        virtual void resetPlayback() = 0;
        virtual uint32 resetPlayback(uint32 time, uint16 numTracks, uint32 *trackList, bool bResetToIFrame = true) = 0;
        virtual int32 queryRepositionTime(uint32 time,
                                          uint16 numTracks,
                                          uint32 *trackList,
                                          bool bResetToIFrame = true,
                                          bool bBeforeRequestedTime = true) = 0;

        virtual int32 querySyncFrameBeforeTime(uint32 time, uint16 numTracks, uint32 *trackList) = 0;

        virtual int32 getNextBundledAccessUnits(const uint32 id,
                                                uint32 *n,
                                                GAU    *pgau) = 0;

        virtual int32 peekNextBundledAccessUnits(const uint32 id,
                uint32 *n,
                MediaMetaInfo *mInfo) = 0;

        virtual uint32 getSampleCountInTrack(uint32 id) = 0;

        virtual uint32 getAssetInfoClassificationEntity(int32 index = 0) const = 0;
        virtual uint16 getAssetInfoClassificationTable(int32 index = 0) const = 0;
        virtual uint16 getAssetInfoClassificationLangCode(int32 index = 0) const = 0;
        virtual OSCL_wString& getAssetInfoClassificationNotice(MP4FFParserOriginalCharEnc &charType, int32 index = 0) const = 0;
        virtual uint16 getAssetInfoNumKeyWords(int32 index = 0) const = 0;
        virtual uint16 getAssetInfoKeyWordLangCode(int32 index = 0) const = 0;
        virtual OSCL_wString& getAssetInfoKeyWord(int32 atomIndex, int32 keyWordIndex) const = 0;
        virtual int32 getNumAssetInfoClassificationAtoms() = 0;
        virtual int32 getNumAssetInfoKeyWordAtoms() = 0;
        virtual int32 getNumAssetInfoLocationAtoms() = 0;

        virtual PvmfAssetInfo3GPPLocationStruct *getAssetInfoLocationStruct(int32 index = 0) const = 0;
        virtual uint8 getAssetInfoAlbumTrackNumber(int32 index = 0) = 0;

        //id3
        virtual PVID3Version GetID3Version() const = 0;
        virtual void GetID3MetaData(PvmiKvpSharedPtrVector &id3Frames) = 0;

        virtual bool IsTFRAPresentForAllTrack(uint32 numTracks, uint32 *trackList) = 0;
        virtual uint32 getNumKeyFrames(uint32 trackid) = 0;

        virtual int16 getLayer(uint32 trackid) = 0;
        virtual uint16 getAlternateGroup(uint32 trackid) = 0;

        virtual int32 getVideoFrameHeight(uint32 trackid) = 0;
        virtual int32 getVideoFrameWidth(uint32 trackid) = 0;

        virtual int32 getTextTrackWidth(uint32 trackid) = 0;
        virtual int32 getTextTrackHeight(uint32 trackid) = 0;
        virtual int32 getTextTrackXOffset(uint32 trackid) = 0;
        virtual int32 getTextTrackYOffset(uint32 trackid) = 0;
        virtual SampleEntry *getTextSampleEntryAt(uint32 trackid, uint32 index) = 0;


        OSCL_IMPORT_REF static int32 IsXXXable(OSCL_wString& filename,
                                               int32 &metaDataSize,
                                               int32  &oMoovBeforeMdat,
                                               uint32 *pMajorBrand,
                                               uint32 *pCompatibleBrands,
                                               Oscl_FileServer* fileServSession = NULL);


        OSCL_IMPORT_REF static int32 IsXXXable(Oscl_File* fileRef,
                                               int32 &metaDataSize,
                                               int32  &oMoovBeforeMdat,
                                               uint32 *pMajorBrand,
                                               uint32 *pCompatibleBrands);

        virtual bool IsMobileMP4() = 0;

        virtual int32 getNumAMRFramesPerSample(uint32 trackID) = 0;

        virtual uint8 parseBufferAndGetNumAMRFrames(uint8* buffer, uint32 size) = 0;

        virtual uint32 getNumAVCSampleEntries(uint32 trackID) = 0;

        virtual AVCSampleEntry* getAVCSampleEntry(uint32 trackID, uint32 index = 0) = 0;

        virtual	uint32 getAVCNALLengthSize(uint32 trackID, uint32 index = 0) = 0;

        /*
         * @param Oscl_File* filePtr - File pointer to the MP4/3GP
         * file. Please note that the file open and close are handled by the
         * caller.
         *
         * @param uint32 fileSize - Size of the downloaded file, thus far.
         *
         *
         * @param bool& oIsProgressiveDownloadable - Set to true if the clip is
         * is progressive dowmloadable.
         *
         * @param uint32& metaDataSize - If the clip is progressive
         * downloadable then this API also returns the meta data size. Player
         * needs to wait for the file to grow past the metaDataSize before
         * starting playback.This param is valid only if oIsProgressiveDownloadable
         * is set to TRUE.
         *
         * @return MP4_ERROR_CODE - EVERYTHING_FINE, if a conclusion is reached
         * either way on whether a clip is progressive downloadable or not.
         * INSUFFICIENT_DATA, if more calls to this API are needed to reach a
         * decision
         * Any other return value indicates error.
         */
        OSCL_IMPORT_REF static MP4_ERROR_CODE IsProgressiveDownloadable(Oscl_File* filePtr,
                uint32  fileSize,
                bool& oIsProgressiveDownloadable,
                uint32& metaDataSize);

        /*
         * @param aCPMAccessFactory aCPMAccessFactory - Pointer to the datastream
         * factory.
         *
         * @param bool& oIsProgressiveDownloadable - Set to true if the clip is
         * is progressive dowmloadable.
         *
         * @param uint32& metaDataSize - If the clip is progressive
         * downloadable then this API also returns the meta data size. Player
         * needs to wait for the file to grow past the metaDataSize before
         * starting playback.This param is valid only if oIsProgressiveDownloadable
         * is set to TRUE.
         *
         * @return MP4_ERROR_CODE - EVERYTHING_FINE, if a conclusion is reached
         * either way on whether a clip is progressive downloadable or not.
         * INSUFFICIENT_DATA, if more calls to this API are needed to reach a
         * decision
         * Any other return value indicates error.
         */
        OSCL_IMPORT_REF static MP4_ERROR_CODE GetMetaDataSize(PVMFCPMPluginAccessInterfaceFactory* aCPMAccessFactory,
                bool& oIsProgressiveDownloadable,
                uint32& metaDataSize);


        /*
         * This API return the timestamp of a sample that is closest to the given
         * fileSize.
         *
         * @param trackID
         *
         * @param fileSize
         *
         * @param uint32& timeStamp
         *
         * @return EVERYTHING_FINE - In case a valid sample and corresponding time
         * stamp was located.
         * INSUFFICIENT_DATA in case the very first sample location is past the fileSize
         * NOT_SUPPORTED - in case "parsingMode" is set to 1, in "readMP4File"
         * call
         * Any other return value indicates ERROR.
         *
         */

        MP4_ERROR_CODE virtual getMaxTrackTimeStamp(uint32 trackID,
                uint32 fileSize,
                uint32& timeStamp) = 0;

        /*
         * This API returns the closest sample number, prior to the required timestamp
         * The timestamp is assumed to be in media time scale, hence no timescale
         * conversions are performed internally.Closest frame number is returned in
         * the argument "sampleNumber", and it is offset by "sampleOffset".
         *
         * @param trackID
         *
         * @param frameNumber
         *
         * @param timeStamp
         *
         * @param frameOffset
         *
         * @return MP4_ERROR_CODE
         * In case the frameNumber closest to timestamp plus frameOffset falls
         * beyond the track boundary, frameNumber is set to total number of samples
         * per track and the return code is "END_OF_TRACK".In other error scenarios
         * (like invalid timestamps, some missing atoms etc) frameNumber is set to
         * zero, and return code is "READ_FAILED". In case of normal operation,
         * return code is "EVERYTHING_FINE"
         */
        virtual MP4_ERROR_CODE getSampleNumberClosestToTimeStamp(uint32 trackID,
                uint32 &sampleNumber,
                uint32 timeStamp,
                uint32 sampleOffset = 0) = 0;

        /*
         * This API returns the size of the "odkm" header if present at the
         * track level. If the "odkm" header were not present at the
         * track level, this API would return ZERO. Please note that
         * the "odkm" is only present for OMA2 protected content.
         */
        virtual uint32 getTrackLevelOMA2DRMInfoSize(uint32 trackID) = 0;
        /*
         * This API returns a buffer containing the "odkm" header if present at the
         * track level. If the "odkm" header were not present at the
         * track level, this API would return NULL. The memory buffer
         * returned is owned by the file parser lib. Please note that
         * the "odkm" is only present for OMA2 protected content.
         */
        virtual uint8* getTrackLevelOMA2DRMInfo(uint32 trackID) = 0;

        /*
         * This API is used to set a callback request on the datastream interface.
         */
        virtual MP4_ERROR_CODE RequestReadCapacityNotification(PvmiDataStreamObserver& aObserver,
                uint32 aFileOffset,
                OsclAny* aContextData = NULL) = 0;

        /*
         * This API is used to cancel the callback request on the datastream interface.
         */
        virtual MP4_ERROR_CODE CancelNotificationSync() = 0;

        /*
         * This API is used to get the current file size from the datastream interface.
         */
        virtual MP4_ERROR_CODE GetCurrentFileSize(uint32& aFileSize) = 0;

        virtual int32 getTrackTSStartOffset(uint32& aTSOffset, uint32 aTrackID) = 0;

        // ITunes Specific functions

        virtual OSCL_wHeapString<OsclMemAllocator> getITunesTool() const = 0;
        virtual OSCL_wHeapString<OsclMemAllocator> getITunesEncodedBy() const = 0;
        virtual OSCL_wHeapString<OsclMemAllocator> getITunesWriter() const = 0;
        virtual OSCL_wHeapString<OsclMemAllocator> getITunesGroupData() const = 0;
        virtual uint16 getITunesThisTrackNo() const = 0;
        virtual PvmfApicStruct* getITunesImageData() const = 0;
        virtual uint16 getITunesTotalTracks() const = 0;
        virtual bool IsITunesCompilationPart() const = 0;
        virtual bool IsITunesContentRating() const = 0;
        virtual uint16 getITunesBeatsPerMinute() const = 0;
        virtual uint16 getITunesThisDiskNo() const = 0;
        virtual uint16 getITunesTotalDisks() const = 0;
        virtual OSCL_wHeapString<OsclMemAllocator> getITunesNormalizationData() const = 0;
        virtual OSCL_wHeapString<OsclMemAllocator> getITunesCDIdentifierData(uint8 aCDdatanumIndex) = 0;
        virtual OSCL_wHeapString<OsclMemAllocator> getITunesCDTrackNumberData() const = 0;
        virtual OSCL_wHeapString<OsclMemAllocator> getITunesCDDB1Data() const = 0;
        virtual OSCL_wHeapString<OsclMemAllocator> getITunesLyrics() const = 0;
        virtual	uint8 getITunesTotalCDIdentifierData() const = 0;
        virtual bool IsMovieFragmentsPresent() const = 0;
        //Returns the Subtitle of individual track
        virtual OSCL_wHeapString<OsclMemAllocator> getITunesTrackSubTitle() const = 0;

        // Reposition Related Video Track present API
        virtual void ResetVideoTrackPresentFlag() = 0;

        //APIs to return the no. of titles and their metadata values respectively.
        virtual uint32 getNumTitle() = 0;
        virtual PVMFStatus getTitle(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType) = 0;

        //APIs to return the no. of authors and their metadata values respectively.
        virtual uint32 getNumAuthor() = 0;
        virtual PVMFStatus getAuthor(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType) = 0;

        //APIs to return the no. of albums and their metadata values respectively.
        virtual uint32 getNumAlbum() = 0;
        virtual PVMFStatus getAlbum(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType) = 0;

        //APIs to return the no. of artist and their metadata values respectively.
        virtual uint32 getNumArtist() = 0;
        virtual PVMFStatus getArtist(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType) = 0;

        //APIs to return the no. of genre and their metadata values respectively.
        virtual uint32 getNumGenre() = 0;
        virtual PVMFStatus getGenre(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType) = 0;
        virtual uint16 getITunesGnreID() const = 0;

        //APIs to return the no. of year and their metadata values respectively.
        virtual uint32 getNumYear() = 0;
        virtual PVMFStatus getYear(uint32 index, uint32& aVal) = 0;

        //APIs to return the no. of copyright and their metadata values respectively.
        virtual uint32 getNumCopyright() = 0;
        virtual PVMFStatus getCopyright(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType) = 0;

        //APIs to return the no. of comment and their metadata values respectively.
        virtual uint32 getNumComment() = 0;
        virtual PVMFStatus getComment(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType) = 0;

        //APIs to return the no. of description and their metadata values respectively.
        virtual uint32 getNumDescription() = 0;
        virtual PVMFStatus getDescription(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType) = 0;

        //APIs to return the no. of rating and their metadata values respectively.
        virtual uint32 getNumRating() = 0;
        virtual PVMFStatus getRating(uint32 index, OSCL_wString& aVal, uint16& aLangCode, MP4FFParserOriginalCharEnc& aCharEncType) = 0;


};

#endif // IMPEG4FILE_H_INCLUDED
