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
#ifndef PVMF_MP4FFPARSER_EVENTS_H_INCLUDED
#define PVMF_MP4FFPARSER_EVENTS_H_INCLUDED

/**
 UUID for PV MP4 FF parser node error and information event type codes
 **/
#define PVMFMP4FFParserEventTypesUUID PVUuid(0xf86cd94a,0x3b18,0x4597,0x9f,0x54,0xb8,0x0b,0x29,0x4c,0xbb,0x12)

/**
 * An enumeration of error types from PV MP4 FF parser node
 **/
typedef enum
{
    /**
     When MP4 FF reports error READ_USER_DATA_ATOM_FAILED
    **/
    PVMFMP4FFParserErrUserDataAtomReadFailed = 1024,

    /**
     When MP4 FF reports error READ_MEDIA_DATA_ATOM_FAILED
    **/
    PVMFMP4FFParserErrMediaDataAtomReadFailed,

    /**
     When MP4 FF reports error READ_MOVIE_ATOM_FAILED
    **/
    PVMFMP4FFParserErrMovieAtomReadFailed,

    /**
     When MP4 FF reports error READ_MOVIE_HEADER_ATOM_FAILED
    **/
    PVMFMP4FFParserErrMovieHeaderAtomReadFailed,

    /**
     When MP4 FF reports error READ_TRACK_ATOM_FAILED
    **/
    PVMFMP4FFParserErrTrackAtomReadFailed,

    /**
     When MP4 FF reports error READ_TRACK_HEADER_ATOM_FAILED
    **/
    PVMFMP4FFParserErrTrackHeaderAtomReadFailed,

    /**
     When MP4 FF reports error READ_TRACK_REFERENCE_ATOM_FAILED
    **/
    PVMFMP4FFParserErrTrackReferenceAtomReadFailed,

    /**
     When MP4 FF reports error READ_TRACK_REFERENCE_TYPE_ATOM_FAILED
    **/
    PVMFMP4FFParserErrTrackReferenceTypeAtomReadFailed,

    /**
     When MP4 FF reports error READ_OBJECT_DESCRIPTOR_ATOM_FAILED
    **/
    PVMFMP4FFParserErrObjectDescriptorAtomReadFailed,

    /**
     When MP4 FF reports error READ_INITIAL_OBJECT_DESCRIPTOR_FAILED
    **/
    PVMFMP4FFParserErrInitialObjectDescriptorReadFailed,

    /**
     When MP4 FF reports error READ_OBJECT_DESCRIPTOR_FAILED
    **/
    PVMFMP4FFParserErrObjectDescriptorReadFailed,

    /**
     When MP4 FF reports error READ_MEDIA_ATOM_FAILED
    **/
    PVMFMP4FFParserErrMediaAtomReadFailed,

    /**
     When MP4 FF reports error READ_MEDIA_HEADER_ATOM_FAILED
    **/
    PVMFMP4FFParserErrMediaHeaderAtomReadFailed,

    /**
     When MP4 FF reports error READ_HANDLER_ATOM_FAILED
    **/
    PVMFMP4FFParserErrHandlerAtomReadFailed,

    /**
     When MP4 FF reports error READ_MEDIA_INFORMATION_ATOM_FAILED
    **/
    PVMFMP4FFParserErrMediaInformationAtomReadFailed,

    /**
     When MP4 FF reports error READ_MEDIA_INFORMATION_HEADER_ATOM_FAILED
    **/
    PVMFMP4FFParserErrMediaInformationHeaderAtomReadFailed,

    /**
     When MP4 FF reports error READ_VIDEO_MEDIA_HEADER_ATOM_FAILED
    **/
    PVMFMP4FFParserErrVideoMediaHeaderAtomReadFailed,

    /**
     When MP4 FF reports error READ_SOUND_MEDIA_HEADER_ATOM_FAILED
    **/
    PVMFMP4FFParserErrSoundMediaHeaderAtomReadFailed,

    /**
     When MP4 FF reports error READ_HINT_MEDIA_HEADER_ATOM_FAILED
    **/
    PVMFMP4FFParserErrHintMediaHeaderAtomReadFailed,

    /**
     When MP4 FF reports error READ_MPEG4_MEDIA_HEADER_ATOM_FAILED
    **/
    PVMFMP4FFParserErrMPEG4MediaHeaderAtomReadFailed,

    /**
     When MP4 FF reports error READ_DATA_INFORMATION_ATOM_FAILED
    **/
    PVMFMP4FFParserErrDataInformationAtomReadFailed,

    /**
     When MP4 FF reports error READ_DATA_REFERENCE_ATOM_FAILED
    **/
    PVMFMP4FFParserErrDataReferenceAtomReadFailed,

    /**
     When MP4 FF reports error READ_DATA_ENTRY_URL_ATOM_FAILED
    **/
    PVMFMP4FFParserErrDataEntryURLAtomReadFailed,

    /**
     When MP4 FF reports error READ_DATA_ENTRY_URN_ATOM_FAILED
    **/
    PVMFMP4FFParserErrDataEntryURNAtomReadFailed,

    /**
     When MP4 FF reports error READ_SAMPLE_TABLE_ATOM_FAILED
    **/
    PVMFMP4FFParserErrSampleTableAtomReadFailed,

    /**
     When MP4 FF reports error READ_TIME_TO_SAMPLE_ATOM_FAILED
    **/
    PVMFMP4FFParserErrTimeToSampleAtomReadFailed,

    /**
     When MP4 FF reports error READ_SAMPLE_DESCRIPTION_ATOM_FAILED
    **/
    PVMFMP4FFParserErrSampleDescriptionAtomReadFailed,

    /**
     When MP4 FF reports error READ_SAMPLE_SIZE_ATOM_FAILED
    **/
    PVMFMP4FFParserErrSampleSizeAtomReadFailed,

    /**
     When MP4 FF reports error READ_SAMPLE_TO_CHUNK_ATOM_FAILED
    **/
    PVMFMP4FFParserErrSampleToChunkAtomReadFailed,

    /**
     When MP4 FF reports error READ_CHUNK_OFFSET_ATOM_FAILED
    **/
    PVMFMP4FFParserErrChunkOffsetAtomReadFailed,

    /**
     When MP4 FF reports error READ_SYNC_SAMPLE_ATOM_FAILED
    **/
    PVMFMP4FFParserErrSyncSampleAtomReadFailed,

    /**
     When MP4 FF reports error READ_SAMPLE_ENTRY_FAILED
    **/
    PVMFMP4FFParserErrSampleEntryReadFailed,

    /**
     When MP4 FF reports error READ_AUDIO_SAMPLE_ENTRY_FAILED
    **/
    PVMFMP4FFParserErrAudioSampleEntryReadFailed,

    /**
     When MP4 FF reports error READ_VISUAL_SAMPLE_ENTRY_FAILED
    **/
    PVMFMP4FFParserErrVisualSampleEntryReadFailed,

    /**
     When MP4 FF reports error READ_HINT_SAMPLE_ENTRY_FAILED
    **/
    PVMFMP4FFParserErrHintSampleEntryReadFailed,

    /**
     When MP4 FF reports error READ_MPEG_SAMPLE_ENTRY_FAILED
    **/
    PVMFMP4FFParserErrMPEGSampleEntryReadFailed,

    /**
     When MP4 FF reports error READ_AUDIO_HINT_SAMPLE_FAILED
    **/
    PVMFMP4FFParserErrAudioHintSampleReadFailed,

    /**
     When MP4 FF reports error READ_VIDEO_HINT_SAMPLE_FAILED
    **/
    PVMFMP4FFParserErrVideoHintSampleReadFailed,

    /**
     When MP4 FF reports error READ_ESD_ATOM_FAILED
    **/
    PVMFMP4FFParserErrESDAtomReadFailed,

    /**
     When MP4 FF reports error READ_ES_DESCRIPTOR_FAILED
    **/
    PVMFMP4FFParserErrESDescriptorReadFailed,

    /**
     When MP4 FF reports error READ_SL_CONFIG_DESCRIPTOR_FAILED
    **/
    PVMFMP4FFParserErrSLConfigDescriptorReadFailed,

    /**
     When MP4 FF reports error READ_DECODER_CONFIG_DESCRIPTOR_FAILED
    **/
    PVMFMP4FFParserErrDecoderConfigDescriptorReadFailed,

    /**
     When MP4 FF reports error READ_DECODER_SPECIFIC_INFO_FAILED
    **/
    PVMFMP4FFParserErrDecoderSpecificInfoReadFailed,

    /**
     When MP4 FF reports error READ_FILE_TYPE_ATOM_FAILED
    **/
    PVMFMP4FFParserErrFileTypeAtomReadFailed,

    /**
     When MP4 FF reports error READ_PVTI_SESSION_INFO_FAILED
    **/
    PVMFMP4FFParserErrPVTISessionInfoReadFailed,

    /**
     When MP4 FF reports error READ_PVTI_MEDIA_INFO_FAILED
    **/
    PVMFMP4FFParserErrPVTIMediaInfoReadFailed,

    /**
     When MP4 FF reports error READ_CONTENT_VERSION_FAILED
    **/
    PVMFMP4FFParserErrContentVersionReadFailed,

    /**
     When MP4 FF reports error READ_DOWNLOAD_ATOM_FAILED
    **/
    PVMFMP4FFParserErrDownloadAtomReadFailed,

    /**
     When MP4 FF reports error READ_TRACK_INFO_ATOM_FAILED
    **/
    PVMFMP4FFParserErrTrackInfoAtomReadFailed,

    /**
     When MP4 FF reports error READ_REQUIREMENTS_ATOM_FAILED
    **/
    PVMFMP4FFParserErrRequirementsAtomReadFailed,

    /**
     When MP4 FF reports error READ_WMF_SET_MEDIA_ATOM_FAILED
    **/
    PVMFMP4FFParserErrWMFSetMediaAtomReadFailed,

    /**
     When MP4 FF reports error READ_WMF_SET_SESSION_ATOM_FAILED
    **/
    PVMFMP4FFParserErrWMFSetSessionAtomReadFailed,

    /**
     When MP4 FF reports error READ_PV_CONTENT_TYPE_ATOM_FAILED
    **/
    PVMFMP4FFParserErrPVContentTypeAtomReadFailed,

    /**
     When MP4 FF reports error READ_PV_USER_DATA_ATOM_FAILED
    **/
    PVMFMP4FFParserErrPVUserDataAtomReadFailed,

    /**
     When MP4 FF reports error READ_VIDEO_INFORMATION_ATOM_FAILED
    **/
    PVMFMP4FFParserErrVideoInformationAtomReadFailed,

    /**
     When MP4 FF reports error READ_RANDOM_ACCESS_ATOM_FAILED
    **/
    PVMFMP4FFParserErrRandomAccessAtomReadFailed,

    /**
     When MP4 FF reports error READ_AMR_SAMPLE_ENTRY_FAILED
    **/
    PVMFMP4FFParserErrAMRSampleEntryReadFailed,

    /**
     When MP4 FF reports error READ_H263_SAMPLE_ENTRY_FAILED
    **/
    PVMFMP4FFParserErrH263SampleEntryReadFailed,

    /**
     When MP4 FF reports error READ_UUID_ATOM_FAILED
    **/
    PVMFMP4FFParserErrUUIDAtomReadFailed,

    /**
     When MP4 FF reports error READ_DRM_ATOM_FAILED
    **/
    PVMFMP4FFParserErrDRMAtomReadFailed,

    /**
     When MP4 FF reports error READ_DCMD_ATOM_FAILED
    **/
    PVMFMP4FFParserErrDCMDAtomReadFailed,

    /**
     When MP4 FF reports error READ_COPYRIGHT_ATOM_FAILED
    **/
    PVMFMP4FFParserErrCopyrightAtomReadFailed,

    /**
     When MP4 FF reports error READ_FONT_TABLE_ATOM_FAILED
    **/
    PVMFMP4FFParserErrFontTableAtomReadFailed,

    /**
     When MP4 FF reports error READ_FONT_RECORD_FAILED
    **/
    PVMFMP4FFParserErrFontRecordReadFailed,

    /**
     When MP4 FF reports error READ_PV_ENTITY_TAG_ATOM_FAILED
    **/
    PVMFMP4FFParserErrPVEntityTagAtomReadFailed,

    /**
     When MP4 FF reports error READ_EDIT_ATOM_FAILED
    **/
    PVMFMP4FFParserErrEditAtomReadFailed,

    /**
     When MP4 FF reports error READ_EDITLIST_ATOM_FAILED
    **/
    PVMFMP4FFParserErrEditlistAtomReadFailed,

    /**
     When MP4 FF reports error READ_UDTA_TITL_FAILED
    **/
    PVMFMP4FFParserErrUDTATITLReadFailed,

    /**
     When MP4 FF reports error READ_UDTA_DSCP_FAILED
    **/
    PVMFMP4FFParserErrUDTADSCPReadFailed,

    /**
     When MP4 FF reports error READ_UDTA_CPRT_FAILED
    **/
    PVMFMP4FFParserErrUDTACPRTReadFailed,

    /**
     When MP4 FF reports error READ_UDTA_PERF_FAILED
    **/
    PVMFMP4FFParserErrUDTAPERFReadFailed,

    /**
     When MP4 FF reports error READ_UDTA_AUTH_FAILED
    **/
    PVMFMP4FFParserErrUDTAUTHReadFailed,

    /**
     When MP4 FF reports error READ_UDTA_GNRE_FAILED
    **/
    PVMFMP4FFParserErrUDTAGNREReadFailed,

    /**
     When MP4 FF reports error READ_UDTA_RTNG_FAILED
    **/
    PVMFMP4FFParserErrUDTARTNGReadFailed,

    /**
     When MP4 FF reports error READ_UDTA_CLSF_FAILED
    **/
    PVMFMP4FFParserErrUDTACLSFReadFailed,

    /**
     When MP4 FF reports error READ_UDTA_KYWD_FAILED
    **/
    PVMFMP4FFParserErrUDTAKYWDReadFailed,

    /**
     When MP4 FF reports error DUPLICATE_MOVIE_ATOMS
    **/
    PVMFMP4FFParserErrDuplicateMovieAtoms,

    /**
     When MP4 FF reports error NO_MOVIE_ATOM_PRESENT
    **/
    PVMFMP4FFParserErrNoMovieAtomPresent,

    /**
     When MP4 FF reports error DUPLICATE_OBJECT_DESCRIPTORS
    **/
    PVMFMP4FFParserErrDuplicateObjectDescriptors,

    /**
     When MP4 FF reports error NO_OBJECT_DESCRIPTOR_ATOM_PRESENT
    **/
    PVMFMP4FFParserErrNoObjectDescriptorAtomPresent,

    /**
     When MP4 FF reports error DUPLICATE_MOVIE_HEADERS
    **/
    PVMFMP4FFParserErrDuplicateMovieHeaders,

    /**
     When MP4 FF reports error NO_MOVIE_HEADER_ATOM_PRESENT
    **/
    PVMFMP4FFParserErrNoMovieHeaderAtomPresent,

    /**
     When MP4 FF reports error DUPLICATE_TRACK_REFERENCE_ATOMS
    **/
    PVMFMP4FFParserErrDuplicateTrackReferenceAtoms,

    /**
     When MP4 FF reports error DUPLICATE_TRACK_HEADER_ATOMS
    **/
    PVMFMP4FFParserErrDuplicateTrackHeaderAtoms,

    /**
     When MP4 FF reports error NO_TRACK_HEADER_ATOM_PRESENT
    **/
    PVMFMP4FFParserErrNoTrackHeaderAtomPresent,

    /**
     When MP4 FF reports error DUPLICATE_MEDIA_ATOMS
    **/
    PVMFMP4FFParserErrDuplicateMediaAtoms,

    /**
     When MP4 FF reports error NO_MEDIA_ATOM_PRESENT
    **/
    PVMFMP4FFParserErrNoMediaAtomPresent,

    /**
     When MP4 FF reports error READ_UNKOWN_ATOM
    **/
    PVMFMP4FFParserErrUnknownAtom,

    /**
     When MP4 FF reports error NON_PV_CONTENT
    **/
    PVMFMP4FFParserErrNonPVContent,

    /**
     When MP4 FF reports error FILE_NOT_STREAMABLE
    **/
    PVMFMP4FFParserErrFileNotStreamable,

    /**
     When MP4 FF reports error INVALID_SAMPLE_SIZE
    **/
    PVMFMP4FFParserErrInvalidSampleSize,

    /**
     When MP4 FF reports error INVALID_CHUNK_OFFSET
    **/
    PVMFMP4FFParserErrInvalidChunkOffset,

    /**
     When MP4 FF reports error MEMORY_ALLOCATION_FAILED
    **/
    PVMFMP4FFParserErrMemoryAllocationFailed,

    /**
     When MP4 FF reports error ZERO_OR_NEGATIVE_ATOM_SIZE
    **/
    PVMFMP4FFParserErrZeroOrNegativeAtomSize,

    /**
     When MP4 FF reports error NO_MEDIA_TRACKS_IN_FILE
    **/
    PVMFMP4FFParserErrNoMediaTracksInFile,

    /**
     When MP4 FF reports error NO_META_DATA_FOR_MEDIA_TRACKS
    **/
    PVMFMP4FFParserErrNoMetadataForMediaTracks,

    /**
     When MP4 FF reports error MEDIA_DATA_NOT_SELF_CONTAINED
    **/
    PVMFMP4FFParserErrMediaDataNotSelfContained,

    /**
     When MP4 FF reports error FILE_VERSION_NOT_SUPPORTED
    **/
    PVMFMP4FFParserErrFileVersionNotSupported,

    /**
     When MP4 FF reports error TRACK_VERSION_NOT_SUPPORTED
    **/
    PVMFMP4FFParserErrTrackVersioNotSupported,

    /**
     When MP4 FF reports error FILE_PSEUDO_STREAMABLE
    **/
    PVMFMP4FFParserErrPseudostreamableFile,

    /**
     When MP4 FF reports error FILE_NOT_PSEUDO_STREAMABLE
    **/
    PVMFMP4FFParserErrNotPseudostreamableFile,

    /**
     When MP4 FF reports error DUPLICATE_FILE_TYPE_ATOMS
    **/
    PVMFMP4FFParserErrDuplicateFileTypeAtoms,

    /**
     When MP4 FF reports error UNSUPPORTED_FILE_TYPE
    **/
    PVMFMP4FFParserErrUnsupportedFileType,

    /**
     When MP4 FF reports error FILE_TYPE_ATOM_NOT_FOUND
    **/
    PVMFMP4FFParserErrFileTypeAtomNotFound,

    /**
     When MP4 FF reports error ATOM_VERSION_NOT_SUPPORTED
    **/
    PVMFMP4FFParserErrAtomVersionNotSupported,

    /**
     When MP4 FF reports error READ_AVC_SAMPLE_ENTRY_FAILED
    **/
    PVMFMP4FFParserErrReadAVCSampleEntryFailed,

    /**
     When MP4 FF reports error READ_AVC_CONFIG_BOX_FAILED
    **/
    PVMFMP4FFParserErrReadAVCConfigBoxFailed,

    /**
     When MP4 FF reports error READ_MPEG4_BITRATE_BOX_FAILED
    **/
    PVMFMP4FFParserErrReadMPEG4BitRateBoxFailed,

    /**
     Placeholder for the last PV MP4 FF parser error event
     **/
    PVMFMP4FFParserErrLast = 8191
} PVMFMP4FFParserErrorEventType;

/**
 * An enumeration of informational event types from PV MP4 FF parser node
 **/
typedef enum
{
    /**
     MP4 file is pseudostreamable
     **/
    PVMFMP4FFParserInfoPseudostreamableFile	= 8192,

    /**
     MP4 file is non-pseudostreamable
     **/
    PVMFMP4FFParserInfoNotPseudostreamableFile,

    /**
     Placeholder for the last PV MP4 FF parser informational event
     **/
    PVMFMP4FFParserInfoLast = 10000

} PVMFMP4FFParserInformationalEventType;

#endif // PVMF_MP4FFPARSER_EVENTS_H_INCLUDED


