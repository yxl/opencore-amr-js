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
//
//                 M P 3   F I L E   P A R S E R
//
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


/**
 *  @file imp3ff.cpp
 *  @brief This file contains the implementation of the MP3 File Format
 *  interface. It initializes and maintains the MP3 File Format Library
 */

//----------------------------------------------------------------------
// Include Files
//----------------------------------------------------------------------

#include "imp3ff.h"
#include "mp3fileio.h"
#include "mp3utils.h"
#include "mp3parser.h"
#include "oscl_mem.h"
#include "oscl_utf8conv.h"
#include "pvmi_kvp_util.h"
#include "pvmi_kvp.h"
#include "pvmf_format_type.h"
#include "pv_mime_string_utils.h"
// Constant character strings for metadata keys
static const char PVMP3METADATA_TITLE_KEY[] = "title";
static const char PVMP3METADATA_ARTIST_KEY[] = "artist";
static const char PVMP3METADATA_ALBUM_KEY[] = "album";
static const char PVMP3METADATA_YEAR_KEY[] = "year";
static const char PVMP3METADATA_COMMENT_KEY[] = "comment";
static const char PVMP3METADATA_COPYRIGHT_KEY[] = "copyright";
static const char PVMP3METADATA_GENRE_KEY[] = "genre";
static const char PVMP3METADATA_TRACKNUMBER_KEY[] = "tracknumber";
static const char PVMP3METADATA_DURATION_KEY[] = "duration";
static const char PVMP3METADATA_DURATION_FROM_METADATA_KEY[] = "duration-from-metadata";
static const char PVMP3METADATA_NUMTRACKS_KEY[] = "num-tracks";
static const char PVMP3METADATA_TRACKINFO_BITRATE_KEY[] = "track-info/bit-rate";
static const char PVMP3METADATA_TRACKINFO_SAMPLERATE_KEY[] = "track-info/sample-rate";
static const char PVMP3METADATA_TRACKINFO_AUDIO_FORMAT_KEY[] = "track-info/audio/format";
static const char PVMP3METADATA_TRACKINFO_AUDIO_CHANNELS_KEY[] = "track-info/audio/channels";
static const char PVMP3METADATA_TRACKINFO_AUDIO_CHANNEL_MODE_KEY[] = "track-info/audio/channel-mode";
static const char PVMP3METADATA_BITRATE_KEY[] = "bit-rate";
static const char PVMP3METADATA_SAMPLERATE_KEY[] = "sample-rate";
static const char PVMP3METADATA_FORMAT_KEY[] = "format";
static const char PVMP3METADATA_CHANNELS_KEY[] = "channels";
static const char PVMP3METADATA_CHANNEL_MODE_KEY[] = "channel-mode";
static const char PVMP3METADATA_RANDOM_ACCESS_DENIED_KEY[] = "random-access-denied";
static const char PVMP3METADATA_SEMICOLON[] = ";";
static const char PVMP3METADATA_CHARENCUTF8[] = "char-encoding=UTF8";
static const char PVMP3METADATA_CHARENCUTF16BE[] = "char-encoding=UTF16BE";
static const char PVMP3METADATA_FORMATID3V1[] = "format=id3v1";
static const char PVMP3METADATA_FORMATID3V11[] = "format=id3v1.1";
static const char PVMP3METADATA_TIMESCALE1000[] = "timescale=1000";
static const char PVMP3METADATA_INDEX0[] = "index=0";
static const char PVMP3METADATA_UNKNOWN[] = "unknown";
static const char PVMP3METADATA_COMPUTE[] = "compute=true";

// Use default DLL entry point for Symbian
#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()

//----------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Type Declarations
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Global Constant Definitions
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Global Data Definitions
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Static Variable Definitions
//----------------------------------------------------------------------


OSCL_EXPORT_REF IMpeg3File::IMpeg3File(OSCL_wString& filename, MP3ErrorType &bSuccess, Oscl_FileServer* fileServSession, PVMFCPMPluginAccessInterfaceFactory*aCPM, OsclFileHandle*aFileHandle, bool enableCRC) :
        pMP3Parser(NULL)
{
    bSuccess = MP3_SUCCESS;

    // Initialize the metadata related variables
    iAvailableMetadataKeys.reserve(14);
    iAvailableMetadataKeys.clear();

    iEnableCrcCalc = enableCRC;
    // Open the specified MP3 file
    iMP3File.SetCPM(aCPM);
    iMP3File.SetFileHandle(aFileHandle);
    if (iMP3File.Open(filename.get_cstr(), (Oscl_File::MODE_READ | Oscl_File::MODE_BINARY), *fileServSession) != 0)
    {
        bSuccess = MP3_FILE_OPEN_ERR;
        return;
    }

    if (!aCPM)
    {
        iScanFP.SetCPM(aCPM);
        iScanFP.SetFileHandle(aFileHandle);
        if (iScanFP.Open(filename.get_cstr(), (Oscl_File::MODE_READ | Oscl_File::MODE_BINARY), *fileServSession) != 0)
        {
            bSuccess = MP3_FILE_OPEN_ERR;
            return;
        }
    }

    int32 leavecode = OsclErrNone;
    OSCL_TRY(leavecode, pMP3Parser = OSCL_NEW(MP3Parser, (&iMP3File)););
    if (pMP3Parser && OsclErrNone == leavecode)
        bSuccess = MP3_SUCCESS;
    else
        bSuccess = MP3_ERROR_UNKNOWN;
}

OSCL_EXPORT_REF IMpeg3File::IMpeg3File(MP3ErrorType &bSuccess): pMP3Parser(NULL)
{
    int32 leavecode = OsclErrNone;
    OSCL_TRY(leavecode, pMP3Parser = OSCL_NEW(MP3Parser, ()););
    if (pMP3Parser && OsclErrNone == leavecode)
        bSuccess = MP3_SUCCESS;
    else
        bSuccess = MP3_ERROR_UNKNOWN;
}

OSCL_EXPORT_REF IMpeg3File::~IMpeg3File()
{
    iAvailableMetadataKeys.clear();

    if (pMP3Parser != NULL)
    {
        OSCL_DELETE(pMP3Parser);
        pMP3Parser = NULL;
    }

    if (iScanFP.IsOpen())
    {
        iScanFP.Close();
    }
    if (iMP3File.IsOpen())
    {
        iMP3File.Close();
    }
}

OSCL_EXPORT_REF MP3ErrorType IMpeg3File::ParseMp3File()
{
    MP3ErrorType mp3Err = MP3_SUCCESS;

    // Parse the mp3 clip
    mp3Err = pMP3Parser->ParseMP3File(&iMP3File, iEnableCrcCalc);
    if (mp3Err == MP3_INSUFFICIENT_DATA)
    {
        // not enough data was available to parse the clip
        return mp3Err;
    }
    else if (mp3Err != MP3_SUCCESS)
    {
        // some other error occured while parsing the clip.
        // parsing can not proceed further
        OSCL_DELETE(pMP3Parser);
        pMP3Parser = NULL;
        iMP3File.Close();
        return mp3Err;
    }
    else
    {
        // parsing of clip was successful, id3 frames can now be fetched
        PvmiKvpSharedPtrVector id3Frames;
        pMP3Parser->GetMetaData(id3Frames);

        // Populate metadata key list vector
        int32 leavecode = OsclErrNone;
        for (uint32 p = 0;p < id3Frames.size();p++)
        {
            OSCL_HeapString<OsclMemAllocator> keystr((const char *)((*id3Frames[p]).key), oscl_strlen((const char *)((*id3Frames[p]).key)));
            leavecode = PushKVPKey(keystr, iAvailableMetadataKeys);
            if (OsclErrNone != leavecode)
            {
                return MP3_ERR_NO_MEMORY;
            }

        }

        bool metadataDuration = true;
        if (pMP3Parser->GetDuration(metadataDuration) > 0)
        {
            leavecode = OsclErrNone;
            leavecode = PushKVPKey(PVMP3METADATA_DURATION_FROM_METADATA_KEY, iAvailableMetadataKeys);
            if (OsclErrNone != leavecode)
            {
                return MP3_ERR_NO_MEMORY;
            }
        }

        // Following keys are available when the MP3 file has been parsed
        leavecode = OsclErrNone;
        leavecode = PushKVPKey(PVMP3METADATA_DURATION_KEY, iAvailableMetadataKeys);
        if (OsclErrNone != leavecode)
        {
            return MP3_ERR_NO_MEMORY;
        }

        leavecode = OsclErrNone;
        leavecode = PushKVPKey(PVMP3METADATA_RANDOM_ACCESS_DENIED_KEY, iAvailableMetadataKeys);
        if (OsclErrNone != leavecode)
        {
            return MP3_ERR_NO_MEMORY;
        }

        leavecode = OsclErrNone;
        leavecode = PushKVPKey(PVMP3METADATA_NUMTRACKS_KEY, iAvailableMetadataKeys);
        if (OsclErrNone != leavecode)
        {
            return MP3_ERR_NO_MEMORY;
        }

        leavecode = OsclErrNone;
        leavecode = PushKVPKey(PVMP3METADATA_TRACKINFO_AUDIO_FORMAT_KEY, iAvailableMetadataKeys);
        if (OsclErrNone != leavecode)
        {
            return MP3_ERR_NO_MEMORY;
        }

        MP3ContentFormatType mp3info;
        if (GetConfigDetails(mp3info) == MP3_SUCCESS)
        {
            if (mp3info.Bitrate > 0)
            {
                leavecode = OsclErrNone;
                leavecode = PushKVPKey(PVMP3METADATA_TRACKINFO_BITRATE_KEY, iAvailableMetadataKeys);
                if (OsclErrNone != leavecode)
                {
                    return MP3_ERR_NO_MEMORY;
                }
            }
            if (mp3info.SamplingRate > 0)
            {
                leavecode = OsclErrNone;
                leavecode = PushKVPKey(PVMP3METADATA_TRACKINFO_SAMPLERATE_KEY, iAvailableMetadataKeys);
                if (OsclErrNone != leavecode)
                {
                    return MP3_ERR_NO_MEMORY;
                }
            }
            if (mp3info.NumberOfChannels > 0)
            {
                leavecode = OsclErrNone;
                leavecode = PushKVPKey(PVMP3METADATA_TRACKINFO_AUDIO_CHANNELS_KEY, iAvailableMetadataKeys);
                if (OsclErrNone != leavecode)
                {
                    return MP3_ERR_NO_MEMORY;
                }
            }
            // valid channel mode is present
            if (mp3info.ChannelMode <= MP3_CHANNEL_MODE_MONO)
            {
                leavecode = 0;
                leavecode = PushKVPKey(PVMP3METADATA_CHANNEL_MODE_KEY, iAvailableMetadataKeys);
                if (OsclErrNone != leavecode)
                {
                    return MP3_ERR_NO_MEMORY;
                }
            }
        }
    }
    return mp3Err;
}

OSCL_EXPORT_REF MP3ErrorType IMpeg3File::GetConfigDetails(MP3ContentFormatType &mp3Config)
{
    if (pMP3Parser)
    {
        MP3ConfigInfoType mp3ConfigHdr;
        if (pMP3Parser->GetMP3FileHeader(&mp3ConfigHdr))
        {
            mp3Config.Bitrate          = mp3ConfigHdr.BitRate;
            mp3Config.FrameSize        = mp3ConfigHdr.FrameLengthInBytes;
            mp3Config.NumberOfChannels = mp3ConfigHdr.NumberOfChannels;
            mp3Config.SamplingRate     = mp3ConfigHdr.SamplingRate;
            mp3Config.FrameSizeUnComp  = mp3ConfigHdr.FrameSizeUnComp;
            mp3Config.FileSizeInBytes  = pMP3Parser->GetFileSize();
            mp3Config.ChannelMode      = pMP3Parser->GetChannelMode();
            return MP3_SUCCESS;
        }
    }
    return MP3_ERROR_UNKNOWN;
}

OSCL_EXPORT_REF MP3ErrorType IMpeg3File::GetNextMediaSample(uint8 *buf, uint32 size, uint32& framesize, uint32& timestamp)
{
    if (pMP3Parser != NULL)
    {
        return (pMP3Parser->GetNextMediaSample(buf, size, framesize, timestamp));
    }
    else
    {
        return MP3_ERROR_UNKNOWN;
    }
}

OSCL_EXPORT_REF uint32 IMpeg3File::ResetPlayback(uint32 time)
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->SeekToTimestamp(time);
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF int32 IMpeg3File::GetNextBundledAccessUnits(uint32 *n, GAU *pgau, MP3ErrorType &err)
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->GetNextBundledAccessUnits(n, pgau, err);
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF int32 IMpeg3File::PeekNextBundledAccessUnits(uint32 *n, MediaMetaInfo *mInfo)
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->PeekNextBundledAccessUnits(n, mInfo);
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF int32 IMpeg3File::GetNumTracks()
{
    return 1;
}

OSCL_EXPORT_REF uint32 IMpeg3File::GetDuration() const
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->GetDuration();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF int32 IMpeg3File::ConvertSizeToTime(uint32 aFileSize, uint32& aNPTInMS) const
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->ConvertSizeToTime(aFileSize, aNPTInMS);
    }
    else
    {
        return -1;
    }
}
OSCL_EXPORT_REF MP3ErrorType IMpeg3File::GetMetadataSize(uint32 &aSize)
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->GetMetadataSize(aSize);
    }
    return MP3_ERROR_UNKNOWN;
}

OSCL_EXPORT_REF uint32 IMpeg3File::GetMinBytesRequired(bool aNextBytes)
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->GetMinBytesRequired(aNextBytes);
    }
    return 0;
}

OSCL_EXPORT_REF uint32 IMpeg3File::GetTimestampForCurrentSample()
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->GetTimestampForCurrentSample();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint32  IMpeg3File::SeekToTimestamp(uint32 timestamp)
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->SeekToTimestamp(timestamp);
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint32  IMpeg3File::SeekPointFromTimestamp(uint32& timestamp)
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->SeekPointFromTimestamp(timestamp);
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint32  IMpeg3File::GetFileOffsetForAutoResume(uint32& timestamp)
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->GetFileOffsetForAutoResume(timestamp);
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint32 IMpeg3File::GetTimescale() const
{
    return 1000;
}

OSCL_EXPORT_REF uint8 IMpeg3File::RandomAccessDenied()
{
    return false;
}

OSCL_EXPORT_REF int32 IMpeg3File::GetNumSampleEntries()
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->GetSampleCountInFile();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF int32  IMpeg3File::GetMaxBufferSizeDB()
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->GetMaximumDecodeBufferSize();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint8 const * IMpeg3File::GetDecoderSpecificInfoContent() const
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->GetDecoderSpecificInfoContent();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint32 IMpeg3File::GetDecoderSpecificInfoSize()
{
    if (pMP3Parser != NULL)
    {
        return pMP3Parser->GetDecoderSpecificInfoSize();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF MP3ErrorType IMpeg3File::SetFileSize(const uint32 aFileSize)
{
    MP3ErrorType errCode = MP3_ERROR_UNKNOWN;
    if (pMP3Parser != NULL)
    {
        errCode	= pMP3Parser->SetFileSize(aFileSize);
    }
    return errCode;
}

OSCL_EXPORT_REF uint32 IMpeg3File::GetNumMetadataKeys(char* aQueryKeyString)
{
    uint32 num_entries = 0;

    if (aQueryKeyString == NULL)
    {
        // No query key so just return all the available keys
        num_entries = iAvailableMetadataKeys.size();
    }
    else
    {
        // Determine the number of metadata keys based on the query key string provided
        for (uint32 i = 0; i < iAvailableMetadataKeys.size(); i++)
        {
            // Check if the key matches the query key
            if (oscl_strstr(iAvailableMetadataKeys[i].get_cstr(), aQueryKeyString) != NULL)
            {
                num_entries++;
            }
        }
    }

    return num_entries;
}

OSCL_EXPORT_REF uint32 IMpeg3File::GetNumMetadataValues(PVMFMetadataList& aKeyList)
{
    PvmiKvpSharedPtrVector iFrame;

    uint32 numKeys = aKeyList.size();
    if (numKeys == 0)
    {
        aKeyList = iAvailableMetadataKeys;
        numKeys = aKeyList.size();
    }

    uint32 numvalentries = 0;
    for (uint32 lcv = 0; lcv < numKeys; lcv++)
    {
        if (pMP3Parser->IsID3Frame(aKeyList[lcv]))
        {
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_DURATION_KEY) == 0)
        {
            // Duration
            if (pMP3Parser)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;
            }
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_RANDOM_ACCESS_DENIED_KEY) == 0)
        {
            if (pMP3Parser)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;
            }
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_NUMTRACKS_KEY) == 0)
        {
            // Number of tracks
            if (pMP3Parser)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;
            }
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_BITRATE_KEY) == 0 ||
                 oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_TRACKINFO_BITRATE_KEY) == 0)
        {
            // Bitrate
            MP3ContentFormatType mp3info;
            if (GetConfigDetails(mp3info) == MP3_SUCCESS)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;
            }
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_SAMPLERATE_KEY) == 0 ||
                 oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_TRACKINFO_SAMPLERATE_KEY) == 0)
        {
            // Sampling rate
            MP3ContentFormatType mp3info;
            if (GetConfigDetails(mp3info) == MP3_SUCCESS)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;
            }
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_FORMAT_KEY) == 0 ||
                 oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_TRACKINFO_AUDIO_FORMAT_KEY) == 0)
        {
            // Format
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_CHANNELS_KEY) == 0 ||
                 oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_TRACKINFO_AUDIO_CHANNELS_KEY) == 0)
        {
            // Channels
            MP3ContentFormatType mp3info;
            if (GetConfigDetails(mp3info) == MP3_SUCCESS)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;
            }
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_CHANNEL_MODE_KEY) == 0 ||
                 oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_TRACKINFO_AUDIO_CHANNEL_MODE_KEY) == 0)
        {
            // Channel mode
            MP3ContentFormatType mp3info;
            if (GetConfigDetails(mp3info) == MP3_SUCCESS)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;
            }
        }
    }  // End of for loop
    return numvalentries;
}

OSCL_EXPORT_REF PVMFStatus IMpeg3File::GetMetadataKeys(PVMFMetadataList& aKeyList, uint32 aStartingKeyIndex, int32 aMaxKeyEntries, char* aQueryKeyString)
{
    // Check parameters
    if ((aStartingKeyIndex > (iAvailableMetadataKeys.size() - 1)) || aMaxKeyEntries == 0)
    {
        // Invalid starting index and/or max entries
        return PVMFErrArgument;
    }

    // Copy the requested keys
    uint32 num_entries = 0;
    int32 num_added = 0;
    int32 leavecode = OsclErrNone;
    for (uint32 lcv = 0; lcv < iAvailableMetadataKeys.size(); lcv++)
    {
        if (aQueryKeyString == NULL)
        {
            // No query key so this key is counted
            ++num_entries;
            if (num_entries > aStartingKeyIndex)
            {
                // Past the starting index so copy the key
                leavecode = OsclErrNone;
                leavecode = PushKVPKey(iAvailableMetadataKeys[lcv].get_cstr(), aKeyList);
                if (OsclErrNone != leavecode)
                {
                    return PVMFErrNoMemory;
                }
                num_added++;
            }
        }
        else
        {
            // Check if the key matches the query key
            if (oscl_strstr(iAvailableMetadataKeys[lcv].get_cstr(), aQueryKeyString) != NULL)
            {
                // This key is counted
                ++num_entries;
                if (num_entries > aStartingKeyIndex)
                {
                    // Past the starting index so copy the key
                    leavecode = OsclErrNone;
                    leavecode = PushKVPKey(iAvailableMetadataKeys[lcv].get_cstr(), aKeyList);
                    if (OsclErrNone != leavecode)
                    {
                        return PVMFErrNoMemory;
                    }
                    num_added++;
                }
            }
        }

        // Check if max number of entries have been copied
        if (aMaxKeyEntries > 0 && num_added >= aMaxKeyEntries)
        {
            break;
        }
    }

    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus IMpeg3File::GetMetadataValues(PVMFMetadataList& aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
        uint32 aStartingValueIndex, int32 aMaxValueEntries)
{
    uint32 numKeys = aKeyList.size();
    PvmiKvpSharedPtrVector iFrame;
    PVMFMetadataList parsedKeys;
    if (numKeys == 0 || aStartingValueIndex > (numKeys - 1) || aMaxValueEntries == 0)
    {
        return PVMFErrArgument;
    }

    uint32 numvalentries = 0;
    int32 numentriesadded = 0;
    bool gotvalue = false;
    uint32 lcv = 0;
    for (lcv = 0; lcv < numKeys; lcv++)
    {
        //check if this key has already been parsed.
        for (uint32 pks = 0; pks < parsedKeys.size(); pks++)
        {
            if (pv_mime_strcmp(parsedKeys[pks].get_cstr(), aKeyList[lcv].get_cstr()) >= 0)
            {
                gotvalue = true; //key already parsed.
                break;
            }
        }

        if (gotvalue)	//get next key since this key has already been parsed.
        {
            gotvalue = false;
            continue;
        }
        pMP3Parser->GetMetaData(aKeyList[lcv], iFrame);
        if (iFrame.size() > 1) //multiple id3 frames exist for this key.
        {
            parsedKeys.push_back(aKeyList[lcv]);
        }

        while (iFrame.size() > 0)
        {
            PvmiKvp KeyVal;
            KeyVal.key = NULL;
            KeyVal.length = 0;
            char *key = (*(iFrame.back())).key;
            int32 len = (*(iFrame.back())).length;

            ++numvalentries;

            int32 leavecode = OsclErrNone;
            KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, oscl_strlen(key) + 1);

            if (OsclErrNone != leavecode)
            {
                // allocation failed
                return PVMFErrNoMemory;
            }

            oscl_strncpy(KeyVal.key , key, oscl_strlen(key) + 1);
            KeyVal.length = len;
            KeyVal.capacity = (*(iFrame.back())).capacity;

            if (KeyVal.length > 0)
            {
                PvmiKvpValueType ValueType = GetValTypeFromKeyString(key);
                leavecode = OsclErrNone;
                switch (ValueType)
                {

                    case PVMI_KVPVALTYPE_WCHARPTR:
                        KeyVal.value.pWChar_value = (oscl_wchar*) AllocateKVPKeyArray(leavecode, ValueType, len);
                        oscl_strncpy(KeyVal.value.pWChar_value , (*(iFrame.back())).value.pWChar_value, len);
                        KeyVal.value.pWChar_value[len] = 0;
                        break;
                    case PVMI_KVPVALTYPE_CHARPTR:
                        KeyVal.value.pChar_value = (char*) AllocateKVPKeyArray(leavecode, ValueType, len);
                        oscl_strncpy(KeyVal.value.pChar_value , (*(iFrame.back())).value.pChar_value, len);
                        KeyVal.value.pChar_value[len] = 0;
                        break;
                    case PVMI_KVPVALTYPE_UINT8PTR:
                        KeyVal.value.pUint8_value = (uint8*) AllocateKVPKeyArray(leavecode, ValueType, len);
                        oscl_memcpy(KeyVal.value.pUint8_value, (uint8*)(*(iFrame.back())).value.pUint8_value, len);
                        break;
                    case PVMI_KVPVALTYPE_UINT32:
                        KeyVal.value.uint32_value = (*(iFrame.back())).value.uint32_value;
                        break;
                    case PVMI_KVPVALTYPE_KSV:
                        KeyVal.value.key_specific_value = (*(iFrame.back())).value.key_specific_value;
                        break;
                    default:
                        break;
                }

                if (OsclErrNone != leavecode)
                {
                    // allocation failed
                    return PVMFErrNoMemory;
                }

                leavecode = OsclErrNone;
                leavecode = PushKVPValue(KeyVal, aValueList);
                if (OsclErrNone != leavecode)
                {
                    // push kvp failed
                    return PVMFErrNoMemory;
                }
                ++numentriesadded;
            }
            iFrame.pop_back();
        }
    }

    for (lcv = 0; lcv < numKeys; lcv++)
    {

        int32 leavecode = OsclErrNone;
        PvmiKvp KeyVal;
        KeyVal.key = NULL;
        uint32 KeyLen = 0;

        if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP3METADATA_DURATION_KEY))
        {
            // Duration
            if (pMP3Parser)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > aStartingValueIndex)
                {
                    uint32 duration = 0;
                    // decision for walking the entire file in order to calculate
                    // clip duration is made here. currently complete file is scanned
                    // when compute flag is not present , Compute clip duration by default
                    duration = pMP3Parser->GetDuration();

                    if (duration > 0)
                    {
                        KeyLen = oscl_strlen(PVMP3METADATA_DURATION_KEY) + 1; // for "duration;"
                        KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                        KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32;"
                        KeyLen += oscl_strlen(PVMP3METADATA_TIMESCALE1000) + 1; // for "timescale=1000" and NULL terminator

                        // Allocate memory for the string
                        leavecode = OsclErrNone;
                        KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);
                        if (OsclErrNone == leavecode)
                        {
                            // Copy the key string
                            oscl_strncpy(KeyVal.key, PVMP3METADATA_DURATION_KEY, oscl_strlen(PVMP3METADATA_DURATION_KEY) + 1);
                            oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                            oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                            oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                            oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                            oscl_strncat(KeyVal.key, PVMP3METADATA_TIMESCALE1000, oscl_strlen(PVMP3METADATA_TIMESCALE1000));
                            KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                            // Copy the value
                            KeyVal.value.uint32_value = pMP3Parser->GetDuration();
                            // Set the length and capacity
                            KeyVal.length = 1;
                            KeyVal.capacity = 1;
                        }
                        else
                        {
                            // Memory allocation failed
                            KeyVal.key = NULL;
                            break;
                        }
                    }
                    else
                    {
                        KeyLen = oscl_strlen(PVMP3METADATA_DURATION_KEY) + 1; // for "duration;"
                        KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                        KeyLen += oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR) + 1; // for "char*;"

                        // Allocate memory for the string
                        leavecode = OsclErrNone;
                        KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);

                        if (OsclErrNone == leavecode)
                        {
                            oscl_strncpy(KeyVal.key, PVMP3METADATA_DURATION_KEY, oscl_strlen(PVMP3METADATA_DURATION_KEY) + 1);
                            oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                            oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                            oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR));

                            KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;

                        }
                        else
                        {
                            // Memory allocation failed
                            KeyVal.key = NULL;
                            break;
                        }

                        uint32 valuelen = oscl_strlen(_STRLIT_CHAR(PVMP3METADATA_UNKNOWN)) + 1; // Add value string plus one for NULL terminator
                        leavecode = OsclErrNone;
                        KeyVal.value.pChar_value = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, valuelen);

                        if (OsclErrNone == leavecode)
                        {
                            // Copy the value
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMP3METADATA_UNKNOWN), valuelen);
                            KeyVal.value.pChar_value[valuelen-1] = NULL_TERM_CHAR;
                            // Set the length and capacity
                            KeyVal.length = valuelen;
                            KeyVal.capacity = valuelen;
                        }
                        else
                        {
                            // Memory allocation failed
                            KeyVal.key = NULL;
                            break;
                        }
                    }
                }
            }
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP3METADATA_RANDOM_ACCESS_DENIED_KEY))
        {
            // Duration
            if (pMP3Parser)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > aStartingValueIndex)
                {
                    KeyLen = oscl_strlen(PVMP3METADATA_RANDOM_ACCESS_DENIED_KEY) + 1; // for "random-access-denied;"
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_BOOL_STRING_CONSTCHAR) + 1; // for "bool;"

                    // Allocate memory for the string
                    leavecode = OsclErrNone;
                    KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);
                    if (OsclErrNone == leavecode)
                    {
                        // Copy the key string
                        oscl_strncpy(KeyVal.key, PVMP3METADATA_RANDOM_ACCESS_DENIED_KEY, oscl_strlen(PVMP3METADATA_RANDOM_ACCESS_DENIED_KEY) + 1);
                        oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_BOOL_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                        KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                        // Copy the value
                        KeyVal.value.bool_value = pMP3Parser->GetDuration() > 0 ? false : true;
                        // Set the length and capacity
                        KeyVal.length = 1;
                        KeyVal.capacity = 1;
                    }
                    else
                    {
                        // Memory allocation failed
                        KeyVal.key = NULL;
                        break;
                    }
                }
            }
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP3METADATA_DURATION_FROM_METADATA_KEY))
        {
            // Duration
            if (pMP3Parser)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > aStartingValueIndex)
                {
                    uint32 duration = 0;
                    // decision for walking the entire file in order to calculate
                    // clip duration is made here. currently complete file is scanned
                    // when compute flag is not present , Compute clip duration by default
                    bool metadataDuration = true;
                    duration = pMP3Parser->GetDuration(metadataDuration);

                    if (duration > 0)
                    {
                        KeyLen = oscl_strlen(PVMP3METADATA_DURATION_FROM_METADATA_KEY) + 1; // for "duration;"
                        KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                        KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32;"
                        KeyLen += oscl_strlen(PVMP3METADATA_TIMESCALE1000) + 1; // for "timescale=1000" and NULL terminator

                        // Allocate memory for the string
                        leavecode = OsclErrNone;
                        KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);
                        if (OsclErrNone == leavecode)
                        {
                            // Copy the key string
                            oscl_strncpy(KeyVal.key, PVMP3METADATA_DURATION_FROM_METADATA_KEY, oscl_strlen(PVMP3METADATA_DURATION_KEY) + 1);
                            oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                            oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                            oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                            oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                            oscl_strncat(KeyVal.key, PVMP3METADATA_TIMESCALE1000, oscl_strlen(PVMP3METADATA_TIMESCALE1000));
                            KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                            // Copy the value
                            KeyVal.value.uint32_value = pMP3Parser->GetDuration();
                            // Set the length and capacity
                            KeyVal.length = 1;
                            KeyVal.capacity = 1;
                        }
                        else
                        {
                            // Memory allocation failed
                            KeyVal.key = NULL;
                            break;
                        }
                    }
                    else
                    {
                        KeyLen = oscl_strlen(PVMP3METADATA_DURATION_FROM_METADATA_KEY) + 1; // for "duration;"
                        KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                        KeyLen += oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR) + 1; // for "char*;"

                        // Allocate memory for the string
                        leavecode = OsclErrNone;
                        KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);


                        if (OsclErrNone == leavecode)
                        {
                            oscl_strncpy(KeyVal.key, PVMP3METADATA_DURATION_FROM_METADATA_KEY, oscl_strlen(PVMP3METADATA_DURATION_KEY) + 1);
                            oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                            oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                            oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR));

                            KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                        }
                        else
                        {
                            // Memory allocation failed
                            KeyVal.key = NULL;
                            break;
                        }

                        uint32 valuelen = oscl_strlen(_STRLIT_CHAR(PVMP3METADATA_UNKNOWN)) + 1; // Add value string plus one for NULL terminator
                        leavecode = OsclErrNone;
                        KeyVal.value.pChar_value = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, valuelen);

                        if (OsclErrNone == leavecode)
                        {
                            // Copy the value
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMP3METADATA_UNKNOWN), valuelen);
                            KeyVal.value.pChar_value[valuelen-1] = NULL_TERM_CHAR;
                            // Set the length and capacity
                            KeyVal.length = valuelen;
                            KeyVal.capacity = valuelen;
                        }
                        else
                        {
                            // Memory allocation failed
                            KeyVal.key = NULL;
                            break;
                        }
                    }
                }
            }
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_NUMTRACKS_KEY) == 0)
        {
            // Number of tracks
            if (pMP3Parser)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > aStartingValueIndex)
                {
                    KeyLen = oscl_strlen(PVMP3METADATA_NUMTRACKS_KEY) + 1; // for "num-tracks;"
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                    // Allocate memory for the string
                    leavecode = OsclErrNone;
                    KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);

                    if (OsclErrNone == leavecode)
                    {
                        // Copy the key string
                        oscl_strncpy(KeyVal.key, PVMP3METADATA_NUMTRACKS_KEY, oscl_strlen(PVMP3METADATA_NUMTRACKS_KEY) + 1);
                        oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                        KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                        // Copy the value
                        KeyVal.value.uint32_value = 1; // Number of tracks supported in PV MP3 parser would always be 1
                        // Set the length and capacity
                        KeyVal.length = 1;
                        KeyVal.capacity = 1;
                    }
                    else
                    {
                        // Memory allocation failed
                        KeyVal.key = NULL;
                        break;
                    }
                }
            }
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_BITRATE_KEY) == 0 ||
                 oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_TRACKINFO_BITRATE_KEY) == 0)
        {
            // Bitrate
            MP3ContentFormatType mp3info;
            if (GetConfigDetails(mp3info) == MP3_SUCCESS)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > aStartingValueIndex)
                {
                    KeyLen = oscl_strlen(PVMP3METADATA_TRACKINFO_BITRATE_KEY) + 1; // for "track-info/bitrate;"
                    KeyLen += oscl_strlen(PVMP3METADATA_INDEX0) + 1; // for "index=0;"
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                    // Allocate memory for the string
                    leavecode = OsclErrNone;
                    KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);

                    if (OsclErrNone == leavecode)
                    {
                        // Copy the key string
                        oscl_strncpy(KeyVal.key, PVMP3METADATA_TRACKINFO_BITRATE_KEY, oscl_strlen(PVMP3METADATA_TRACKINFO_BITRATE_KEY) + 1);
                        oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMP3METADATA_INDEX0, oscl_strlen(PVMP3METADATA_INDEX0));
                        oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                        KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                        // Copy the value
                        KeyVal.value.uint32_value = mp3info.Bitrate;
                        // Set the length and capacity
                        KeyVal.length = 1;
                        KeyVal.capacity = 1;
                    }
                    else
                    {
                        // Memory allocation failed
                        KeyVal.key = NULL;
                        break;
                    }
                }
            }
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_SAMPLERATE_KEY) == 0 ||
                 oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_TRACKINFO_SAMPLERATE_KEY) == 0)
        {
            // Sampling rate
            MP3ContentFormatType mp3info;
            if (GetConfigDetails(mp3info) == MP3_SUCCESS)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > aStartingValueIndex)
                {
                    KeyLen = oscl_strlen(PVMP3METADATA_TRACKINFO_SAMPLERATE_KEY) + 1; // for "track-info/samplingrate;"
                    KeyLen += oscl_strlen(PVMP3METADATA_INDEX0) + 1; // for "index=0;"
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                    // Allocate memory for the string
                    leavecode = 0;
                    KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);

                    if (OsclErrNone == leavecode)
                    {
                        // Copy the key string
                        oscl_strncpy(KeyVal.key, PVMP3METADATA_TRACKINFO_SAMPLERATE_KEY, oscl_strlen(PVMP3METADATA_TRACKINFO_SAMPLERATE_KEY) + 1);
                        oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMP3METADATA_INDEX0, oscl_strlen(PVMP3METADATA_INDEX0));
                        oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                        KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                        // Copy the value
                        KeyVal.value.uint32_value = mp3info.SamplingRate;
                        // Set the length and capacity
                        KeyVal.length = 1;
                        KeyVal.capacity = 1;
                    }
                    else
                    {
                        // Memory allocation failed
                        KeyVal.key = NULL;
                        break;
                    }
                }
            }
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_FORMAT_KEY) == 0 ||
                 oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_TRACKINFO_AUDIO_FORMAT_KEY) == 0)
        {
            // Format
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > aStartingValueIndex)
            {
                KeyLen = oscl_strlen(PVMP3METADATA_TRACKINFO_AUDIO_FORMAT_KEY) + 1; // for "track-info/audio/format;"
                KeyLen += oscl_strlen(PVMP3METADATA_INDEX0) + 1; // for "index=0;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR) + 1; // for "char*" and NULL terminator

                uint32 valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_MP3)) + 1; // Add value string plus one for NULL terminator
                // Allocate memory for the strings
                int32 leavecode1 = OsclErrNone;
                leavecode = OsclErrNone;
                KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);
                if (OsclErrNone == leavecode)
                {
                    KeyVal.value.pChar_value = (char*) AllocateKVPKeyArray(leavecode1, PVMI_KVPVALTYPE_CHARPTR, valuelen);
                }

                if (OsclErrNone == leavecode && OsclErrNone == leavecode1)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVMP3METADATA_TRACKINFO_AUDIO_FORMAT_KEY, oscl_strlen(PVMP3METADATA_TRACKINFO_AUDIO_FORMAT_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMP3METADATA_INDEX0, oscl_strlen(PVMP3METADATA_INDEX0));
                    oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                    // Copy the value
                    oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_MP3), valuelen);
                    KeyVal.value.pChar_value[valuelen-1] = NULL_TERM_CHAR;
                    // Set the length and capacity
                    KeyVal.length = valuelen;
                    KeyVal.capacity = valuelen;
                }
                else
                {
                    // Memory allocation failed so clean up
                    if (KeyVal.key)
                    {
                        OSCL_ARRAY_DELETE(KeyVal.key);
                        KeyVal.key = NULL;
                    }
                    if (KeyVal.value.pChar_value)
                    {
                        OSCL_ARRAY_DELETE(KeyVal.value.pChar_value);
                    }
                    break;
                }
            }
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_CHANNELS_KEY) == 0 ||
                 oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_TRACKINFO_AUDIO_CHANNELS_KEY) == 0)
        {
            // Channels
            MP3ContentFormatType mp3info;
            if (GetConfigDetails(mp3info) == MP3_SUCCESS)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > aStartingValueIndex)
                {
                    KeyLen = oscl_strlen(PVMP3METADATA_TRACKINFO_AUDIO_CHANNELS_KEY) + 1; // for "track-info/audio/channels;"
                    KeyLen += oscl_strlen(PVMP3METADATA_INDEX0) + 1; // for "index=0;"
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                    // Allocate memory for the string
                    leavecode = OsclErrNone;
                    KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);

                    if (OsclErrNone == leavecode)
                    {
                        // Copy the key string
                        oscl_strncpy(KeyVal.key, PVMP3METADATA_TRACKINFO_AUDIO_CHANNELS_KEY, oscl_strlen(PVMP3METADATA_TRACKINFO_AUDIO_CHANNELS_KEY) + 1);
                        oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMP3METADATA_INDEX0, oscl_strlen(PVMP3METADATA_INDEX0));
                        oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                        KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                        // Copy the value
                        KeyVal.value.uint32_value = mp3info.NumberOfChannels;
                        // Set the length and capacity
                        KeyVal.length = 1;
                        KeyVal.capacity = 1;
                    }
                    else
                    {
                        // Memory allocation failed
                        KeyVal.key = NULL;
                        break;
                    }
                }
            }
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_CHANNEL_MODE_KEY) == 0 ||
                 oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP3METADATA_TRACKINFO_AUDIO_CHANNEL_MODE_KEY) == 0)
        {
            // Channel mode
            MP3ContentFormatType mp3info;
            if (GetConfigDetails(mp3info) == MP3_SUCCESS)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > aStartingValueIndex)
                {
                    KeyLen = oscl_strlen(PVMP3METADATA_TRACKINFO_AUDIO_CHANNEL_MODE_KEY) + 1; // for "track-info/audio/channel-mode;"
                    KeyLen += oscl_strlen(PVMP3METADATA_INDEX0) + 1; // for "index=0;"
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                    KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                    // Allocate memory for the string
                    leavecode = 0;
                    KeyVal.key = (char*) AllocateKVPKeyArray(leavecode, PVMI_KVPVALTYPE_CHARPTR, KeyLen);

                    if (leavecode == 0)
                    {
                        // Copy the key string
                        oscl_strncpy(KeyVal.key, PVMP3METADATA_TRACKINFO_AUDIO_CHANNEL_MODE_KEY, oscl_strlen(PVMP3METADATA_TRACKINFO_AUDIO_CHANNEL_MODE_KEY) + 1);
                        oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMP3METADATA_INDEX0, oscl_strlen(PVMP3METADATA_INDEX0));
                        oscl_strncat(KeyVal.key, PVMP3METADATA_SEMICOLON, oscl_strlen(PVMP3METADATA_SEMICOLON));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                        oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                        KeyVal.key[KeyLen-1] = NULL_TERM_CHAR;
                        // Copy the value
                        KeyVal.value.uint32_value = mp3info.ChannelMode;
                        // Set the length and capacity
                        KeyVal.length = 1;
                        KeyVal.capacity = 1;
                    }
                    else
                    {
                        // Memory allocation failed
                        KeyVal.key = NULL;
                        break;
                    }
                }
            }
        }

        // Add the KVP to the list if the key string was created
        if (KeyVal.key != NULL)
        {
            leavecode = OsclErrNone;
            leavecode = PushKVPValue(KeyVal, aValueList);
            if (OsclErrNone != leavecode)
            {
                // push kvp failed
                return PVMFErrNoMemory;
            }

            // Increment the counter for number of value entries added to the list
            ++numentriesadded;
            // Check if the max number of value entries were added
            if (aMaxValueEntries > 0 && numentriesadded >= aMaxValueEntries)
            {
                // Maximum number of values added so break out of the loop
                break;
            }
        }

    }  // End of for loop

    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus IMpeg3File::ReleaseMetadataValue(PvmiKvp& aValueKVP)
{
    if (aValueKVP.key == NULL)
    {
        return PVMFErrArgument;
    }

    switch (GetValTypeFromKeyString(aValueKVP.key))
    {
        case PVMI_KVPVALTYPE_WCHARPTR:
            if ((aValueKVP.value.pWChar_value != NULL) && (aValueKVP.length != 0))
            {
                OSCL_ARRAY_DELETE(aValueKVP.value.pWChar_value);
                aValueKVP.value.pWChar_value = NULL;

            }
            break;

        case PVMI_KVPVALTYPE_CHARPTR:
            if ((aValueKVP.value.pChar_value != NULL) && (aValueKVP.length != 0))
            {
                OSCL_ARRAY_DELETE(aValueKVP.value.pChar_value);
                aValueKVP.value.pChar_value = NULL;
            }
            break;

        case PVMI_KVPVALTYPE_UINT8PTR:
            if ((aValueKVP.value.pUint8_value != NULL) && (aValueKVP.length != 0))
            {
                OSCL_ARRAY_DELETE(aValueKVP.value.pUint8_value);
                aValueKVP.value.pUint8_value = NULL;
            }

            break;

        case PVMI_KVPVALTYPE_UINT32:
        case PVMI_KVPVALTYPE_UINT8:
            // No memory to free for these valtypes
            break;

        default:
            // Should not get a value that wasn't created from here
            break;
    }

    OSCL_ARRAY_DELETE(aValueKVP.key);
    aValueKVP.key = NULL;

    return PVMFSuccess;
}


OSCL_EXPORT_REF MP3ErrorType IMpeg3File::IsMp3File(OSCL_wString& aFileName,
        PVMFCPMPluginAccessInterfaceFactory *aCPMAccessFactory,
        uint32 aInitSearchFileSize)
{
    MP3ErrorType errCode = MP3_ERROR_UNKNOWN_OBJECT;

    MP3_FF_FILE fileStruct;
    MP3_FF_FILE *fp = &fileStruct;

    fp->_pvfile.SetCPM(aCPMAccessFactory);

    // open the dummy file
    if (MP3Utils::OpenFile(aFileName,
                           Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,
                           fp) != 0)
    {
        errCode = MP3_FILE_OPEN_FAILED;
        return errCode;
    }
    // create the file parser object to recognize the clip
    MP3Parser* mp3Parser = NULL;
    mp3Parser = OSCL_NEW(MP3Parser, ());

    errCode = (mp3Parser) ? MP3_SUCCESS : MP3_ERROR_UNKNOWN;

    // File was opened successfully, clip recognition can proceed
    if (errCode == MP3_SUCCESS)
    {
        errCode = mp3Parser->IsMp3File(fp, aInitSearchFileSize);
        //deallocate the MP3Parser object created
        if (mp3Parser)
        {
            delete mp3Parser;
            mp3Parser = NULL;
        }
    }
    // close the file
    MP3Utils::CloseFile(&(fp->_pvfile)); //Close the MP3 File
    return errCode;
}


OSCL_EXPORT_REF MP3ErrorType IMpeg3File::RequestReadCapacityNotification(PvmiDataStreamObserver& aObserver,
        uint32 aFileOffset,
        OsclAny* aContextData)
{
    uint32 capacity = 0;
    uint32 currFilePosn = MP3Utils::getCurrentFilePosition(&iMP3File);
    if (aFileOffset > currFilePosn)
    {
        capacity = (aFileOffset - currFilePosn);
        bool retVal =
            iMP3File.RequestReadCapacityNotification(aObserver, capacity, aContextData);
        if (retVal)
        {
            return MP3_SUCCESS;
        }
    }
    return MP3_END_OF_FILE;
}

OSCL_EXPORT_REF MP3ErrorType IMpeg3File::ScanMP3File(uint32 aFramesToScan)
{
    if (pMP3Parser && iScanFP.IsOpen())
        return pMP3Parser->ScanMP3File(&iScanFP, aFramesToScan);
    return MP3_ERROR_UNKNOWN;
}

OsclAny* IMpeg3File::AllocateKVPKeyArray(int32& aLeaveCode, PvmiKvpValueType aValueType, int32 aNumElements)
{
    int32 leaveCode = OsclErrNone;
    OsclAny* aBuffer = NULL;
    switch (aValueType)
    {
        case PVMI_KVPVALTYPE_WCHARPTR:
            OSCL_TRY(leaveCode,
                     aBuffer = (oscl_wchar*) OSCL_ARRAY_NEW(oscl_wchar, aNumElements + 1);
                    );
            break;

        case PVMI_KVPVALTYPE_CHARPTR:
            OSCL_TRY(leaveCode,
                     aBuffer = (char*) OSCL_ARRAY_NEW(char, aNumElements + 1);
                    );
            break;
        case PVMI_KVPVALTYPE_UINT8PTR:
            OSCL_TRY(leaveCode,
                     aBuffer = (uint8*) OSCL_ARRAY_NEW(uint8, aNumElements);
                    );
            break;
        default:
            break;
    }
    aLeaveCode = leaveCode;
    return aBuffer;
}


int32 IMpeg3File::PushKVPValue(PvmiKvp aKVP, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList)
{
    int32 leavecode = OsclErrNone;
    OSCL_TRY(leavecode, aValueList.push_back(aKVP));
    if (OsclErrNone != leavecode)
    {
        ReleaseMetadataValue(aKVP);
    }
    return leavecode;
}

int32 IMpeg3File::PushKVPKey(const char* aString, PVMFMetadataList& aKeyList)
{
    int32 leavecode = OsclErrNone;
    OSCL_TRY(leavecode, aKeyList.push_back(aString));
    return leavecode;
}

int32 IMpeg3File::PushKVPKey(OSCL_HeapString<OsclMemAllocator>& aString, Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator>& aKeyList)
{
    int32 leavecode = OsclErrNone;
    OSCL_TRY(leavecode, aKeyList.push_back(aString));
    return leavecode;
}
