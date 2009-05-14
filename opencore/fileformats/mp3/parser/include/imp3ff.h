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
// -*- c++ -*-
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//                 M P 3   F I L E   P A R S E R

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


/**
 *  @file imp3ff.h
 *  @brief This file defines the MP3 File Format Interface Definition.
 *  It initializes and maintains the MP3 File Format Library
 */

#ifndef IMP3FF_H_INCLUDED
#define IMP3FF_H_INCLUDED


//----------------------------------------------------------------------
// Include Files
//----------------------------------------------------------------------

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_String_H_INCLUDED
#include "oscl_string.h"

#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef PVFILE_H_INCLUDED
#include "pvfile.h"
#endif

#ifndef PV_ID3_PARCOM_H_INCLUDED
#include "pv_id3_parcom.h"
#endif
// Header files reqd for multiple sample retrieval api
#ifndef OSCL_MEDIA_DATA_H
#include "oscl_media_data.h"
#endif
#ifndef PV_GAU_H_
#include "pv_gau.h"
#endif

// Header files for pvmf metadata handling
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PVMF_META_DATA_H_INCLUDED
#include "pvmf_meta_data_types.h"
#endif
#ifndef PVMI_KVP_INCLUDED
#include "pvmi_kvp.h"
#endif



//----------------------------------------------------------------------
// Global Type Declarations
//----------------------------------------------------------------------

typedef struct _MP3ContentFormatTypeTag
{
    uint32 Bitrate;
    uint32 SamplingRate;
    uint32 NumberOfChannels;
    uint32 NumberOfFrames;
    uint32 FrameSize;
    uint32 FileSizeInBytes;
    uint32 FrameSizeUnComp;
    uint32 ChannelMode;  //0: Stereo 1: Joint Ch. Stereo 2: Dual  Ch. Stereo 3: Mono
} MP3ContentFormatType;

typedef enum
{
    MP3_CHANNEL_MODE_STEREO = 0,
    MP3_CHANNEL_MODE_JOINT_CHANNEL_STEREO = 1,
    MP3_CHANNEL_MODE_DUAL_CHANNEL_STEREO = 2,
    MP3_CHANNEL_MODE_MONO = 3
}MP3ChannelMode;

typedef enum
{
    MP3_ERROR_UNKNOWN = 0,
    MP3_SUCCESS = 1,
    MP3_END_OF_FILE = 2,
    MP3_CRC_ERR = 3,
    MP3_FILE_READ_ERR = 4,
    MP3_FILE_HDR_READ_ERR = 5,
    MP3_FILE_HDR_DECODE_ERR = 6,
    MP3_FILE_XING_HDR_ERR = 7,
    MP3_FILE_VBRI_HDR_ERR = 8,
    MP3_ERR_NO_MEMORY = 9,
    MP3_NO_SYNC_FOUND = 10,
    MP3_FILE_OPEN_ERR = 11,
    /* PD related Error values*/
    MP3_ERROR_UNKNOWN_OBJECT = 12,
    MP3_FILE_OPEN_FAILED = 13,
    MP3_INSUFFICIENT_DATA = 14,
    MP3_METADATA_NOTPARSED = 15,
    /* Duration related Info value*/
    MP3_DURATION_PRESENT = 16
}MP3ErrorType;

//----------------------------------------------------------------------
// Forward Class Declarations
//----------------------------------------------------------------------
class MP3Parser;
class PVMFCPMPluginAccessInterfaceFactory;

/**
 *  @brief The IMpeg3File Class is the class that will construct and maintain all the
 *  necessary data structures to be able to render a valid MP3 file to disk.
 *
 */

class IMpeg3File
{
    public:
        /**
        * @brief Constructor
        *
        * @param filename MP3 filename
        * @param bSuccess Result of operation: true=successful, false=failed
        * @param fileServSession Pointer to opened file server session. Used when opening
        * and reading the file on certain operating systems.
        * @returns None
        */
        OSCL_IMPORT_REF  IMpeg3File(OSCL_wString& filename, MP3ErrorType &bSuccess, Oscl_FileServer* fileServSession = NULL, PVMFCPMPluginAccessInterfaceFactory*aCPM = NULL, OsclFileHandle*aHandle = NULL, bool enableCRC = true);

        /**
        * @brief Constructor
        *
        * @param bSuccess Result of operation: true=successful, false=failed
        * @returns None
        */
        OSCL_IMPORT_REF IMpeg3File(MP3ErrorType &bSuccess);

        /**
        * @brief Destructor
        *
        * @param None
        * @returns None
        */
        OSCL_IMPORT_REF ~IMpeg3File();

        /**
        * @brief Returns the parse status of the file
        *
        * @param None
        * @returns Result of operation
        */
        OSCL_IMPORT_REF MP3ErrorType ParseMp3File();

        /**
        * @brief Returns the configuration details of the file
        *
        * @param mp3Config Data structure that will contain config info
        * @returns Result of operation: 1=success; 0=fail
        */
        OSCL_IMPORT_REF MP3ErrorType GetConfigDetails(MP3ContentFormatType &mp3Config);

        /**
        * @brief Resets the parser variables so playback can be restarted at the
        * specified time.
        *
        * @param timestamp value as where to start repositioning to
        * @returns Result of operation: 1=success; 0=fail
        */
        OSCL_IMPORT_REF uint32 ResetPlayback(uint32 timestamp = 0);

        /**
        * @brief Attempts to read in the number of audio frames specified by n
        *
        * @param n Requested number of frames to be read from file
        * @param pgau Frame information structure of type GAU
        * @returns Result of operation: 1=success; 0=fail
        */
        OSCL_IMPORT_REF int32  GetNextBundledAccessUnits(uint32 *n, GAU *pgau, MP3ErrorType &err);

        /**
        * @brief Attempts to peek into the number of audio frames specified by n
        *
        * @param n Requested number of frames to be read from file
        * @param mInfo Frame information structure of type MediaMetaInfo
        * @returns Result of operation: 1=success; 0=fail
        */
        OSCL_IMPORT_REF int32  PeekNextBundledAccessUnits(uint32 *n, MediaMetaInfo *mInfo);

        /**
        * @brief Reads the next frame from the file
        *
        * @param buf Buffer containing the frame read
        * @param size Size of the buffer
        * @param framesize Size of the frame data read
        * @param timestamp Timestamp of the frame
        * @returns Result of operation: true=success; false=fail
        */
        OSCL_IMPORT_REF MP3ErrorType GetNextMediaSample(uint8 *buf, uint32 size, uint32& framesize, uint32& timestamp);

        /**
        * @brief Returns the timestamp of the current frame
        *
        * @param None
        * @returns Timestamp
        */
        OSCL_IMPORT_REF uint32 GetTimestampForCurrentSample();

        /**
        * @brief Moves the file pointer to the specified timestamp
        *
        * @param timestamp Time to seek to
        * @returns Timestamp seeked to. If timestamp is past end of clip, timestamp is 0.
        */
        OSCL_IMPORT_REF uint32  SeekToTimestamp(uint32 timestamp);

        /**
        * @brief Queries the seek point file positiion and timestamp corresponding to the specified timestamp
        *
        * @param timestamp Time to seek to. Will be set to the actual timestamp.
        * @returns File position closest to the specified timestamp. If timestamp is past end of clip, file position is set to 0.
        */
        OSCL_IMPORT_REF uint32  SeekPointFromTimestamp(uint32& timestamp);

        /**
        * @brief Queries the offset corresponding to the specified timestamp
        *
        * @param timestamp Time to seek to. Will be set to the actual timestamp.
        * @returns offset
        */
        OSCL_IMPORT_REF uint32  GetFileOffsetForAutoResume(uint32& timestamp);

        /**
        * @brief Returns whether random access is allowed or not.
        *
        * @param None
        * @returns true=allowed, false=not allowed
        */
        OSCL_IMPORT_REF uint8  RandomAccessDenied();

        /**
        * @brief Returns the number of tracks in the file.
        *
        * @param None
        * @returns Number of tracks
        */
        OSCL_IMPORT_REF int32 GetNumTracks();

        /**
        * @brief Returns the timescale used.
        *
        * @param None
        * @returns Timescale
        */
        OSCL_IMPORT_REF uint32 GetTimescale() const;

        /**
        * @brief Returns the duration of the clip.
        *
        * @param None
        * @returns Duration
        */
        OSCL_IMPORT_REF uint32 GetDuration() const;

        /**
        * @brief Returns the approximate duration of downloaded data.
        *
        * @param aFileSize, aNPTInMS
        * @returns aNPTInMS
        */
        OSCL_IMPORT_REF int32 ConvertSizeToTime(uint32 aFileSize, uint32& aNPTInMS) const;

        /**
        * @brief Returns the number of frames in the clip.
        *
        * @param None
        * @returns Number of frames
        */
        OSCL_IMPORT_REF int32 GetNumSampleEntries();

        /**
        * @brief Returns the mime type of the clip.
        *
        * @param None
        * @returns Mime type
        */
        OSCL_IMPORT_REF OSCL_wHeapString<OsclMemAllocator>  GetMIMEType();

        /**
        * @brief Returns the maximum buffer size used.
        *
        * @param None
        * @returns Buffer size
        */
        OSCL_IMPORT_REF int32 GetMaxBufferSizeDB();

        /**
        * @brief Returns the content of decoder specific info.
        *
        * @param None
        * @returns Decoder specific info
        */
        OSCL_IMPORT_REF uint8 const *GetDecoderSpecificInfoContent() const;

        /**
        * @brief Returns the size of decoder specific info.
        *
        * @param None
        * @returns Decoder specific info size
        */
        OSCL_IMPORT_REF uint32 GetDecoderSpecificInfoSize();

        /**
        * @brief Returns the number of available metadata keys. Subset is returned if a query string is specified
        *
        * @param aQueryString A query string for the keys to select a subset of the all available keys
        *
        * @returns Number of available metadata keys
        */
        OSCL_IMPORT_REF uint32 GetNumMetadataKeys(char* aQueryString = NULL);

        /**
        * @brief Returns the number of available metadata values based on the key list provided
        *
        * @param aKeyList A list of key strings to return the number of values
        *
        * @returns Number of available metadata values
        */
        OSCL_IMPORT_REF uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList);

        /**
        * @brief Returns a list of the available metadata keys.
        *
        * @param aKeyList Reference to a keylist to be filled in by this method.
        * @param starting_index Zero based index of which keys to get.
        * @param max_entries Max # of entries this method should write to the key list.
        * @returns None
        */
        OSCL_IMPORT_REF PVMFStatus GetMetadataKeys(PVMFMetadataList& aKeyList, uint32 aStartingKeyIndex, int32 aMaxKeyEntries, char* aQueryString);

        /**
        * @brief Returns the metadata values corresponding to the specified keys.
        *
        * @param aKeyList Reference to a keylist specifying keys for which the values are being requested.
        * @param aValueList Reference to value list which this method will fill in.
        * @param starting_index
        */
        OSCL_IMPORT_REF PVMFStatus GetMetadataValues(PVMFMetadataList& aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 aStartingValueIndex, int32 aMaxValueEntries);

        /**
        * @brief Releases memory allocated for the metadata values
        *
        * @param aValueKVP Reference to metadata value stored in a KVP to release memory
        */
        OSCL_IMPORT_REF PVMFStatus ReleaseMetadataValue(PvmiKvp& aValueKVP);

        /**
        * @brief Requests call back for parser node when data stream is out of data
        *
        * @param aObserver - observer, which would receive the call back once
        *					the file has been downloaded till provided offset.
        * @param aFileOffset - offset for which data is not available.
        * @param aContextData - context data
        */
        OSCL_IMPORT_REF  MP3ErrorType RequestReadCapacityNotification(PvmiDataStreamObserver& aObserver,
                uint32 aFileOffset,
                OsclAny* aContextData = NULL);
        /**
        * @brief Retrieves the Metadata size in the mp3 clip
        *
        * @param reference to metadatasize
        * @returns error type.
        */
        OSCL_IMPORT_REF MP3ErrorType GetMetadataSize(uint32& aMetaDataSize);

        /**
        * @brief Retrieves minimum bytes required for getting the config info
        *
        * @param
        * @returns byte size of firstframe and id3 tags.
        */
        OSCL_IMPORT_REF uint32 GetMinBytesRequired(bool aNextBytes = false);

        /**
        * @brief Sets the file size to the parser
        *
        * @param aFileSize
        * @returns error type.
        */
        OSCL_IMPORT_REF MP3ErrorType SetFileSize(const uint32 aFileSize);

        /**
        * @brief Verifies if the supplied file is valid mp3 file
        *
        * @param aFileName
        * @param aCPMAccessFactory
        * @param aEnableCRC - whether or not to enable crc check
        */
        OSCL_IMPORT_REF MP3ErrorType IsMp3File(OSCL_wString& aFileName,
                                               PVMFCPMPluginAccessInterfaceFactory *aCPMAccessFactory,
                                               uint32 aInitSearchFileSize);

        OSCL_EXPORT_REF MP3ErrorType ScanMP3File(uint32 aFramesToScan);

    private:
        OsclAny* AllocateKVPKeyArray(int32& leavecode, PvmiKvpValueType aValueType, int32 aNumElements);
        int32 PushKVPValue(PvmiKvp aKVP, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList);
        int32 PushKVPKey(const char* aString, PVMFMetadataList& aKeyList);
        int32 PushKVPKey(OSCL_HeapString<OsclMemAllocator>& aString, Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator>& aKeyList);

        MP3Parser* pMP3Parser;
        PVFile iMP3File;
        bool iEnableCrcCalc;
        PVFile iScanFP;
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iAvailableMetadataKeys;
};


#endif // #ifndef IMP3FF_H_INCLUDED
