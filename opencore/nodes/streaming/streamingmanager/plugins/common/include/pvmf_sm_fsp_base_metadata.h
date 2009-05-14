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
#ifndef PVMF_SM_FSP_BASE_METADATA_H_INCLUDED
#define PVMF_SM_FSP_BASE_METADATA_H_INCLUDED

#ifndef COMMONINFO_H
#include "common_info.h"
#endif

// Number of metadata keys supported in this node.
#define PVMFSTREAMINGMGRNODE_NUM_METADATAKEYS 16
// Constant character strings for metadata keys
//Common to all streaming sessions
static const char PVMFSTREAMINGMGRNODE_ALBUM_KEY[] = "album";
static const char PVMFSTREAMINGMGRNODE_AUTHOR_KEY[] = "author";
static const char PVMFSTREAMINGMGRNODE_ARTIST_KEY[] = "artist";
static const char PVMFSTREAMINGMGRNODE_TITLE_KEY[] = "title";
static const char PVMFSTREAMINGMGRNODE_DESCRIPTION_KEY[] = "description";
static const char PVMFSTREAMINGMGRNODE_RATING_KEY[] = "rating";
static const char PVMFSTREAMINGMGRNODE_COPYRIGHT_KEY[] = "copyright";
static const char PVMFSTREAMINGMGRNODE_GENRE_KEY[] = "genre";
static const char PVMFSTREAMINGMGRNODE_LYRICS_KEY[] = "lyrics";
static const char PVMFSTREAMINGMGRNODE_CLASSIFICATION_KEY[] = "classification";
static const char PVMFSTREAMINGMGRNODE_KEYWORDS_KEY[] = "keywords";
static const char PVMFSTREAMINGMGRNODE_LOCATION_KEY[] = "location;format=3GPP_LOCATION";
static const char PVMFSTREAMINGMGRNODE_DURATION_KEY[] = "duration";
static const char PVMFSTREAMINGMGRNODE_NUMTRACKS_KEY[] = "num-tracks";
static const char PVMFSTREAMINGMGRNODE_RANDOM_ACCESS_DENIED_KEY[] = "random-access-denied";
static const char PVMFSTREAMINGMGRNODE_YEAR_KEY[] = "year";
static const char PVMFSTREAMINGMGRNODE_NUM_GRAPHICS_KEY[] = "graphic/num-frames;format=APIC";
static const char PVMFSTREAMINGMGRNODE_GRAPHICS_KEY[] = "graphic;format=APIC";
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_TYPE_KEY[] = "track-info/type";
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_DURATION_KEY[] = "track-info/duration";
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_SELECTED_KEY[] = "track-info/selected";
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_WIDTH_KEY[] = "track-info/video/width";
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_HEIGHT_KEY[] = "track-info/video/height";
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_SAMPLERATE_KEY[] = "track-info/sample-rate";
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_NUMCHANNELS_KEY[] = "track-info/audio/channels";
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_AUDIO_BITS_PER_SAMPLE_KEY[] = "track-info/audio/bits-per-sample";
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_TRACKID_KEY[] = "track-info/track-id";

static const char PVMFSTREAMINGMGRNODE_CLIP_TYPE_KEY[] = "clip-type";
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_FRAME_RATE_KEY[] = "track-info/frame-rate";
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_NAME_KEY[] = "track-info/codec-name";
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_DESCRIPTION_KEY[] = "track-info/codec-description";
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_DATA_KEY[] = "track-info/codec-specific-info";

//PVMF_DRM_INFO_IS_PROTECTED_QUERY

//Extended metadata [FF specific]

//Streaming specific metadata keys
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_BITRATE_KEY[] = "track-info/bit-rate";			//RTSP Specific
static const char PVMFSTREAMINGMGRNODE_PAUSE_DENIED_KEY[] = "pause-denied";						//RTSP Unicast Specific
static const char PVMFSTREAMINGMGRNODE_TRACKINFO_MAX_BITRATE_KEY[] = "track-info/max-bitrate";	//HTTP streaming Specific

///////////////////////////////////////////////////////////////////////////////
static const char PVMFSTREAMINGMGRNODE_SEMICOLON[] = ";";
static const char PVMFSTREAMINGMGRNODE_TIMESCALE[] = "timescale=";
static const char PVMFSTREAMINGMGRNODE_INDEX[] = "index=";
static const char PVMFSTREAMINGMGRNODE_MAXSIZE[] = "maxsize=";
static const char PVMFSTREAMINGMGRNODE_REQSIZE[] = "reqsize=";
static const char PVMFSTREAMINGMGRNODE_TRUNCATE_FLAG[] = "truncate=";

static const unsigned int RELIABLE_BURST_PVSS_VER = 5; /* It was found that only PVSS version 5 does reliable burst streaming */

///////////////////////////////////////////////////////////////////////////////
//Datastructures to store metadata info
///////////////////////////////////////////////////////////////////////////////
class PVMFSMTrackMetaDataInfo
{
    public:
        PVMFSMTrackMetaDataInfo()
        {
            iTrackDurationAvailable = false;
            iTrackDuration = 0;
            iTrackDurationTimeScale = 0;
            iTrackBitRate = 0;
            iTrackMaxBitRate = 0;
            iTrackSelected = false;
            iTrackID = iTrackID;
            iCodecName = NULL;
            iCodecDescription = NULL;
            iTrackWidth = 0;
            iTrackHeight = 0;
            iVideoFrameRate = 0;
            iAudioSampleRate = 0;
            iAudioNumChannels = 0;
            iAudioBitsPerSample = 0;
        };

        PVMFSMTrackMetaDataInfo(const PVMFSMTrackMetaDataInfo& a)
        {
            iTrackDurationAvailable = a.iTrackDurationAvailable;
            iTrackDuration = a.iTrackDuration;
            iTrackDurationTimeScale = a.iTrackDurationTimeScale;
            iMimeType = a.iMimeType;
            iTrackBitRate = a.iTrackBitRate;
            iTrackMaxBitRate = a.iTrackMaxBitRate;
            iTrackSelected = a.iTrackSelected;
            iTrackID = a.iTrackID;
            iCodecName = a.iCodecName;
            iCodecDescription = a.iCodecDescription;
            iCodecSpecificInfo = a.iCodecSpecificInfo;
            iTrackWidth = a.iTrackWidth;
            iTrackHeight = a.iTrackHeight;
            iVideoFrameRate = a.iVideoFrameRate;
            iAudioSampleRate = a.iAudioSampleRate;
            iAudioNumChannels = a.iAudioNumChannels;
            iAudioBitsPerSample = a.iAudioBitsPerSample;
        };

        PVMFSMTrackMetaDataInfo& operator=(const PVMFSMTrackMetaDataInfo& a)
        {
            if (&a != this)
            {
                iTrackDurationAvailable = a.iTrackDurationAvailable;
                iTrackDuration = a.iTrackDuration;
                iTrackDurationTimeScale = a.iTrackDurationTimeScale;
                iMimeType = a.iMimeType;
                iTrackBitRate = a.iTrackBitRate;
                iTrackMaxBitRate = a.iTrackMaxBitRate;
                iTrackSelected = a.iTrackSelected;
                iTrackID = a.iTrackID;
                iCodecName = a.iCodecName;
                iCodecDescription = a.iCodecDescription;
                iCodecSpecificInfo = a.iCodecSpecificInfo;
                iTrackWidth = a.iTrackWidth;
                iTrackHeight = a.iTrackHeight;
                iVideoFrameRate = a.iVideoFrameRate;
                iAudioSampleRate = a.iAudioSampleRate;
                iAudioNumChannels = a.iAudioNumChannels;
                iAudioBitsPerSample = a.iAudioBitsPerSample;
            }
            return (*this);
        };

        bool   iTrackDurationAvailable;
        uint64 iTrackDuration;
        uint64 iTrackDurationTimeScale;
        OSCL_HeapString<OsclMemAllocator> iMimeType;
        uint32 iTrackBitRate;
        uint32 iTrackMaxBitRate;
        bool   iTrackSelected;
        uint32 iTrackID;
        OSCL_wHeapString<OsclMemAllocator> iCodecName;
        OSCL_wHeapString<OsclMemAllocator> iCodecDescription;
        uint32 iTrackWidth;
        uint32 iTrackHeight;
        uint32 iVideoFrameRate;
        uint32 iAudioSampleRate;
        uint32 iAudioNumChannels;
        uint32 iAudioBitsPerSample;
        OsclRefCounterMemFrag iCodecSpecificInfo;
};

class PVMFSMSessionMetaDataInfo
{
    public:
        PVMFSMSessionMetaDataInfo()
        {
            Reset();
        };

        void Reset()
        {
            iMetadataAvailable = false;
            iTitlePresent = false;
            iDescriptionPresent = false;
            iCopyRightPresent = false;
            iPerformerPresent = false;
            iAuthorPresent = false;
            iGenrePresent = false;
            iRatingPresent = false;
            iClassificationPresent = false;
            iKeyWordsPresent = false;
            iLocationPresent = false;
            iLyricsPresent = false;
            iWMPicturePresent = false;
            iAlbumPresent = false;

            iIsTitleUnicode = false;
            iIsDescriptionUnicode = false;
            iIsCopyRightUnicode = false;
            iIsPerformerUnicode = false;
            iIsAuthorUnicode = false;
            iIsGenreUnicode = false;
            iIsRatingUnicode = false;
            iIsClassificationUnicode = false;
            iIsKeyWordsUnicode = false;
            iIsLocationUnicode = false;
            iIsLyricsUnicode = false;
            iIsAlbumUnicode = false;

            iSessionDurationAvailable = false;
            iSessionDuration = 0;
            iSessionDurationTimeScale = 0;
            iNumTracks = 0;
            iRandomAccessDenied = false;
            iNumWMPicture = 0;

            iExtendedMetaDataDescriptorCount = 0;
            iExtendedMetaDataNameVec.clear();
            iExtendedMetaDataValueTypeVec.clear();
            iExtendedMetaDataValueLenVec.clear();
            iExtendedMetaDataIndexVec.clear();

            iTrackMetaDataInfoVec.clear();

            iNumKeyWords = 0;
            iYear = 0;
        };

        PVMFSMTrackMetaDataInfo* getTrackMetaDataInfo(uint32 aId)
        {
            Oscl_Vector<PVMFSMTrackMetaDataInfo, OsclMemAllocator>::iterator it;
            for (it = iTrackMetaDataInfoVec.begin(); it != iTrackMetaDataInfoVec.end(); it++)
            {
                if (it->iTrackID == aId)
                {
                    return (it);
                }
            }
            return NULL;
        };

        bool iTitlePresent;
        bool iDescriptionPresent;
        bool iCopyRightPresent;
        bool iPerformerPresent;
        bool iAuthorPresent;
        bool iGenrePresent;
        bool iRatingPresent;
        bool iClassificationPresent;
        bool iKeyWordsPresent;
        bool iLocationPresent;
        bool iLyricsPresent;
        bool iWMPicturePresent;
        bool iAlbumPresent;

        bool iIsTitleUnicode;
        bool iIsDescriptionUnicode;
        bool iIsCopyRightUnicode;
        bool iIsPerformerUnicode;
        bool iIsAuthorUnicode;
        bool iIsGenreUnicode;
        bool iIsRatingUnicode;
        bool iIsClassificationUnicode;
        bool iIsKeyWordsUnicode;
        bool iIsLocationUnicode;
        bool iIsLyricsUnicode;
        bool iIsAlbumUnicode;
        uint32 iNumWMPicture;

        OSCL_HeapString<OsclMemAllocator> iTitle;
        OSCL_HeapString<OsclMemAllocator> iDescription;
        OSCL_HeapString<OsclMemAllocator> iCopyright;
        OSCL_HeapString<OsclMemAllocator> iPerformer;
        OSCL_HeapString<OsclMemAllocator> iAuthor;
        OSCL_HeapString<OsclMemAllocator> iGenre;
        OSCL_HeapString<OsclMemAllocator> iRating;
        OSCL_HeapString<OsclMemAllocator> iClassification;
        uint32 iNumKeyWords;
        OSCL_HeapString<OsclMemAllocator> iKeyWords[MAX_ASSET_INFO_KEYWORDS];
        OSCL_HeapString<OsclMemAllocator> iLyrics;
        OSCL_HeapString<OsclMemAllocator> iAlbum;

        OSCL_wHeapString<OsclMemAllocator> iTitleUnicode;
        OSCL_wHeapString<OsclMemAllocator> iDescriptionUnicode;
        OSCL_wHeapString<OsclMemAllocator> iCopyrightUnicode;
        OSCL_wHeapString<OsclMemAllocator> iPerformerUnicode;
        OSCL_wHeapString<OsclMemAllocator> iAuthorUnicode;
        OSCL_wHeapString<OsclMemAllocator> iGenreUnicode;
        OSCL_wHeapString<OsclMemAllocator> iRatingUnicode;
        OSCL_wHeapString<OsclMemAllocator> iClassificationUnicode;
        OSCL_wHeapString<OsclMemAllocator> iKeyWordUnicode[MAX_ASSET_INFO_KEYWORDS];
        OSCL_wHeapString<OsclMemAllocator> iLyricsUnicode;
        OSCL_wHeapString<OsclMemAllocator> iAlbumUnicode;

        uint32 iYear;
        bool   iSessionDurationAvailable;
        uint64 iSessionDuration;
        uint64 iSessionDurationTimeScale;
        uint32 iNumTracks;
        bool   iRandomAccessDenied;
        PvmfAssetInfo3GPPLocationStruct iLocationStruct;

        Oscl_Vector<PVMFSMTrackMetaDataInfo, OsclMemAllocator> iTrackMetaDataInfoVec;

        uint32 iExtendedMetaDataDescriptorCount;
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iExtendedMetaDataNameVec;
        Oscl_Vector<uint16, OsclMemAllocator> iExtendedMetaDataValueTypeVec;
        Oscl_Vector<uint32, OsclMemAllocator> iExtendedMetaDataValueLenVec;
        Oscl_Vector<uint32, OsclMemAllocator> iExtendedMetaDataIndexVec;
        Oscl_Vector<uint32, OsclMemAllocator> iWMPictureIndexVec;

        bool iMetadataAvailable;
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#endif
