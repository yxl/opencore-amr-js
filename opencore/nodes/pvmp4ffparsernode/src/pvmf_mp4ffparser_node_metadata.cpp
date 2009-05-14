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
#include "pvmf_mp4ffparser_node.h"

#include "impeg4file.h"


#include "media_clock_converter.h"

#include "pv_mime_string_utils.h"

#include "oscl_snprintf.h"

#include "pvmf_duration_infomessage.h"

#include "pvmi_kvp_util.h"

#include "h263decoderspecificinfo.h"

#include "oscl_exclusive_ptr.h"

// Constant character strings for metadata keys
static const char PVMP4_ALL_METADATA_KEY[] = "all";
static const char PVMP4METADATA_CLIP_TYPE_KEY[] = "clip-type";
static const char PVMP4METADATA_ALBUM_KEY[] = "album";
static const char PVMP4METADATA_COMMENT_KEY[] = "comment";


static const char PVMP4METADATA_LOCATION_KEY[] = "location;format=3GPP_LOCATION";
static const char PVMP4METADATA_YEAR_KEY[] = "year";
static const char PVMP4METADATA_AUTHOR_KEY[] = "author";
static const char PVMP4METADATA_ARTIST_KEY[] = "artist";
static const char PVMP4METADATA_GENRE_KEY[] = "genre";
static const char PVMP4METADATA_KEYWORD_KEY[] = "keyword";
static const char PVMP4METADATA_CLASSIFICATION_KEY[] = "classification";
static const char PVMP4METADATA_TITLE_KEY[] = "title";
static const char PVMP4METADATA_DESCRIPTION_KEY[] = "description";
static const char PVMP4METADATA_RATING_KEY[] = "rating";
static const char PVMP4METADATA_COPYRIGHT_KEY[] = "copyright";
static const char PVMP4METADATA_VERSION_KEY[] = "version";
static const char PVMP4METADATA_DATE_KEY[] = "date";
static const char PVMP4METADATA_DURATION_KEY[] = "duration";
static const char PVMP4METADATA_NUMTRACKS_KEY[] = "num-tracks";
static const char PVMP4METADATA_IS_MOOF_KEY[] = "movie-fragments-present";

static const char PVMP4METADATA_TOOL_KEY[] = "tool";
static const char PVMP4METADATA_WRITER_KEY[] = "writer";
static const char PVMP4METADATA_GROUPING_KEY[] = "grouping";
static const char PVMP4METADATA_TRACKDATA_KEY[] = "track data";
static const char PVMP4METADATA_COMPILATION_KEY[] = "compilation";
static const char PVMP4METADATA_TEMPO_KEY[] = "tempo";
static const char PVMP4METADATA_COVER_KEY[] = "cover";
static const char PVMP4METADATA_DISKDATA_KEY[] = "disk";
static const char PVMP4METADATA_FREEFORMDATA_KEY[] = "free form data";
static const char PVMP4METADATA_CDDBID_KEY[] = "CD identifier";
static const char PVMP4METADATA_LYRICS_KEY[] = "lyrics";
static const char PVMP4METADATA_RANDOM_ACCESS_DENIED_KEY[] = "random-access-denied";
//////////////////////////////////////////////////////////

static const char PVMP4METADATA_TRACKINFO_TYPE_KEY[] = "track-info/type";
static const char PVMP4METADATA_TRACKINFO_TRACKID_KEY[] = "track-info/track-id";
static const char PVMP4METADATA_TRACKINFO_DURATION_KEY[] = "track-info/duration";
static const char PVMP4METADATA_TRACKINFO_BITRATE_KEY[] = "track-info/bit-rate";
static const char PVMP4METADATA_TRACKINFO_SAMPLECOUNT_KEY[] = "track-info/num-samples";
static const char PVMP4METADATA_TRACKINFO_SELECTED_KEY[] = "track-info/selected";

static const char PVMP4METADATA_TRACKINFO_AUDIO_FORMAT_KEY[] = "track-info/audio/format";
static const char PVMP4METADATA_TRACKINFO_AUDIO_NUMCHANNELS_KEY[] = "track-info/audio/channels";
static const char PVMP4METADATA_TRACKINFO_SAMPLERATE_KEY[] = "track-info/sample-rate";
static const char PVMP4METADATA_TRACKINFO_AUDIO_BITS_PER_SAMPLE_KEY[] = "track-info/audio/bits-per-sample";

static const char PVMP4METADATA_TRACKINFO_VIDEO_FORMAT_KEY[] = "track-info/video/format";
static const char PVMP4METADATA_TRACKINFO_VIDEO_WIDTH_KEY[] = "track-info/video/width";
static const char PVMP4METADATA_TRACKINFO_VIDEO_HEIGHT_KEY[] = "track-info/video/height";
static const char PVMP4METADATA_TRACKINFO_VIDEO_PROFILE_KEY[] = "track-info/video/profile";
static const char PVMP4METADATA_TRACKINFO_VIDEO_LEVEL_KEY[] = "track-info/video/level";
static const char PVMP4METADATA_TRACKINFO_FRAME_RATE_KEY[] = "track-info/frame-rate";
static const char PVMP4METADATA_TRACKINFO_TRACK_NUMBER_KEY[] = "track-info/track-number";
static const char PVMP4METADATA_TRACKINFO_NUM_KEY_SAMPLES_KEY[] = "track-info/num-key-samples";

static const char PVMP4METADATA_MAJORBRAND_KEY[] = "mp4ff/major-brand";
static const char PVMP4METADATA_COMPATIBLEBRAND_KEY[] = "mp4ff/compatible-brand";

static const char PVMP4METADATA_SEMICOLON[] = ";";
static const char PVMP4METADATA_TIMESCALE[] = "timescale=";
static const char PVMP4METADATA_INDEX[] = "index=";
static const char PVMP4METADATA_LANG_CODE[] = "iso-639-2-lang=";
static const char PVMP4METADATA_NOT_SOTRABLE[] = "not-storable";
static const char PVMP4METADATA_MAXSIZE[] = "maxsize=";
static const char PVMP4METADATA_REQ_SIZE[] = "reqsize=";
static const char PVMP4METADATA_ORIG_CHAR_ENC[] = "orig-char-enc=";

#define PVMF_MP4_MIME_FORMAT_AUDIO_UNKNOWN	"x-pvmf/audio/unknown"
#define PVMF_MP4_MIME_FORMAT_VIDEO_UNKNOWN	"x-pvmf/video/unknown"
#define PVMF_MP4_MIME_FORMAT_UNKNOWN		"x-pvmf/unknown-media/unknown"

#define MILLISECOND_TIMESCALE (1000)
#define PVMF_MP4_MAX_UINT32   (0xffffffffU)

uint32 PVMFMP4FFParserNode::GetNumMetadataKeys(char* aQueryKeyString)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP4FFParserNode::GetNumMetadataKeys() called"));

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
            if (pv_mime_strcmp(iAvailableMetadataKeys[i].get_cstr(), aQueryKeyString) >= 0)
            {
                num_entries++;
            }
        }
    }
    if ((iCPMMetaDataExtensionInterface != NULL) &&
            (iProtectedFile == true))
    {
        num_entries +=
            iCPMMetaDataExtensionInterface->GetNumMetadataKeys(aQueryKeyString);
    }
    return num_entries;
}


uint32 PVMFMP4FFParserNode::GetNumMetadataValues(PVMFMetadataList& aKeyList)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP4FFParserNode::GetNumMetadataValues() called"));


    if (aKeyList.size() == 0)
    {
        return 0;
    }

    uint32 numvalentries = 0;

    if ((iCPMMetaDataExtensionInterface != NULL) &&
            (iProtectedFile == true))
    {
        numvalentries +=
            iCPMMetaDataExtensionInterface->GetNumMetadataValues(aKeyList);
    }

    if (iMP4FileHandle == NULL)
    {
        return numvalentries;
    }

    int32 iNumTracks = iMP4FileHandle->getNumTracks();
    uint32 iIdList[16];
    if (iNumTracks != iMP4FileHandle->getTrackIDList(iIdList, iNumTracks))
    {
        return 0;
    }
    // Retrieve the track ID list
    OsclExclusiveArrayPtr<uint32> trackidlistexclusiveptr;
    uint32* trackidlist = NULL;
    uint32 numTracks = (uint32)(iNumTracks);
    PVMFStatus status = CreateNewArray(&trackidlist, numTracks);
    if (PVMFErrNoMemory == status)
    {
        return PVMFErrNoMemory;
    }
    oscl_memset(trackidlist, 0, sizeof(uint32)*(numTracks));
    iMP4FileHandle->getTrackIDList(trackidlist, numTracks);
    trackidlistexclusiveptr.set(trackidlist);


    uint32 numkeys = aKeyList.size();
    for (uint32 lcv = 0; lcv < numkeys; lcv++)
    {
        if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_TOOL_KEY) == 0)
        {
            // Tool
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_WRITER_KEY) == 0)
        {
            // Writer
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_GROUPING_KEY) == 0)
        {
            // Grouping
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKDATA_KEY) == 0)
        {
            // Trackdata
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_COMPILATION_KEY) == 0) && (iMP4FileHandle->IsITunesCompilationPart() == true))
        {
            //Compilation
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_TEMPO_KEY) == 0)
        {
            // Tempo
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_DISKDATA_KEY) == 0)
        {
            // Disk data
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_LYRICS_KEY) == 0)
        {
            // Lyrics
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }

        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_FREEFORMDATA_KEY) == 0)
        {
            // Free form data
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }

        if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_AUTHOR_KEY) == 0) &&
                (iMP4FileHandle->getNumAuthor() > 0))
        {
            // Author
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_ALBUM_KEY) == 0) &&
                 (iMP4FileHandle->getNumAlbum() > 0))
        {
            // Album
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_CLIP_TYPE_KEY) == 0)
        {
            // clip-type
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_COMMENT_KEY) == 0) &&
                 (iMP4FileHandle->getNumComment() > 0))
        {
            // Comment
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_COVER_KEY) == 0)
        {
            // Cover
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_RANDOM_ACCESS_DENIED_KEY) == 0)
        {
            /*
             * Random Access
             * Increment the counter for the number of values found so far
             */
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_IS_MOOF_KEY) == 0)
        {
            /*
             * is-moof
             * Increment the counter for the number of values found so far
             */
            ++numvalentries;
        }

        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_LOCATION_KEY) == 0)
        {
            /*
             * location
             * Determine the index requested. Default to all pictures */

            uint32 NumLocations = iMP4FileHandle->getNumAssetInfoLocationAtoms();

            if (!NumLocations)
                break;

            uint32 startindex = 0;
            uint32 endindex = (uint32)(NumLocations - 1);

            /* Check if the index parameter is present */
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if (startindex > endindex || startindex >= (uint32)NumLocations || endindex >= (uint32)NumLocations)
            {
                break;
            }
            /* Return a KVP for each index */
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                PvmiKvp trackkvp;
                trackkvp.key = NULL;
                /* Increment the counter for the number of values found so far */
                ++numvalentries;
            }
        }

        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_ARTIST_KEY) == 0) &&
                 (iMP4FileHandle->getNumArtist() > 0))
        {
            // Artist
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_GENRE_KEY) == 0) &&
                 (iMP4FileHandle->getNumGenre() > 0))
        {
            // Genre
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_KEYWORD_KEY) == 0)
        {
            int32 numAssetInfoKeyword = iMP4FileHandle->getNumAssetInfoKeyWordAtoms();
            for (int32 idx = 0; idx < numAssetInfoKeyword; idx++)
            {
                int32 AssetInfoKeywordCount = iMP4FileHandle->getAssetInfoNumKeyWords(idx);
                for (int32 idy = 0; idy < AssetInfoKeywordCount; idy++)
                {

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                }
            }
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_CLASSIFICATION_KEY) == 0)
        {

            int32 numAssetInfoClassification = iMP4FileHandle->getNumAssetInfoClassificationAtoms();
            // classification
            // Increment the counter for the number of values found so far
            numvalentries = numvalentries + numAssetInfoClassification;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_MAJORBRAND_KEY) == 0)
        {
            // MAJOR BRAND
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_COMPATIBLEBRAND_KEY) == 0)
        {
            // COMPATIBLE BRAND
            // Increment the counter for the number of values found so far
            ++numvalentries;

            Oscl_Vector<uint32, OsclMemAllocator> *Compatiblebrand_Vec = iMP4FileHandle->getCompatibiltyList();
            if (Compatiblebrand_Vec)
            {
                numvalentries += Compatiblebrand_Vec->size();
            }
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_TITLE_KEY) == 0) &&
                 (iMP4FileHandle->getNumTitle() > 0))
        {
            // Title
            // Increment the counter for the number of values found so far
            numvalentries = numvalentries + iMP4FileHandle->getNumTitle();
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_DESCRIPTION_KEY) == 0)  &&
                 (iMP4FileHandle->getNumDescription() > 0))
        {
            // Description
            // Increment the counter for the number of values found so far
            numvalentries = numvalentries + iMP4FileHandle->getNumDescription();
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_RATING_KEY) == 0) &&
                 (iMP4FileHandle->getNumRating() > 0))
        {
            // Rating
            // Increment the counter for the number of values found so far
            numvalentries = numvalentries + iMP4FileHandle->getNumRating();
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_COPYRIGHT_KEY) == 0) &&
                 (iMP4FileHandle->getNumCopyright() > 0))
        {
            // Copyright
            // Increment the counter for the number of values found so far
            numvalentries = numvalentries + iMP4FileHandle->getNumCopyright();
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_VERSION_KEY) == 0)
        {
            // Version
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_DATE_KEY) == 0)
        {
            // Date
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_DURATION_KEY) == 0)
        {
            // Movie Duration
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_NUMTRACKS_KEY) == 0 &&
                 iMP4FileHandle->getNumTracks() > 0)
        {
            // Number of tracks
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVMP4METADATA_YEAR_KEY) == 0) &&
                 (iMP4FileHandle->getNumYear() > 0))
        {
            // year
            // Increment the counter for the number of values found so far
            numvalentries = numvalentries + iMP4FileHandle->getNumYear();
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_VIDEO_PROFILE_KEY) != NULL)
        {
            // profile
            // Determine the index requested.
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = 0;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices - there should only be one index
            if (startindex != endindex || startindex > (uint32)(numtracks) || endindex > (uint32)(numtracks))
            {
                break;
            }
            //get track id from index
            uint32 trackID = startindex + 1;

            OSCL_HeapString<OsclMemAllocator> trackMIMEType;
            iMP4FileHandle->getTrackMIMEType(trackID, trackMIMEType);

            if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000)) == 0)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;
            }
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_VIDEO_LEVEL_KEY) != NULL)
        {
            // level
            // Determine the index requested.
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = 0;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices - there should only be one index
            if (startindex != endindex || startindex > (uint32)(numtracks) || endindex > (uint32)(numtracks))
            {
                break;
            }
            //get track id from index
            uint32 trackID = startindex + 1;

            OSCL_HeapString<OsclMemAllocator> trackMIMEType;
            iMP4FileHandle->getTrackMIMEType(trackID, trackMIMEType);

            if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000)) == 0)
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;
            }
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_FRAME_RATE_KEY) != NULL)
        {
            // frame-rate
            // Determine the index requested.
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = 0;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices - there should only be one index
            if (startindex != endindex || startindex > (uint32)(numtracks) || endindex > (uint32)(numtracks))
            {
                break;
            }
            //get track id from index
            uint32 trackID = startindex + 1;
            OSCL_HeapString<OsclMemAllocator> trackMIMEType;

            iMP4FileHandle->getTrackMIMEType(trackID, trackMIMEType);

            if ((oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_M4V, oscl_strlen(PVMF_MIME_M4V)) == 0) ||
                    (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000)) == 0) ||
                    (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H264_VIDEO_MP4, oscl_strlen(PVMF_MIME_H264_VIDEO_MP4)) == 0))
            {
                // Increment the counter for the number of values found so far
                ++numvalentries;
            }
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_TYPE_KEY) != NULL)
        {
            // Track type

            // Determine the index requested. Default to all tracks
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = (uint32)numtracks - 1;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                break;
            }

            // Increment the counter for the number of values found so far
            numvalentries += (endindex + 1 - startindex);
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_TRACKID_KEY) != NULL)
        {
            // Track ID

            // Determine the index requested. Default to all tracks
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = (uint32)numtracks - 1;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                break;
            }

            // Increment the counter for the number of values found so far
            numvalentries += (endindex + 1 - startindex);
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_DURATION_KEY) != NULL)
        {
            // Track duration

            // Determine the index requested. Default to all tracks
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = (uint32)numtracks - 1;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                break;
            }

            // Increment the counter for the number of values found so far
            numvalentries += (endindex + 1 - startindex);
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_TRACK_NUMBER_KEY) != NULL)
        {
            uint32 numCDTrackNumber = 0;

            if (iMP4FileHandle->getITunesThisTrackNo() > 0)
                numCDTrackNumber++;


            if (numCDTrackNumber > 0)
            {
                // Track Number

                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Increment the counter for the number of values found so far
                numvalentries += (endindex + 1 - startindex);
                numvalentries = numCDTrackNumber * numvalentries;
            }
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_BITRATE_KEY) != NULL)
        {
            // Track bitrate

            // Determine the index requested. Default to all tracks
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = (uint32)numtracks - 1;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                break;
            }

            // Increment the counter for the number of values found so far
            numvalentries += (endindex + 1 - startindex);
        }
        else if ((oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_AUDIO_FORMAT_KEY) != NULL) ||
                 (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_VIDEO_FORMAT_KEY) != NULL))
        {
            // Audio or video track format
            // Set index for track type
            uint32 tracktype = 0; // 0 unknown, 1 video, 2 audio
            if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_VIDEO_FORMAT_KEY) != NULL)
            {
                tracktype = 1;
            }
            else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_AUDIO_FORMAT_KEY) != NULL)
            {
                tracktype = 2;
            }

            // Determine the index requested. Default to all tracks
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = (uint32)numtracks - 1;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                break;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                OSCL_HeapString<OsclMemAllocator> trackMIMEType;

                iMP4FileHandle->getTrackMIMEType(trackidlist[i], trackMIMEType);

                if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_M4V, oscl_strlen(PVMF_MIME_M4V)) == 0)
                {
                    if (tracktype == 1)
                    {
                        ++numvalentries;
                    }
                }
                else if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000)) == 0)
                {
                    if (tracktype == 1)
                    {
                        ++numvalentries;
                    }
                }
                else if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H264_VIDEO_MP4, oscl_strlen(PVMF_MIME_H264_VIDEO_MP4)) == 0)
                {
                    if (tracktype == 1)
                    {
                        ++numvalentries;
                    }
                }
                else if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_MPEG4_AUDIO, oscl_strlen(PVMF_MIME_MPEG4_AUDIO)) == 0)
                {
                    if (tracktype == 2)
                    {
                        ++numvalentries;
                    }
                }
                else if ((oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_AMR_IETF, oscl_strlen(PVMF_MIME_AMR_IETF)) == 0) ||
                         (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_AMRWB_IETF, oscl_strlen(PVMF_MIME_AMRWB_IETF)) == 0))
                {
                    if (tracktype == 2)
                    {
                        ++numvalentries;
                    }
                }
            }
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_VIDEO_WIDTH_KEY) != NULL)
        {
            // Video track width

            // Determine the index requested. Default to all tracks
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = (uint32)numtracks - 1;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                break;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                PvmiKvp trackkvp;
                trackkvp.key = NULL;

                if (iMP4FileHandle->getTrackMediaType(trackidlist[i]) == MEDIA_TYPE_VISUAL)
                {
                    // Increment the counter for the number of values found so far
                    numvalentries++;
                }
            }
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_VIDEO_HEIGHT_KEY) != NULL)
        {
            // Video track height

            // Determine the index requested. Default to all tracks
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = (uint32)numtracks - 1;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                break;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                PvmiKvp trackkvp;
                trackkvp.key = NULL;

                if (iMP4FileHandle->getTrackMediaType(trackidlist[i]) == MEDIA_TYPE_VISUAL)
                {
                    // Increment the counter for the number of values found so far
                    numvalentries++;
                }
            }
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_SAMPLERATE_KEY) != NULL)
        {
            // Sampling rate (only for video tracks)

            // Determine the index requested. Default to all tracks
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = (uint32)numtracks - 1;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                break;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                PvmiKvp trackkvp;
                trackkvp.key = NULL;

                if (iMP4FileHandle->getTrackMediaType(trackidlist[i]) == MEDIA_TYPE_AUDIO)
                {
                    // Increment the counter for the number of values found so far
                    numvalentries++;
                }
            }
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_SAMPLECOUNT_KEY) != NULL)
        {
            // Sample count

            // Determine the index requested. Default to all tracks
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = (uint32)numtracks - 1;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                break;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                PvmiKvp trackkvp;
                trackkvp.key = NULL;

                // Increment the counter for the number of values found so far
                numvalentries++;
            }
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_SELECTED_KEY) != NULL)
        {
            // Track selected info

            // Determine the index requested. Default to all tracks
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = (uint32)numtracks - 1;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                break;
            }

            // Increment the counter for the number of values found so far
            numvalentries += (endindex + 1 - startindex);
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_NUM_KEY_SAMPLES_KEY) != NULL)
        {
            // Num-Key-Samples

            // Determine the index requested. Default to all tracks
            // Check if the file has at least one track
            int32 numtracks = iMP4FileHandle->getNumTracks();
            if (numtracks <= 0)
            {
                break;
            }
            uint32 startindex = 0;
            uint32 endindex = (uint32)numtracks - 1;
            // Check if the index parameter is present
            const char* indexstr = oscl_strstr(aKeyList[lcv].get_cstr(), PVMP4METADATA_INDEX);
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                break;
            }

            // Return a KVP for each index
            numvalentries += endindex - startindex + 1;

        }


    }
    return numvalentries;
}


PVMFCommandId PVMFMP4FFParserNode::GetNodeMetadataKeys(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList, uint32 starting_index, int32 max_entries, char* query_key, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP4FFParserNode::GetNodeMetadataKeys() called"));

    PVMFMP4FFParserNodeCommand cmd;
    cmd.PVMFMP4FFParserNodeCommand::Construct(aSessionId, PVMP4FF_NODE_CMD_GETNODEMETADATAKEYS, aKeyList, starting_index, max_entries, query_key, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFMP4FFParserNode::GetNodeMetadataValues(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 starting_index, int32 max_entries, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP4FFParserNode::GetNodeMetadataValue() called"));

    PVMFMP4FFParserNodeCommand cmd;
    cmd.PVMFMP4FFParserNodeCommand::Construct(aSessionId, PVMP4FF_NODE_CMD_GETNODEMETADATAVALUES, aKeyList, aValueList, starting_index, max_entries, aContext);
    return QueueCommandL(cmd);
}


PVMFStatus PVMFMP4FFParserNode::ReleaseNodeMetadataKeys(PVMFMetadataList& , uint32 , uint32)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP4FFParserNode::ReleaseNodeMetadataKeys() called"));

    // Nothing needed-- there's no dynamic allocation in this node's key list
    return PVMFSuccess;
}


PVMFStatus PVMFMP4FFParserNode::ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
        uint32 start,
        uint32 end)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP4FFParserNode::ReleaseNodeMetadataValues() called"));

    if (start > end || aValueList.size() == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFMP4FFParserNode::ReleaseNodeMetadataValues() Invalid start/end index"));
        return PVMFErrArgument;
    }

    end = iMP4ParserNodeMetadataValueCount;

    PVMF_MP4FFPARSERNODE_LOGERROR((0, "PVMFMP4FFParserNode::ReleaseNodeMetadataValues() - iMP4ParserNodeMetadataValueCount=%d", iMP4ParserNodeMetadataValueCount));
    PVMF_MP4FFPARSERNODE_LOGERROR((0, "PVMFMP4FFParserNode::ReleaseNodeMetadataValues() - Start=%d, End=%d", start, end));

    for (uint32 i = start; i < end; i++)
    {
        char* key = aValueList[i].key;
        if (key != NULL)
        {
            switch (GetValTypeFromKeyString(key))
            {
                case PVMI_KVPVALTYPE_WCHARPTR:
                    if (aValueList[i].value.pWChar_value != NULL)
                    {
                        OSCL_ARRAY_DELETE(aValueList[i].value.pWChar_value);
                        aValueList[i].value.pWChar_value = NULL;
                    }
                    break;

                case PVMI_KVPVALTYPE_CHARPTR:
                    if (aValueList[i].value.pChar_value != NULL)
                    {
                        OSCL_ARRAY_DELETE(aValueList[i].value.pChar_value);
                        aValueList[i].value.pChar_value = NULL;
                    }
                    break;

                case PVMI_KVPVALTYPE_UINT32:
                case PVMI_KVPVALTYPE_FLOAT:
                case PVMI_KVPVALTYPE_BOOL:
                    // No need to free memory for this valtype
                    break;


                case PVMI_KVPVALTYPE_KSV:


                    /*				if (aValueList[i].value.key_specific_value != NULL)
                    				 {

                    				     	if( ((PvmfApicStruct *)aValueList[i].value.key_specific_value)->iGraphicData != NULL)
                    						{
                    							oscl_free(((PvmfApicStruct *)aValueList[i].value.key_specific_value)->iGraphicData);											//OSCL_DEFAULT_FREE(((PvmfApicStruct *)aValueKVP.value.key_specific_value)->iGraphicMimeType);
                     							((PvmfApicStruct *)aValueList[i].value.key_specific_value)->iGraphicData=NULL;
                    						}

                    						OSCL_DELETE(((PvmfApicStruct *)aValueList[i].value.key_specific_value));

                    						aValueList[i].value.key_specific_value=NULL;

                    				}
                    */
                    break;

                default:
                {
                    // Should not get a value that wasn't created from this node
                    PVMF_MP4FFPARSERNODE_LOGERROR((0, "PVMFMP4FFParserNode::ReleaseNodeMetadataValues - ErrKey=%s", aValueList[i].key));
                    PVMF_MP4FFPARSERNODE_LOGERROR((0, "PVMFMP4FFParserNode::ReleaseNodeMetadataValues - Key Not Created By This Node"));
                    OSCL_ASSERT(false);
                }
                break;
            }
            PVMF_MP4FFPARSERNODE_LOGERROR((0, "PVMFMP4FFParserNode::ReleaseNodeMetadataValues - Deleting - Index=%d, Key=%s", i, aValueList[i].key));
            OSCL_ARRAY_DELETE(aValueList[i].key);
            aValueList[i].key = NULL;
        }
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP4FFParserNode::ReleaseNodeMetadataValues() complete"));
    return PVMFSuccess;
}


void PVMFMP4FFParserNode::PushToAvailableMetadataKeysList(const char* aKeystr, char* aOptionalParam)
{
    if (aKeystr == NULL)
    {
        return;
    }

    if (aOptionalParam)
    {
        iAvailableMetadataKeys.push_front(aKeystr);
        iAvailableMetadataKeys[0] += aOptionalParam;
    }

    else
    {
        iAvailableMetadataKeys.push_front(aKeystr);
    }
}



int32 PVMFMP4FFParserNode::CountMetaDataKeys()
{
    MP4FFParserOriginalCharEnc charType;
    if (iMP4FileHandle == NULL)
    {
        return -1;
    }

    int32 NumMetaDataKeysAvailable = 0;

    int32 iNumTracks = iMP4FileHandle->getNumTracks();
    uint32 iIdList[16];
    if (iNumTracks != iMP4FileHandle->getTrackIDList(iIdList, iNumTracks))
    {
        return -1;
    }




    for (int32 i = iNumTracks - 1; i >= 0; i--)
    {
        uint32 trackID = iIdList[i];

        OSCL_HeapString<OsclMemAllocator> trackMIMEType;

        iMP4FileHandle->getTrackMIMEType(trackID, trackMIMEType);

        if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000)) == 0)
        {
            //track id is a one based index
            NumMetaDataKeysAvailable += 2;
        }
        if ((oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_M4V, oscl_strlen(PVMF_MIME_M4V)) == 0) ||
                (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000)) == 0) ||
                (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H264_VIDEO_MP4, oscl_strlen(PVMF_MIME_H264_VIDEO_MP4)) == 0))
        {
            NumMetaDataKeysAvailable += 4;
        }
        if ((oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_MPEG4_AUDIO, oscl_strlen(PVMF_MIME_MPEG4_AUDIO)) == 0) ||
                (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_AMR, oscl_strlen(PVMF_MIME_AMR)) == 0) ||
                (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_AMR_IETF, oscl_strlen(PVMF_MIME_AMR_IETF)) == 0) ||
                (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_AMRWB_IETF, oscl_strlen(PVMF_MIME_AMRWB_IETF)) == 0))
        {
            NumMetaDataKeysAvailable += 3;
        }
    }

    if (iMP4FileHandle->getNumAuthor() > 0)
    {
        NumMetaDataKeysAvailable++;
    }
    //Common Keys
    if (iMP4FileHandle->getNumAlbum() > 0)
    {
        NumMetaDataKeysAvailable++;
    }


    if (iMP4FileHandle->getNumArtist() > 0)
    {
        NumMetaDataKeysAvailable++;
    }

    if (iMP4FileHandle->getNumGenre() > 0)
    {
        NumMetaDataKeysAvailable++;
    }


    if (iMP4FileHandle->getNumYear() > 0)
    {
        NumMetaDataKeysAvailable++;
    }

    if (iMP4FileHandle->getNumTitle() > 0)
    {
        NumMetaDataKeysAvailable++;
    }
    if (iMP4FileHandle->getNumCopyright() > 0)
    {
        NumMetaDataKeysAvailable++;
    }

    if (iMP4FileHandle->getNumComment() > 0)
    {
        NumMetaDataKeysAvailable++;
    }

    if (iMP4FileHandle->getNumDescription() > 0)
    {
        NumMetaDataKeysAvailable++;
    }

    if (iMP4FileHandle->getNumRating() > 0)
    {
        NumMetaDataKeysAvailable++;
    }

    if (iMP4FileHandle->getNumAssetInfoKeyWordAtoms() > 0)
    {
        NumMetaDataKeysAvailable++;
    }
    if (iMP4FileHandle->getNumAssetInfoClassificationAtoms() > 0)
    {
        NumMetaDataKeysAvailable++;
    }
    if (iMP4FileHandle->getCompatibiltyMajorBrand() > 0)
    {
        NumMetaDataKeysAvailable++;
    }

    if (iMP4FileHandle->getCompatibiltyList() != NULL)
    {
        if (iMP4FileHandle->getCompatibiltyList()->size() > 0)
        {
            NumMetaDataKeysAvailable++;
        }
    }

    if (iMP4FileHandle->getPVVersion(charType).get_size() > 0)
    {
        NumMetaDataKeysAvailable++;
    }

    if (iMP4FileHandle->getCreationDate(charType).get_size() > 0)
    {
        NumMetaDataKeysAvailable++;
    }

    if (iMP4FileHandle->getMovieDuration() > (uint64)0)
    {
        NumMetaDataKeysAvailable++;
    }

    if (iMP4FileHandle->getITunesBeatsPerMinute() > 0)
    {
        NumMetaDataKeysAvailable++;
    }

    if (iMP4FileHandle->getITunesCDIdentifierData(0).get_size() > 0)
    {
        NumMetaDataKeysAvailable++;
    }

    if (iMP4FileHandle->getITunesGroupData().get_size() > 0)
    {
        NumMetaDataKeysAvailable++;
    }
    if (iMP4FileHandle->getITunesImageData() != NULL)
    {
        NumMetaDataKeysAvailable++;
    }
    if (iMP4FileHandle->getITunesLyrics().get_size() > 0)
    {
        NumMetaDataKeysAvailable++;
    }
    if (iMP4FileHandle->getITunesNormalizationData().get_size() > 0)
    {
        NumMetaDataKeysAvailable++;
    }
    if (iMP4FileHandle->getITunesThisDiskNo() > 0)
    {
        NumMetaDataKeysAvailable++;
    }
    if (iMP4FileHandle->getITunesThisTrackNo() > 0)
    {
        NumMetaDataKeysAvailable++;
    }
    if (iMP4FileHandle->getITunesTool().get_size() > 0)
    {
        NumMetaDataKeysAvailable++;
    }
    if (iMP4FileHandle->getITunesTotalDisks() > 0)
    {
        NumMetaDataKeysAvailable++;
    }
    if (iMP4FileHandle->getITunesTotalTracks() > 0)
    {
        NumMetaDataKeysAvailable++;
    }
    if (iMP4FileHandle->getITunesWriter().get_size() > 0)
    {
        NumMetaDataKeysAvailable++;
    }


    NumMetaDataKeysAvailable++;

    int32 numtracks = iMP4FileHandle->getNumTracks();
    if (numtracks > 0)
    {
        NumMetaDataKeysAvailable += 8;
    }
    return NumMetaDataKeysAvailable;
}


PVMFStatus PVMFMP4FFParserNode::InitMetaData()
{
    MP4FFParserOriginalCharEnc charType;
    // Populate the available metadata keys based on what's available in the MP4 file
    if (iMP4FileHandle == NULL)
    {
        return PVMFErrNoResources;
    }
    int32 leavecode = 0;

    int32 AvailableMetaDataKeysCount = CountMetaDataKeys();
    OSCL_TRY(leavecode, iAvailableMetadataKeys.reserve(AvailableMetaDataKeysCount));

    int32 iNumTracks = iMP4FileHandle->getNumTracks();
    uint32 iIdList[16];

    if (iNumTracks != iMP4FileHandle->getTrackIDList(iIdList, iNumTracks))
    {
        return PVMFFailure;
    }
    for (int32 i = iNumTracks - 1; i >= 0; i--)
    {
        //track id is a one based index
        char indexparam[18];
        oscl_snprintf(indexparam, 18, ";index=%d", i);
        indexparam[17] = '\0';

        uint32 trackID = iIdList[i];

        OSCL_HeapString<OsclMemAllocator> trackMIMEType;

        iMP4FileHandle->getTrackMIMEType(trackID, (OSCL_String&)trackMIMEType);

        if ((oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000))) == 0)
        {
            PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_VIDEO_PROFILE_KEY, indexparam);
            PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_VIDEO_LEVEL_KEY, indexparam);
        }

        if ((oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_M4V, oscl_strlen(PVMF_MIME_M4V)) == 0) ||
                (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000)) == 0) ||
                (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H264_VIDEO_MP4, oscl_strlen(PVMF_MIME_H264_VIDEO_MP4)) == 0))
        {
            uint64 trackduration  = iMP4FileHandle->getTrackMediaDuration(trackID);
            uint32 samplecount = iMP4FileHandle->getSampleCountInTrack(trackID);

            MediaClockConverter mcc(iMP4FileHandle->getTrackMediaTimescale(trackID));
            mcc.update_clock(trackduration);
            uint32 TrackDurationInSec = mcc.get_converted_ts(1);
            uint32 frame_rate = 0;
            uint32 OverflowThreshold = PVMF_MP4_MAX_UINT32 / MILLISECOND_TIMESCALE;
            // If overflow could not happen, we calculate it in millisecond
            if (TrackDurationInSec < OverflowThreshold && samplecount < OverflowThreshold)
            {
                uint32 TrackDurationInMilliSec = mcc.get_converted_ts(MILLISECOND_TIMESCALE);
                if (TrackDurationInMilliSec > 0)
                {
                    frame_rate = samplecount * MILLISECOND_TIMESCALE / TrackDurationInMilliSec;
                }
            }
            else // if overflow could happen when calculate in millisecond, we calculate it in second
            {
                if (TrackDurationInSec > 0)
                {
                    frame_rate = samplecount / TrackDurationInSec;
                }
            }
            if (frame_rate > 0)
            {
                PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_FRAME_RATE_KEY, indexparam);
            }
            if (PVMFSuccess == PopulateVideoDimensions(trackID))
            {
                PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_VIDEO_WIDTH_KEY, indexparam);
                PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_VIDEO_HEIGHT_KEY, indexparam);
            }
            PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_VIDEO_FORMAT_KEY, indexparam);
        }

        if ((oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_MPEG4_AUDIO, oscl_strlen(PVMF_MIME_MPEG4_AUDIO)) == 0) ||
                (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_AMR, oscl_strlen(PVMF_MIME_AMR)) == 0) ||
                (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_AMR_IETF, oscl_strlen(PVMF_MIME_AMR_IETF)) == 0) ||
                (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_AMRWB_IETF, oscl_strlen(PVMF_MIME_AMRWB_IETF)) == 0))
        {
            PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_AUDIO_FORMAT_KEY, indexparam);
            PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_AUDIO_NUMCHANNELS_KEY, indexparam);
            PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_SAMPLERATE_KEY, indexparam);
            PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_AUDIO_BITS_PER_SAMPLE_KEY, indexparam);
        }
    }

    if (iMP4FileHandle->getNumAuthor() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_AUTHOR_KEY);
    }

    //Common Keys
    if (iMP4FileHandle->getNumAlbum() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_ALBUM_KEY);
    }
    if (iMP4FileHandle->getNumComment() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_COMMENT_KEY);
    }
    if (iMP4FileHandle->getNumGenre() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_GENRE_KEY);
    }
    if (iMP4FileHandle->getNumTitle() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_TITLE_KEY);
    }
    if (iMP4FileHandle->getNumCopyright() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_COPYRIGHT_KEY);
    }
    if (iMP4FileHandle->getNumYear() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_YEAR_KEY);
    }
    if (iMP4FileHandle->getNumArtist() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_ARTIST_KEY);
    }
    if (iMP4FileHandle->getNumDescription() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_DESCRIPTION_KEY);
    }

    if (iMP4FileHandle->getNumRating() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_RATING_KEY);
    }


    if (iMP4FileHandle->getNumAssetInfoLocationAtoms() > 0)
    {
        uint32 numLocations = iMP4FileHandle->getNumAssetInfoLocationAtoms();
        if (numLocations > 0)
        {
            //PushToAvailableMetadataKeysList(PVMP4METADATA_LOCATION_KEY);
            // Create the parameter string for the index range
            char indexparam[18];
            oscl_snprintf(indexparam, 18, ";index=0...%d", (numLocations - 1));
            indexparam[17] = '\0';

            PushToAvailableMetadataKeysList(PVMP4METADATA_LOCATION_KEY, indexparam);
        }

    }
    if (iMP4FileHandle->getNumAssetInfoKeyWordAtoms() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_KEYWORD_KEY);
    }
    if (iMP4FileHandle->getNumAssetInfoClassificationAtoms() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_CLASSIFICATION_KEY);
    }
    if (iMP4FileHandle->getCompatibiltyMajorBrand() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_MAJORBRAND_KEY);
    }

    if (iMP4FileHandle->getCompatibiltyList() != NULL)
    {
        if (iMP4FileHandle->getCompatibiltyList()->size() > 0)
        {
            PushToAvailableMetadataKeysList(PVMP4METADATA_COMPATIBLEBRAND_KEY);
        }
    }

    if (iMP4FileHandle->getPVVersion(charType).get_size() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_VERSION_KEY);
    }

    if (iMP4FileHandle->getCreationDate(charType).get_size() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_DATE_KEY);
    }

    if (iMP4FileHandle->getMovieDuration() > (uint64)0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_DURATION_KEY);
        // Intimate the Duration info available to the engine through Informational Event.
        uint64 duration64 = iMP4FileHandle->getMovieDuration();
        uint32 durationms = 0;
        uint32 duration = durationms = Oscl_Int64_Utils::get_uint64_lower32(duration64);
        uint32 timescale = iMP4FileHandle->getMovieTimescale();
        if (timescale > 0 && timescale != 1000)
        {
            // Convert to milliseconds
            MediaClockConverter mcc(timescale);
            mcc.update_clock(duration);
            durationms = mcc.get_converted_ts(1000);
        }
        CreateDurationInfoMsg(durationms);
    }

    if (iMP4FileHandle->getITunesBeatsPerMinute() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_TEMPO_KEY);
    }

    if (iMP4FileHandle->getITunesCDIdentifierData(0).get_size() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_CDDBID_KEY);
    }
    if (iMP4FileHandle->getITunesGroupData().get_size() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_GROUPING_KEY);
    }
    if (iMP4FileHandle->getITunesImageData() != NULL)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_COVER_KEY);
    }
    if (iMP4FileHandle->getITunesLyrics().get_size() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_LYRICS_KEY);
    }
    if (iMP4FileHandle->getITunesNormalizationData().get_size() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_FREEFORMDATA_KEY);
    }
    if (iMP4FileHandle->getITunesThisDiskNo() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_DISKDATA_KEY);
    }
    if (iMP4FileHandle->getITunesThisTrackNo() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKDATA_KEY);
    }
    if (iMP4FileHandle->getITunesTool().get_size() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_TOOL_KEY);
    }
    if (iMP4FileHandle->getITunesWriter().get_size() > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_WRITER_KEY);
    }
    if (iMP4FileHandle->IsITunesCompilationPart() != false)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_COMPILATION_KEY);
    }

    PushToAvailableMetadataKeysList(PVMP4METADATA_CLIP_TYPE_KEY);

    PushToAvailableMetadataKeysList(PVMP4METADATA_RANDOM_ACCESS_DENIED_KEY);

    PushToAvailableMetadataKeysList(PVMP4METADATA_IS_MOOF_KEY);

    int32 numtracks = iMP4FileHandle->getNumTracks();
    if (numtracks > 0)
    {
        PushToAvailableMetadataKeysList(PVMP4METADATA_NUMTRACKS_KEY);

        //Create the parameter string for the index range
        char indexparam[18];
        oscl_snprintf(indexparam, 18, ";index=0...%d", (numtracks - 1));
        indexparam[17] = '\0';

        PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_TYPE_KEY, indexparam);

        PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_TRACKID_KEY, indexparam);

        PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_DURATION_KEY, indexparam);

        PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_BITRATE_KEY, indexparam);

        PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_SAMPLECOUNT_KEY, indexparam);

        PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_SELECTED_KEY, indexparam);

        if (iMP4FileHandle->getITunesThisTrackNo() > 0)
        {
            PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_TRACK_NUMBER_KEY, indexparam);
        }

        PushToAvailableMetadataKeysList(PVMP4METADATA_TRACKINFO_NUM_KEY_SAMPLES_KEY, indexparam);
    }

    //set clip duration on download progress interface
    //applicable to PDL sessions
    {
        if (iMP4FileHandle != NULL)
        {
            MediaClockConverter mcc(iMP4FileHandle->getMovieTimescale());
            uint32 movieduration =
                Oscl_Int64_Utils::get_uint64_lower32(iMP4FileHandle->getMovieDuration());
            mcc.update_clock(movieduration);
            uint32 moviedurationInMS = mcc.get_converted_ts(1000);
            if ((download_progress_interface != NULL) && (moviedurationInMS != 0))
            {
                download_progress_interface->setClipDuration(OSCL_CONST_CAST(uint32, moviedurationInMS));
            }
        }
    }
    return PVMFSuccess;
}

PVMFStatus
PVMFMP4FFParserNode::DoGetMetadataKeys(PVMFMP4FFParserNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP4FFParserNode::DoGetMetadataKeys() In"));
    /* Get Metadata keys from CPM for protected content only */
    if ((iCPMMetaDataExtensionInterface != NULL) &&
            (iProtectedFile == true))
    {
        GetCPMMetaDataKeys();
        return PVMFPending;
    }
    if (iMP4FileHandle == NULL)
    {
        return PVMFErrInvalidState;
    }
    return (CompleteGetMetadataKeys(aCmd));
}

PVMFStatus PVMFMP4FFParserNode::CompleteGetMetadataKeys(PVMFMP4FFParserNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP4FFParserNode::CompleteGetMetadataKeys() In"));

    PVMFMetadataList* keylistptr = NULL;
    uint32 starting_index;
    int32 max_entries;
    char* query_key = NULL;

    aCmd.PVMFMP4FFParserNodeCommand::Parse(keylistptr, starting_index, max_entries, query_key);

    // Check parameters
    if (keylistptr == NULL)
    {
        // The list pointer is invalid
        return PVMFErrArgument;
    }

    if ((starting_index > (iAvailableMetadataKeys.size() - 1)) || max_entries == 0)
    {
        // Invalid starting index and/or max entries
        return PVMFErrArgument;
    }

    // Copy the requested keys
    uint32 num_entries = 0;
    int32 num_added = 0;
    uint32 lcv = 0;
    for (lcv = 0; lcv < iCPMMetadataKeys.size(); lcv++)
    {
        if (query_key == NULL)
        {
            /* No query key so this key is counted */
            ++num_entries;
            if (num_entries > starting_index)
            {
                /* Past the starting index so copy the key */
                PVMFStatus status = PushValueToList(iCPMMetadataKeys, keylistptr, lcv);
                if (PVMFErrNoMemory == status)
                {
                    return status;
                }
                num_added++;
            }
        }
        else
        {
            /* Check if the key matches the query key */
            if (pv_mime_strcmp(iCPMMetadataKeys[lcv].get_cstr(), query_key) >= 0)
            {
                /* This key is counted */
                ++num_entries;
                if (num_entries > starting_index)
                {
                    /* Past the starting index so copy the key */
                    PVMFStatus status = PushValueToList(iCPMMetadataKeys, keylistptr, lcv);
                    if (PVMFErrNoMemory == status)
                    {
                        return status;
                    }
                    num_added++;
                }
            }
        }
        /* Check if max number of entries have been copied */
        if ((max_entries > 0) && (num_added >= max_entries))
        {
            break;
        }
    }
    for (lcv = 0; lcv < iAvailableMetadataKeys.size(); lcv++)
    {
        if (query_key == NULL)
        {
            // No query key so this key is counted
            ++num_entries;
            if (num_entries > starting_index)
            {
                // Past the starting index so copy the key
                PVMFStatus status = PushValueToList(iAvailableMetadataKeys, keylistptr, lcv);
                if (PVMFErrNoMemory == status)
                {
                    return status;
                }
                num_added++;
            }
        }
        else
        {
            // Check if the key matche the query key
            if (pv_mime_strcmp(iAvailableMetadataKeys[lcv].get_cstr(), query_key) >= 0)
            {
                // This key is counted
                ++num_entries;
                if (num_entries > starting_index)
                {
                    // Past the starting index so copy the key
                    PVMFStatus status = PushValueToList(iAvailableMetadataKeys, keylistptr, lcv);
                    if (PVMFErrNoMemory == status)
                    {
                        return status;
                    }
                    num_added++;
                }
            }
        }

        // Check if max number of entries have been copied
        if (max_entries > 0 && num_added >= max_entries)
        {
            break;
        }
    }
    return PVMFSuccess;
}


PVMFStatus PVMFMP4FFParserNode::DoGetMetadataValues(PVMFMP4FFParserNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP4FFParserNode::DoGetMetadataValues() In"));

    PVMFMetadataList* keylistptr_in = NULL;
    PVMFMetadataList* keylistptr = NULL;
    OSCL_wHeapString<OsclMemAllocator> valuestring = NULL;
    Oscl_Vector<PvmiKvp, OsclMemAllocator>* valuelistptr = NULL;
    uint32 starting_index;
    int32 max_entries;
    MP4FFParserOriginalCharEnc charType = ORIGINAL_CHAR_TYPE_UNKNOWN;
    uint16 iLangCode = 0;
    aCmd.PVMFMP4FFParserNodeCommand::Parse(keylistptr_in,
                                           valuelistptr,
                                           starting_index,
                                           max_entries);

    // Check the parameters
    if (keylistptr_in == NULL || valuelistptr == NULL)
    {
        return PVMFErrArgument;
    }

    keylistptr = keylistptr_in;
    //If numkeys is one, just check to see if the request
    //is for ALL metadata
    if (keylistptr_in->size() == 1)
    {
        if (oscl_strncmp((*keylistptr)[0].get_cstr(),
                         PVMP4_ALL_METADATA_KEY,
                         oscl_strlen(PVMP4_ALL_METADATA_KEY)) == 0)
        {
            //use the complete metadata key list
            keylistptr = &iAvailableMetadataKeys;
        }
    }
    uint32 numkeys = keylistptr->size();

    if (starting_index > (numkeys - 1) || numkeys <= 0 || max_entries == 0)
    {
        // Don't do anything
        return PVMFErrArgument;
    }

    uint32 numvalentries = 0;
    int32 numentriesadded = 0;
    uint32 lcv = 0;

    if (iMP4FileHandle != NULL)
    {
        // Retrieve the track ID list
        OsclExclusiveArrayPtr<uint32> trackidlistexclusiveptr;
        uint32* trackidlist = NULL;
        uint32 numTracks = (uint32)(iMP4FileHandle->getNumTracks());
        PVMFStatus status = CreateNewArray(&trackidlist, numTracks);
        if (PVMFErrNoMemory == status)
        {
            return PVMFErrNoMemory;
        }
        oscl_memset(trackidlist, 0, sizeof(uint32)*(numTracks));
        iMP4FileHandle->getTrackIDList(trackidlist, numTracks);
        trackidlistexclusiveptr.set(trackidlist);


        for (lcv = 0; lcv < numkeys; lcv++)
        {
            int32 leavecode = 0;
            PvmiKvp KeyVal;
            KeyVal.key = NULL;
            KeyVal.value.pWChar_value = NULL;
            KeyVal.value.pChar_value = NULL;
            int32 idx = 0;
            char orig_char_enc[2][7] = {"UTF-8", "UTF-16"};

            // const char *x = (*keylistptr)[lcv].get_cstr();
            bool IsMetadataValAddedBefore = false;

            if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_AUTHOR_KEY) == 0)
            {
                // Author

                uint32 countAuthor = 0;
                countAuthor = iMP4FileHandle->getNumAuthor();

                if (countAuthor > 0)
                {
                    for (idx = 0; idx < (int32)countAuthor ; idx++)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;

                        // Create a value entry if past the starting index
                        if (numvalentries > starting_index)
                        {

                            if (!iMP4FileHandle->getAuthor(idx, valuestring, iLangCode, charType))
                            {
                                PVMF_MP4FFPARSERNODE_LOGERROR((0, "PVMFMP4FFParserNode::DoGetMetadataValues - getAuthor Failed"));
                                return PVMFFailure;
                            }


                            char lang_param[43];
                            if (iLangCode != 0)
                            {
                                int8 LangCode[4];
                                getLanguageCode(iLangCode, LangCode);
                                LangCode[3] = '\0';
                                oscl_snprintf(lang_param, 20, ";%s%s", PVMP4METADATA_LANG_CODE, LangCode);
                                lang_param[20] = '\0';
                            }
                            else
                            {
                                lang_param[0] = '\0';
                            }
                            KeyVal.key = NULL;
                            KeyVal.value.pWChar_value = NULL;
                            KeyVal.value.pChar_value = NULL;
                            if (charType != ORIGINAL_CHAR_TYPE_UNKNOWN)
                            {
                                char char_enc_param[22];
                                oscl_snprintf(char_enc_param, 22, ";%s%s", PVMP4METADATA_ORIG_CHAR_ENC, orig_char_enc[charType-1]);
                                char_enc_param[21] = '\0';
                                oscl_strncat(lang_param, char_enc_param, oscl_strlen(char_enc_param));
                            }
                            PVMFStatus retval =
                                PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                        PVMP4METADATA_AUTHOR_KEY,
                                        valuestring,
                                        lang_param);
                            if (retval != PVMFSuccess && retval != PVMFErrArgument)
                            {
                                break;
                            }
                            // Add the KVP to the list if the key string was created
                            if (KeyVal.key != NULL)
                            {
                                leavecode = AddToValueList(*valuelistptr, KeyVal);
                                if (leavecode != 0)
                                {
                                    if (KeyVal.value.pWChar_value != NULL)
                                    {
                                        OSCL_ARRAY_DELETE(KeyVal.value.pWChar_value);
                                        KeyVal.value.pWChar_value = NULL;
                                    }

                                    OSCL_ARRAY_DELETE(KeyVal.key);
                                    KeyVal.key = NULL;
                                }
                                else
                                {
                                    // Increment the value list entry counter
                                    ++numentriesadded;
                                    IsMetadataValAddedBefore = true;
                                }

                                // Check if the max number of value entries were added
                                if (max_entries > 0 && numentriesadded >= max_entries)
                                {
                                    iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                    return PVMFSuccess;
                                }
                            }

                        }
                    }

                }
            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TOOL_KEY) == 0)
            {

                KeyVal.key = NULL;
                KeyVal.value.pWChar_value = NULL;
                KeyVal.value.pChar_value = NULL;

                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {

                    OSCL_wHeapString<OsclMemAllocator> valuestring = iMP4FileHandle->getITunesTool();
                    PVMFStatus retval =
                        PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                PVMP4METADATA_TOOL_KEY,
                                valuestring);

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    else
                    {
                        IsMetadataValAddedBefore = false;

                    }

                }

            }

            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_WRITER_KEY) == 0)
            {
                KeyVal.key = NULL;
                KeyVal.value.pWChar_value = NULL;
                KeyVal.value.pChar_value = NULL;
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {


                    OSCL_wHeapString<OsclMemAllocator> valuestring = iMP4FileHandle->getITunesWriter();
                    PVMFStatus retval =
                        PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                PVMP4METADATA_WRITER_KEY,
                                valuestring);

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    else
                    {
                        IsMetadataValAddedBefore = false;

                    }

                }
            }

            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_GROUPING_KEY) == 0)
            {


                KeyVal.key = NULL;
                KeyVal.value.pWChar_value = NULL;
                KeyVal.value.pChar_value = NULL;

                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {


                    OSCL_wHeapString<OsclMemAllocator> valuestring = iMP4FileHandle->getITunesGroupData();
                    PVMFStatus retval =
                        PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                PVMP4METADATA_GROUPING_KEY,
                                valuestring);

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    else
                    {
                        IsMetadataValAddedBefore = false;

                    }

                }

            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKDATA_KEY) == 0)
            {

                KeyVal.key = NULL;
                KeyVal.value.pWChar_value = NULL;
                KeyVal.value.pChar_value = NULL;
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {

                    uint32 numtracks = iMP4FileHandle->getITunesThisTrackNo();
                    PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal, PVMP4METADATA_TRACKDATA_KEY, numtracks);


                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    else
                    {

                        IsMetadataValAddedBefore = false;

                    }

                }

            }

            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_COMPILATION_KEY) == 0)
            {

                KeyVal.key = NULL;
                KeyVal.value.pWChar_value = NULL;
                KeyVal.value.pChar_value = NULL;

                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {


                    bool compilationPart = iMP4FileHandle->IsITunesCompilationPart();
                    PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForBoolValue(KeyVal, PVMP4METADATA_COMPILATION_KEY, compilationPart);

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    else
                    {

                        IsMetadataValAddedBefore = false;

                    }

                }

            }

            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TEMPO_KEY) == 0)
            {

                KeyVal.key = NULL;
                KeyVal.value.pWChar_value = NULL;
                KeyVal.value.pChar_value = NULL;

                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {

                    uint32 beatsperminute = iMP4FileHandle->getITunesBeatsPerMinute();
                    PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal, PVMP4METADATA_TEMPO_KEY, beatsperminute);


                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    else
                    {

                        IsMetadataValAddedBefore = false;

                    }

                }

            }


            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_COVER_KEY) == 0)
            {

                KeyVal.key = NULL;
                KeyVal.value.pWChar_value = NULL;
                KeyVal.value.pChar_value = NULL;

                // Increment the counter for the number of values found so far

                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {

                    PvmfApicStruct*  imagedata = iMP4FileHandle->getITunesImageData();
                    PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForKSVValue(KeyVal, PVMP4METADATA_COVER_KEY, OSCL_STATIC_CAST(OsclAny*, imagedata));

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    else
                    {

                        IsMetadataValAddedBefore = false;

                    }

                }

            }

            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_DISKDATA_KEY) == 0)
            {

                KeyVal.key = NULL;
                KeyVal.value.pWChar_value = NULL;
                KeyVal.value.pChar_value = NULL;

                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {


                    uint32 disknum = iMP4FileHandle->getITunesThisDiskNo();
                    PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal, PVMP4METADATA_DISKDATA_KEY, disknum);



                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    else
                    {

                        IsMetadataValAddedBefore = false;

                    }

                }

            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_FREEFORMDATA_KEY) == 0)
            {

                KeyVal.key = NULL;
                KeyVal.value.pWChar_value = NULL;
                KeyVal.value.pChar_value = NULL;

                // Increment the counter for the number of values found so far
                ++numvalentries;
                PVMFStatus retval;
                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {
                    OSCL_wHeapString<OsclMemAllocator> valuestring = iMP4FileHandle->getITunesNormalizationData();
                    if (valuestring.get_size() > 0)
                    {
                        retval =
                            PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                    PVMP4METADATA_FREEFORMDATA_KEY,
                                    valuestring);
                    }
                    else
                    {
                        OSCL_wHeapString<OsclMemAllocator> cdidentifierstring = iMP4FileHandle->getITunesCDIdentifierData(0);
                        retval =
                            PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                    PVMP4METADATA_FREEFORMDATA_KEY,
                                    cdidentifierstring);
                    }
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    else
                    {

                        IsMetadataValAddedBefore = false;

                    }

                }

            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_IS_MOOF_KEY) == 0)
            {
                /*
                 * is-moof
                 * Increment the counter for the number of values found so far
                 */
                ++numvalentries;

                /* Create a value entry if past the starting index */
                if (numvalentries > (uint32)starting_index)
                {
                    bool is_movie_fragmnent_present = iMP4FileHandle->IsMovieFragmentsPresent();

                    PVMFStatus retval =
                        PVMFCreateKVPUtils::CreateKVPForBoolValue(KeyVal,
                                PVMP4METADATA_IS_MOOF_KEY,
                                is_movie_fragmnent_present,
                                NULL);





                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                }
            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_RANDOM_ACCESS_DENIED_KEY) == 0)
            {
                /*
                 * Random Access
                 * Increment the counter for the number of values found so far
                 */
                ++numvalentries;

                /* Create a value entry if past the starting index */
                if (numvalentries > (uint32)starting_index)
                {
                    uint64 duration64 = iMP4FileHandle->getMovieDuration();
                    uint32 duration = Oscl_Int64_Utils::get_uint64_lower32(duration64);
                    bool random_access_denied = false;
                    if (duration > 0)
                    {
                        random_access_denied = false;
                    }
                    else
                    {
                        random_access_denied = true;
                    }

                    if (iMP4FileHandle->IsMovieFragmentsPresent())
                    {
                        if (iDataStreamInterface != NULL)
                            random_access_denied = true;

                        uint32* trackList = NULL;
                        uint32 numTracks = iNodeTrackPortList.size();
                        CreateNewArray(&trackList, numTracks);
                        if (trackList)
                        {
                            for (uint32 i = 0; i < iNodeTrackPortList.size(); i++)
                            {
                                // Save the track list while in this loop
                                trackList[i] = iNodeTrackPortList[i].iTrackId;
                            }

                            if (!iMP4FileHandle->IsTFRAPresentForAllTrack(numTracks, trackList))
                                random_access_denied = true;

                            OSCL_ARRAY_DELETE(trackList);
                        }
                    }

                    PVMFStatus retval =
                        PVMFCreateKVPUtils::CreateKVPForBoolValue(KeyVal,
                                PVMP4METADATA_RANDOM_ACCESS_DENIED_KEY,
                                random_access_denied,
                                NULL);
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                }
            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_LYRICS_KEY) == 0)
            {

                KeyVal.key = NULL;
                KeyVal.value.pWChar_value = NULL;
                KeyVal.value.pChar_value = NULL;

                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {

                    OSCL_wHeapString<OsclMemAllocator> valuestring = iMP4FileHandle->getITunesLyrics();
                    PVMFStatus retval =
                        PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                PVMP4METADATA_LYRICS_KEY,
                                valuestring);

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    else
                    {

                        IsMetadataValAddedBefore = false;

                    }

                }

            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_CLIP_TYPE_KEY) == 0)
            {
                // clip-type
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {
                    uint32 len = 0;
                    char* clipType = NULL;
                    if (download_progress_interface != NULL)
                    {
                        len = oscl_strlen("download");
                        clipType = OSCL_ARRAY_NEW(char, len + 1);
                        oscl_memset(clipType, 0, len + 1);
                        oscl_strncpy(clipType, ("download"), len);
                    }
                    else
                    {
                        len = oscl_strlen("local");
                        clipType = OSCL_ARRAY_NEW(char, len + 1);
                        oscl_memset(clipType, 0, len + 1);
                        oscl_strncpy(clipType, ("local"), len);
                    }

                    PVMFStatus retval =
                        PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                                PVMP4METADATA_CLIP_TYPE_KEY,
                                clipType);

                    OSCL_ARRAY_DELETE(clipType);
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                }
            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_ALBUM_KEY) == 0)
            {
                // Album

                uint32 countAlbum = 0;
                countAlbum = iMP4FileHandle->getNumAlbum();

                if (countAlbum > 0)
                {
                    for (idx = 0; idx < (int32)countAlbum ; idx++)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;

                        // Create a value entry if past the starting index
                        if (numvalentries > starting_index)
                        {

                            if (iMP4FileHandle->getAlbum(idx, valuestring, iLangCode, charType) != PVMFErrArgument)
                            {

                                char lang_param[43];
                                if (iLangCode != 0)
                                {
                                    int8 LangCode[4];
                                    getLanguageCode(iLangCode, LangCode);
                                    LangCode[3] = '\0';
                                    oscl_snprintf(lang_param, 20, ";%s%s", PVMP4METADATA_LANG_CODE, LangCode);
                                    lang_param[20] = '\0';
                                }
                                else
                                {
                                    lang_param[0] = '\0';
                                }
                                KeyVal.key = NULL;
                                KeyVal.value.pWChar_value = NULL;
                                KeyVal.value.pChar_value = NULL;
                                if (charType != ORIGINAL_CHAR_TYPE_UNKNOWN)
                                {
                                    char char_enc_param[22];
                                    oscl_snprintf(char_enc_param, 22, ";%s%s", PVMP4METADATA_ORIG_CHAR_ENC, orig_char_enc[charType-1]);
                                    char_enc_param[21] = '\0';
                                    oscl_strncat(lang_param, char_enc_param, oscl_strlen(char_enc_param));
                                }


                                PVMFStatus retval =
                                    PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                            PVMP4METADATA_ALBUM_KEY,
                                            valuestring,
                                            lang_param);
                                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                                {
                                    break;
                                }
                                // Add the KVP to the list if the key string was created
                                if (KeyVal.key != NULL)
                                {
                                    leavecode = AddToValueList(*valuelistptr, KeyVal);
                                    if (leavecode != 0)
                                    {
                                        if (KeyVal.value.pWChar_value != NULL)
                                        {
                                            OSCL_ARRAY_DELETE(KeyVal.value.pWChar_value);
                                            KeyVal.value.pWChar_value = NULL;
                                        }

                                        OSCL_ARRAY_DELETE(KeyVal.key);
                                        KeyVal.key = NULL;
                                    }
                                    else
                                    {
                                        // Increment the value list entry counter
                                        ++numentriesadded;
                                        IsMetadataValAddedBefore = true;
                                    }

                                    // Check if the max number of value entries were added
                                    if (max_entries > 0 && numentriesadded >= max_entries)
                                    {
                                        iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                        return PVMFSuccess;
                                    }
                                }
                            }

                        }
                    }

                }
            }

            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_COMMENT_KEY) == 0)
            {
                // Comment

                uint32 countComment = 0;
                countComment = iMP4FileHandle->getNumComment();

                if (countComment > 0)
                {
                    for (idx = 0; idx < (int32)countComment ; idx++)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;

                        // Create a value entry if past the starting index
                        if (numvalentries > starting_index)
                        {

                            if (iMP4FileHandle->getComment(idx, valuestring, iLangCode, charType) != PVMFErrArgument)
                            {

                                char lang_param[43];
                                if (iLangCode != 0)
                                {
                                    int8 LangCode[4];
                                    getLanguageCode(iLangCode, LangCode);
                                    LangCode[3] = '\0';
                                    oscl_snprintf(lang_param, 20, ";%s%s", PVMP4METADATA_LANG_CODE, LangCode);
                                    lang_param[20] = '\0';
                                }
                                else
                                {
                                    lang_param[0] = '\0';
                                }
                                KeyVal.key = NULL;
                                KeyVal.value.pWChar_value = NULL;
                                KeyVal.value.pChar_value = NULL;
                                if (charType != ORIGINAL_CHAR_TYPE_UNKNOWN)
                                {
                                    char char_enc_param[22];
                                    oscl_snprintf(char_enc_param, 22, ";%s%s", PVMP4METADATA_ORIG_CHAR_ENC, orig_char_enc[charType-1]);
                                    char_enc_param[21] = '\0';
                                    oscl_strncat(lang_param, char_enc_param, oscl_strlen(char_enc_param));
                                }
                                PVMFStatus retval =
                                    PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                            PVMP4METADATA_COMMENT_KEY,
                                            valuestring,
                                            lang_param);
                                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                                {
                                    break;
                                }
                                // Add the KVP to the list if the key string was created
                                if (KeyVal.key != NULL)
                                {
                                    leavecode = AddToValueList(*valuelistptr, KeyVal);
                                    if (leavecode != 0)
                                    {
                                        if (KeyVal.value.pWChar_value != NULL)
                                        {
                                            OSCL_ARRAY_DELETE(KeyVal.value.pWChar_value);
                                            KeyVal.value.pWChar_value = NULL;
                                        }

                                        OSCL_ARRAY_DELETE(KeyVal.key);
                                        KeyVal.key = NULL;
                                    }
                                    else
                                    {
                                        // Increment the value list entry counter
                                        ++numentriesadded;
                                        IsMetadataValAddedBefore = true;
                                    }

                                    // Check if the max number of value entries were added
                                    if (max_entries > 0 && numentriesadded >= max_entries)
                                    {
                                        iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                        return PVMFSuccess;
                                    }
                                }
                            }

                        }
                    }

                }
            }
            else if ((oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_LOCATION_KEY) != NULL))
            {
                /* Location */
                /* Determine the index requested. Default to all pictures */
                uint32 startindex = 0;
                int32 numLocationRecords = 0;
                numLocationRecords = iMP4FileHandle->getNumAssetInfoLocationAtoms();

                uint32 endindex = numLocationRecords - 1;

                /* Check if the index parameter is present */
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    /* Retrieve the index values */
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                /* Validate the indices */
                if (startindex > endindex || (int32)startindex >= numLocationRecords || (int32)endindex >= numLocationRecords)
                {
                    break;
                }
                PvmfAssetInfo3GPPLocationStruct* pLocationRecord;

                /* Return a KVP for each index */
                for (uint32 cnt = startindex; (int32)cnt < numLocationRecords; cnt++)
                {
                    pLocationRecord = iMP4FileHandle->getAssetInfoLocationStruct(cnt);
                    char indexparam[29];

                    oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, cnt);
                    indexparam[15] = '\0';

                    PvmiKvp KeyVal;
                    KeyVal.key = NULL;
                    /* Increment the counter for the number of values found so far */
                    ++numvalentries;
                    /* Add the value entry if past the starting index */
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {

                        retval = PVMFCreateKVPUtils::CreateKVPForKSVValue(KeyVal,
                                 PVMP4METADATA_LOCATION_KEY,
                                 OSCL_STATIC_CAST(OsclAny*, pLocationRecord),
                                 indexparam);
                        if (retval != PVMFSuccess && retval != PVMFErrArgument)
                        {
                            break;
                        }
                    }
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    if (KeyVal.key != NULL)
                    {
                        PVMFStatus status = PushKVPToMetadataValueList(valuelistptr, KeyVal);
                        if (status != PVMFSuccess)
                        {
                            return status;
                        }
                        // Increment the counter for number of value entries added to the list
                        ++numentriesadded;
                        IsMetadataValAddedBefore = true;

                        /* Check if the max number of value entries were added */
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }

                }
            }

            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TITLE_KEY) == 0)
            {
                // Title

                uint32 countTitle = 0;
                countTitle = iMP4FileHandle->getNumTitle();

                if (countTitle > 0)
                {
                    for (idx = 0; idx < (int32)countTitle ; idx++)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;

                        // Create a value entry if past the starting index
                        if (numvalentries > starting_index)
                        {

                            if (iMP4FileHandle->getTitle(idx, valuestring, iLangCode, charType) != PVMFErrArgument)
                            {



                                char lang_param[43];
                                if (iLangCode != 0)
                                {
                                    int8 LangCode[4];
                                    getLanguageCode(iLangCode, LangCode);
                                    LangCode[3] = '\0';
                                    oscl_snprintf(lang_param, 20, ";%s%s", PVMP4METADATA_LANG_CODE, LangCode);
                                    lang_param[20] = '\0';
                                }
                                else
                                {
                                    lang_param[0] = '\0';
                                }
                                KeyVal.key = NULL;
                                KeyVal.value.pWChar_value = NULL;
                                KeyVal.value.pChar_value = NULL;
                                if (charType != ORIGINAL_CHAR_TYPE_UNKNOWN)
                                {
                                    char char_enc_param[22];
                                    oscl_snprintf(char_enc_param, 22, ";%s%s", PVMP4METADATA_ORIG_CHAR_ENC, orig_char_enc[charType-1]);
                                    char_enc_param[21] = '\0';
                                    oscl_strncat(lang_param, char_enc_param, oscl_strlen(char_enc_param));
                                }
                                PVMFStatus retval =
                                    PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                            PVMP4METADATA_TITLE_KEY,
                                            valuestring,
                                            lang_param);
                                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                                {
                                    break;
                                }
                                // Add the KVP to the list if the key string was created
                                if (KeyVal.key != NULL)
                                {
                                    leavecode = AddToValueList(*valuelistptr, KeyVal);
                                    if (leavecode != 0)
                                    {
                                        if (KeyVal.value.pWChar_value != NULL)
                                        {
                                            OSCL_ARRAY_DELETE(KeyVal.value.pWChar_value);
                                            KeyVal.value.pWChar_value = NULL;
                                        }

                                        OSCL_ARRAY_DELETE(KeyVal.key);
                                        KeyVal.key = NULL;
                                    }
                                    else
                                    {
                                        // Increment the value list entry counter
                                        ++numentriesadded;
                                        IsMetadataValAddedBefore = true;
                                    }

                                    // Check if the max number of value entries were added
                                    if (max_entries > 0 && numentriesadded >= max_entries)
                                    {
                                        iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                        return PVMFSuccess;
                                    }
                                }
                            }

                        }
                    }

                }
            }

            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_DESCRIPTION_KEY) == 0)
            {
                // Description

                uint32 countDescription = 0;
                countDescription = iMP4FileHandle->getNumDescription();

                if (countDescription > 0)
                {
                    for (idx = 0; idx < (int32)countDescription ; idx++)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;

                        // Create a value entry if past the starting index
                        if (numvalentries > starting_index)
                        {

                            if (iMP4FileHandle->getDescription(idx, valuestring, iLangCode, charType) != PVMFErrArgument)
                            {


                                char lang_param[43];
                                if (iLangCode != 0)
                                {
                                    int8 LangCode[4];
                                    getLanguageCode(iLangCode, LangCode);
                                    LangCode[3] = '\0';
                                    oscl_snprintf(lang_param, 20, ";%s%s", PVMP4METADATA_LANG_CODE, LangCode);
                                    lang_param[20] = '\0';
                                }
                                else
                                {
                                    lang_param[0] = '\0';
                                }
                                KeyVal.key = NULL;
                                KeyVal.value.pWChar_value = NULL;
                                KeyVal.value.pChar_value = NULL;
                                if (charType != ORIGINAL_CHAR_TYPE_UNKNOWN)
                                {
                                    char char_enc_param[22];
                                    oscl_snprintf(char_enc_param, 22, ";%s%s", PVMP4METADATA_ORIG_CHAR_ENC, orig_char_enc[charType-1]);
                                    char_enc_param[21] = '\0';
                                    oscl_strncat(lang_param, char_enc_param, oscl_strlen(char_enc_param));
                                }
                                PVMFStatus retval =
                                    PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                            PVMP4METADATA_DESCRIPTION_KEY,
                                            valuestring,
                                            lang_param);
                                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                                {
                                    break;
                                }
                                // Add the KVP to the list if the key string was created
                                if (KeyVal.key != NULL)
                                {
                                    leavecode = AddToValueList(*valuelistptr, KeyVal);
                                    if (leavecode != 0)
                                    {
                                        if (KeyVal.value.pWChar_value != NULL)
                                        {
                                            OSCL_ARRAY_DELETE(KeyVal.value.pWChar_value);
                                            KeyVal.value.pWChar_value = NULL;
                                        }

                                        OSCL_ARRAY_DELETE(KeyVal.key);
                                        KeyVal.key = NULL;
                                    }
                                    else
                                    {
                                        // Increment the value list entry counter
                                        ++numentriesadded;
                                        IsMetadataValAddedBefore = true;
                                    }

                                    // Check if the max number of value entries were added
                                    if (max_entries > 0 && numentriesadded >= max_entries)
                                    {
                                        iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                        return PVMFSuccess;
                                    }
                                }
                            }

                        }
                    }

                }
            }

            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_RATING_KEY) == 0)
            {
                // Rating

                uint32 countRating = iMP4FileHandle->getNumRating();

                if (countRating > 0)
                {
                    for (idx = 0; idx < (int32)countRating ; idx++)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;

                        // Create a value entry if past the starting index
                        if (numvalentries > starting_index)
                        {

                            if (iMP4FileHandle->getRating(idx, valuestring, iLangCode, charType) != PVMFErrArgument)
                            {

                                char lang_param[43];
                                if (iLangCode != 0)
                                {
                                    int8 LangCode[4];
                                    getLanguageCode(iLangCode, LangCode);
                                    LangCode[3] = '\0';
                                    oscl_snprintf(lang_param, 20, ";%s%s", PVMP4METADATA_LANG_CODE, LangCode);
                                    lang_param[20] = '\0';
                                }
                                else
                                {
                                    lang_param[0] = '\0';
                                }
                                KeyVal.key = NULL;
                                KeyVal.value.pWChar_value = NULL;
                                KeyVal.value.pChar_value = NULL;
                                if (charType != ORIGINAL_CHAR_TYPE_UNKNOWN)
                                {
                                    char char_enc_param[22];
                                    oscl_snprintf(char_enc_param, 22, ";%s%s", PVMP4METADATA_ORIG_CHAR_ENC, orig_char_enc[charType-1]);
                                    char_enc_param[21] = '\0';
                                    oscl_strncat(lang_param, char_enc_param, oscl_strlen(char_enc_param));
                                }
                                PVMFStatus retval =
                                    PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                            PVMP4METADATA_RATING_KEY,
                                            valuestring,
                                            lang_param);
                                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                                {
                                    break;
                                }
                                // Add the KVP to the list if the key string was created
                                if (KeyVal.key != NULL)
                                {
                                    leavecode = AddToValueList(*valuelistptr, KeyVal);
                                    if (leavecode != 0)
                                    {
                                        if (KeyVal.value.pWChar_value != NULL)
                                        {
                                            OSCL_ARRAY_DELETE(KeyVal.value.pWChar_value);
                                            KeyVal.value.pWChar_value = NULL;
                                        }

                                        OSCL_ARRAY_DELETE(KeyVal.key);
                                        KeyVal.key = NULL;
                                    }
                                    else
                                    {
                                        // Increment the value list entry counter
                                        ++numentriesadded;
                                        IsMetadataValAddedBefore = true;
                                    }

                                    // Check if the max number of value entries were added
                                    if (max_entries > 0 && numentriesadded >= max_entries)
                                    {
                                        iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                        return PVMFSuccess;
                                    }
                                }
                            }

                        } //End of Outer If
                    }

                }
            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_COPYRIGHT_KEY) == 0)
            {
                // Copyright

                uint32 countCopyright = 0;
                countCopyright = iMP4FileHandle->getNumCopyright();

                if (countCopyright > 0)
                {
                    for (idx = 0; idx < (int32)countCopyright ; idx++)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;

                        // Create a value entry if past the starting index
                        if (numvalentries > starting_index)
                        {

                            if (iMP4FileHandle->getCopyright(idx, valuestring, iLangCode, charType) != PVMFErrArgument)
                            {
                                char lang_param[43];
                                if (iLangCode != 0)
                                {
                                    int8 LangCode[4];
                                    getLanguageCode(iLangCode, LangCode);
                                    LangCode[3] = '\0';
                                    oscl_snprintf(lang_param, 20, ";%s%s", PVMP4METADATA_LANG_CODE, LangCode);
                                    lang_param[20] = '\0';
                                }
                                else
                                {
                                    lang_param[0] = '\0';
                                }
                                KeyVal.key = NULL;
                                KeyVal.value.pWChar_value = NULL;
                                KeyVal.value.pChar_value = NULL;
                                if (charType != ORIGINAL_CHAR_TYPE_UNKNOWN)
                                {
                                    char char_enc_param[22];
                                    oscl_snprintf(char_enc_param, 22, ";%s%s", PVMP4METADATA_ORIG_CHAR_ENC, orig_char_enc[charType-1]);
                                    char_enc_param[21] = '\0';
                                    oscl_strncat(lang_param, char_enc_param, oscl_strlen(char_enc_param));
                                }
                                PVMFStatus retval =
                                    PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                            PVMP4METADATA_COPYRIGHT_KEY,
                                            valuestring,
                                            lang_param);
                                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                                {
                                    break;
                                }
                                // Add the KVP to the list if the key string was created
                                if (KeyVal.key != NULL)
                                {
                                    leavecode = AddToValueList(*valuelistptr, KeyVal);
                                    if (leavecode != 0)
                                    {
                                        if (KeyVal.value.pWChar_value != NULL)
                                        {
                                            OSCL_ARRAY_DELETE(KeyVal.value.pWChar_value);
                                            KeyVal.value.pWChar_value = NULL;
                                        }

                                        OSCL_ARRAY_DELETE(KeyVal.key);
                                        KeyVal.key = NULL;
                                    }
                                    else
                                    {
                                        // Increment the value list entry counter
                                        ++numentriesadded;
                                        IsMetadataValAddedBefore = true;
                                    }

                                    // Check if the max number of value entries were added
                                    if (max_entries > 0 && numentriesadded >= max_entries)
                                    {
                                        iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                        return PVMFSuccess;
                                    }
                                }
                            }
                        }

                    }

                }
            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_ARTIST_KEY) == 0)
            {
                // Artist

                uint32 countArtist = 0;
                countArtist = iMP4FileHandle->getNumArtist();

                if (countArtist > 0)
                {
                    for (idx = 0; idx < (int32)countArtist ; idx++)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;

                        // Create a value entry if past the starting index
                        if (numvalentries > starting_index)
                        {

                            if (iMP4FileHandle->getArtist(idx, valuestring, iLangCode, charType) != PVMFErrArgument)
                            {

                                char lang_param[43];
                                if (iLangCode != 0)
                                {
                                    int8 LangCode[4];
                                    getLanguageCode(iLangCode, LangCode);
                                    LangCode[3] = '\0';
                                    oscl_snprintf(lang_param, 20, ";%s%s", PVMP4METADATA_LANG_CODE, LangCode);
                                    lang_param[20] = '\0';
                                }
                                else
                                {
                                    lang_param[0] = '\0';
                                }
                                KeyVal.key = NULL;
                                KeyVal.value.pWChar_value = NULL;
                                KeyVal.value.pChar_value = NULL;
                                if (charType != ORIGINAL_CHAR_TYPE_UNKNOWN)
                                {
                                    char char_enc_param[22];
                                    oscl_snprintf(char_enc_param, 22, ";%s%s", PVMP4METADATA_ORIG_CHAR_ENC, orig_char_enc[charType-1]);
                                    char_enc_param[21] = '\0';
                                    oscl_strncat(lang_param, char_enc_param, oscl_strlen(char_enc_param));
                                }
                                PVMFStatus retval =
                                    PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                            PVMP4METADATA_ARTIST_KEY,
                                            valuestring,
                                            lang_param);
                                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                                {
                                    break;
                                }
                                // Add the KVP to the list if the key string was created
                                if (KeyVal.key != NULL)
                                {
                                    leavecode = AddToValueList(*valuelistptr, KeyVal);
                                    if (leavecode != 0)
                                    {
                                        if (KeyVal.value.pWChar_value != NULL)
                                        {
                                            OSCL_ARRAY_DELETE(KeyVal.value.pWChar_value);
                                            KeyVal.value.pWChar_value = NULL;
                                        }

                                        OSCL_ARRAY_DELETE(KeyVal.key);
                                        KeyVal.key = NULL;
                                    }
                                    else
                                    {
                                        // Increment the value list entry counter
                                        ++numentriesadded;
                                        IsMetadataValAddedBefore = true;
                                    }

                                    // Check if the max number of value entries were added
                                    if (max_entries > 0 && numentriesadded >= max_entries)
                                    {
                                        iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                        return PVMFSuccess;
                                    }
                                }
                            }

                        }
                    }

                }
            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_KEYWORD_KEY) == 0)
            {
                int32 numAssetInfoKeyword = iMP4FileHandle->getNumAssetInfoKeyWordAtoms();
                for (idx = 0; idx < numAssetInfoKeyword; idx++)
                {
                    int32 AssetInfoKeywordCount = iMP4FileHandle->getAssetInfoNumKeyWords(idx);
                    for (int32 idy = 0; idy < AssetInfoKeywordCount; idy++)
                    {

                        // Increment the counter for the number of values found so far
                        ++numvalentries;

                        // Create a value entry if past the starting index
                        if (numvalentries > starting_index)
                        {
                            int8 LangCode[4];
                            getLanguageCode(iMP4FileHandle->getAssetInfoKeyWordLangCode(idx), LangCode);
                            LangCode[3] = '\0';

                            char lang_param[21];
                            oscl_snprintf(lang_param, 20, ";%s%s", PVMP4METADATA_LANG_CODE, LangCode);
                            lang_param[20] = '\0';

                            OSCL_wHeapString<OsclMemAllocator> valuestring = iMP4FileHandle->getAssetInfoKeyWord(idx, idy);
                            PVMFStatus retval =
                                PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                        PVMP4METADATA_KEYWORD_KEY,
                                        valuestring,
                                        lang_param);
                            if (retval != PVMFSuccess && retval != PVMFErrArgument)
                            {
                                break;
                            }
                            // Add the KVP to the list if the key string was created
                            if (KeyVal.key != NULL)
                            {
                                leavecode = AddToValueList(*valuelistptr, KeyVal);
                                if (leavecode != 0)
                                {
                                    if (KeyVal.value.pWChar_value != NULL)
                                    {
                                        OSCL_ARRAY_DELETE(KeyVal.value.pWChar_value);
                                        KeyVal.value.pWChar_value = NULL;
                                    }

                                    OSCL_ARRAY_DELETE(KeyVal.key);
                                    KeyVal.key = NULL;
                                }
                                else
                                {
                                    // Increment the value list entry counter
                                    ++numentriesadded;
                                    IsMetadataValAddedBefore = true;
                                }

                                // Check if the max number of value entries were added
                                if (max_entries > 0 && numentriesadded >= max_entries)
                                {
                                    iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                    return PVMFSuccess;
                                }
                            }

                        }
                        KeyVal.key = NULL;
                        KeyVal.value.pWChar_value = NULL;
                        KeyVal.value.pChar_value = NULL;
                    }
                }
            }

            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_GENRE_KEY) == 0)
            {
                // Genre
                PVMFStatus retval = PVMFFailure;
                uint32 countGenre = 0;
                countGenre = iMP4FileHandle->getNumGenre();

                if (countGenre > 0)
                {
                    for (idx = 0; idx < (int32)countGenre ; idx++)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;

                        // Create a value entry if past the starting index
                        if (numvalentries > starting_index)
                        {

                            if (iMP4FileHandle->getGenre(idx, valuestring, iLangCode, charType) != PVMFErrArgument)
                            {
                                char lang_param[43];
                                if (iLangCode != 0)
                                {
                                    int8 LangCode[4];
                                    getLanguageCode(iLangCode, LangCode);
                                    LangCode[3] = '\0';
                                    oscl_snprintf(lang_param, 20, ";%s%s", PVMP4METADATA_LANG_CODE, LangCode);
                                    lang_param[20] = '\0';
                                }
                                else
                                {
                                    lang_param[0] = '\0';
                                }
                                KeyVal.key = NULL;
                                KeyVal.value.pWChar_value = NULL;
                                KeyVal.value.pChar_value = NULL;
                                if (charType != ORIGINAL_CHAR_TYPE_UNKNOWN)
                                {
                                    char char_enc_param[22];
                                    oscl_snprintf(char_enc_param, 22, ";%s%s", PVMP4METADATA_ORIG_CHAR_ENC, orig_char_enc[charType-1]);
                                    char_enc_param[21] = '\0';
                                    oscl_strncat(lang_param, char_enc_param, oscl_strlen(char_enc_param));
                                }
                                retval =
                                    PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                            PVMP4METADATA_GENRE_KEY,
                                            valuestring,
                                            lang_param);
                                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                                {
                                    break;
                                }
                                // Add the KVP to the list if the key string was created
                                if (KeyVal.key != NULL)
                                {
                                    leavecode = AddToValueList(*valuelistptr, KeyVal);
                                    if (leavecode != 0)
                                    {
                                        if (KeyVal.value.pWChar_value != NULL)
                                        {
                                            OSCL_ARRAY_DELETE(KeyVal.value.pWChar_value);
                                            KeyVal.value.pWChar_value = NULL;
                                        }

                                        OSCL_ARRAY_DELETE(KeyVal.key);
                                        KeyVal.key = NULL;
                                    }
                                    else
                                    {
                                        // Increment the value list entry counter
                                        ++numentriesadded;
                                        IsMetadataValAddedBefore = true;
                                    }

                                    // Check if the max number of value entries were added
                                    if (max_entries > 0 && numentriesadded >= max_entries)
                                    {
                                        iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                        return PVMFSuccess;
                                    }
                                }
                            }
                            uint32 value = iMP4FileHandle->getITunesGnreID();
                            if ((idx == 0) && (value != 0))
                            {
                                KeyVal.key = NULL;
                                KeyVal.value.pWChar_value = NULL;
                                KeyVal.value.pChar_value = NULL;
                                retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal, PVMP4METADATA_GENRE_KEY, value);

                                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                                {
                                    break;
                                }
                                // Add the KVP to the list if the key string was created
                                if (KeyVal.key != NULL)
                                {
                                    leavecode = AddToValueList(*valuelistptr, KeyVal);
                                    if (leavecode != 0)
                                    {
                                        KeyVal.key = NULL;
                                    }
                                    else
                                    {
                                        // Increment the value list entry counter
                                        ++numentriesadded;
                                        IsMetadataValAddedBefore = true;
                                    }

                                    // Check if the max number of value entries were added
                                    if (max_entries > 0 && numentriesadded >= max_entries)
                                    {
                                        iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                        return PVMFSuccess;
                                    }

                                }

                            }

                        }
                    }

                }
            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_CLASSIFICATION_KEY) == 0)
            {
                int32 numAssetInfoClassification = iMP4FileHandle->getNumAssetInfoClassificationAtoms();
                for (idx = 0; idx < numAssetInfoClassification; idx++)
                {
                    // Increment the counter for the number of values found so far
                    ++numvalentries;

                    // Create a value entry if past the starting index
                    if (numvalentries > starting_index)
                    {
                        int8 LangCode[4];
                        getLanguageCode(iMP4FileHandle->getAssetInfoClassificationLangCode(idx), LangCode);
                        LangCode[3] = '\0';

                        char lang_param[43];
                        oscl_snprintf(lang_param, 20, ";%s%s", PVMP4METADATA_LANG_CODE, LangCode);
                        lang_param[20] = '\0';

                        OSCL_wHeapString<OsclMemAllocator> valuestring = iMP4FileHandle->getAssetInfoClassificationNotice(charType, idx);
                        if (charType != ORIGINAL_CHAR_TYPE_UNKNOWN)
                        {
                            char char_enc_param[22];
                            oscl_snprintf(char_enc_param, 22, ";%s%s", PVMP4METADATA_ORIG_CHAR_ENC, orig_char_enc[charType-1]);
                            char_enc_param[21] = '\0';
                            oscl_strncat(lang_param, char_enc_param, oscl_strlen(char_enc_param));
                        }

                        PVMFStatus retval =
                            PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                    PVMP4METADATA_CLASSIFICATION_KEY,
                                    valuestring,
                                    lang_param);
                        if (retval != PVMFSuccess && retval != PVMFErrArgument)
                        {
                            break;
                        }
                        // Add the KVP to the list if the key string was created
                        if (KeyVal.key != NULL)
                        {
                            leavecode = AddToValueList(*valuelistptr, KeyVal);
                            if (leavecode != 0)
                            {
                                if (KeyVal.value.pWChar_value != NULL)
                                {
                                    OSCL_ARRAY_DELETE(KeyVal.value.pWChar_value);
                                    KeyVal.value.pWChar_value = NULL;
                                }

                                OSCL_ARRAY_DELETE(KeyVal.key);
                                KeyVal.key = NULL;
                            }
                            else
                            {
                                // Increment the value list entry counter
                                ++numentriesadded;
                                IsMetadataValAddedBefore = true;
                            }

                            // Check if the max number of value entries were added
                            if (max_entries > 0 && numentriesadded >= max_entries)
                            {
                                iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                return PVMFSuccess;
                            }
                        }
                    }
                }
            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_MAJORBRAND_KEY) == 0)
            {
                // MAJOR BRAND
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {
                    char BrandCode[5];
                    uint32 Mbrand = iMP4FileHandle->getCompatibiltyMajorBrand();
                    getBrand(Mbrand, BrandCode);
                    BrandCode[4] = '\0';

                    PVMFStatus retval =
                        PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                                PVMP4METADATA_MAJORBRAND_KEY,
                                BrandCode);
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                }
            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_COMPATIBLEBRAND_KEY) == 0)
            {
                // Compatible Brand
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {
                    Oscl_Vector<uint32, OsclMemAllocator> *Compatiblebrand_Vec = iMP4FileHandle->getCompatibiltyList();
                    uint32 idy = 0;
                    for (idy = 0; idy < Compatiblebrand_Vec->size() ; idy++)
                    {
                        char BrandCode[5];
                        uint32 CbrandNum = (*Compatiblebrand_Vec)[idy];
                        getBrand(CbrandNum, BrandCode);
                        BrandCode[4] = '\0';
                        KeyVal.key = NULL;
                        KeyVal.value.pWChar_value = NULL;
                        KeyVal.value.pChar_value = NULL;

                        PVMFStatus retval =
                            PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                                    PVMP4METADATA_COMPATIBLEBRAND_KEY,
                                    BrandCode);
                        if (retval != PVMFSuccess && retval != PVMFErrArgument)
                        {
                            break;
                        }
                        // Add the KVP to the list if the key string was created
                        if (KeyVal.key != NULL)
                        {
                            leavecode = AddToValueList(*valuelistptr, KeyVal);
                            if (leavecode != 0)
                            {
                                if (KeyVal.value.pChar_value != NULL)
                                {
                                    OSCL_ARRAY_DELETE(KeyVal.value.pChar_value);
                                    KeyVal.value.pChar_value = NULL;
                                }

                                OSCL_ARRAY_DELETE(KeyVal.key);
                                KeyVal.key = NULL;
                            }
                            else
                            {
                                // Increment the value list entry counter
                                ++numentriesadded;
                                IsMetadataValAddedBefore = true;
                            }

                            // Check if the max number of value entries were added
                            if (max_entries > 0 && numentriesadded >= max_entries)
                            {
                                iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                return PVMFSuccess;
                            }
                        }
                    }
                }
            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_VERSION_KEY) == 0)
            {
                // Version
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {
                    OSCL_wHeapString<OsclMemAllocator> valuestring = iMP4FileHandle->getPVVersion(charType);
                    PVMFStatus retval =
                        PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                PVMP4METADATA_VERSION_KEY,
                                valuestring);
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                }
            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_DATE_KEY) == 0)
            {
                // Date
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {
                    OSCL_wHeapString<OsclMemAllocator> valuestring = iMP4FileHandle->getCreationDate(charType);
                    PVMFStatus retval =
                        PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                                PVMP4METADATA_DATE_KEY,
                                valuestring);
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_VIDEO_PROFILE_KEY) != NULL)
            {
                // profile
                // Determine the index requested.
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = 0;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices - there should only be one index
                if (startindex != endindex || startindex > (uint32)(numtracks) || endindex > (uint32)(numtracks))
                {
                    break;
                }
                //get track id from index
                uint32 trackID = startindex + 1;
                uint32 iProfile = 0;

                OSCL_HeapString<OsclMemAllocator> trackMIMEType;

                iMP4FileHandle->getTrackMIMEType(trackID, trackMIMEType);

                if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000)) == 0)
                {
                    H263DecoderSpecificInfo *ptr = (H263DecoderSpecificInfo *)iMP4FileHandle->getTrackDecoderSpecificInfoAtSDI(trackID, 0);
                    iProfile = ptr->getCodecProfile();
                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Create a value entry if past the starting index
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, startindex);
                        indexparam[15] = '\0';

                        PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal,
                                            PVMP4METADATA_TRACKINFO_VIDEO_PROFILE_KEY,
                                            iProfile,
                                            indexparam);
                        if (retval != PVMFSuccess && retval != PVMFErrArgument)
                        {
                            break;
                        }
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_VIDEO_LEVEL_KEY) != NULL)
            {
                // level
                // Determine the index requested.
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = 0;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices - there should only be one index
                if (startindex != endindex || startindex > (uint32)(numtracks) || endindex > (uint32)(numtracks))
                {
                    break;
                }
                //get track id from index
                uint32 trackID = startindex + 1;
                uint32 iLevel = 0;

                OSCL_HeapString<OsclMemAllocator> trackMIMEType;

                iMP4FileHandle->getTrackMIMEType(trackID, trackMIMEType);

                if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000)) == 0)
                {
                    H263DecoderSpecificInfo *ptr = (H263DecoderSpecificInfo *)iMP4FileHandle->getTrackDecoderSpecificInfoAtSDI(trackID, 0);
                    iLevel = ptr->getCodecLevel();
                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Create a value entry if past the starting index
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, startindex);
                        indexparam[15] = '\0';
                        PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal,
                                            PVMP4METADATA_TRACKINFO_VIDEO_LEVEL_KEY,
                                            iLevel,
                                            indexparam);
                        if (retval != PVMFSuccess && retval != PVMFErrArgument)
                        {
                            break;
                        }
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_FRAME_RATE_KEY) != NULL)
            {
                // level
                // Determine the index requested.
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = 0;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices - there should only be one index
                if (startindex != endindex || startindex > (uint32)(numtracks) || endindex > (uint32)(numtracks))
                {
                    break;
                }

                //get track id from index
                //uint32 trackID = startindex+1;

                uint32 iIdList[16];
                iMP4FileHandle->getTrackIDList(iIdList, numtracks);
                uint32 trackID = iIdList[startindex];

                OSCL_HeapString<OsclMemAllocator> trackMIMEType;

                iMP4FileHandle->getTrackMIMEType(trackID, trackMIMEType);

                if ((oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000)) == 0) ||
                        (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H264_VIDEO_MP4, oscl_strlen(PVMF_MIME_H264_VIDEO_MP4)) == 0) ||
                        (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_M4V, oscl_strlen(PVMF_MIME_M4V)) == 0))
                {
                    uint64 trackduration  = iMP4FileHandle->getTrackMediaDuration(trackID);
                    uint32 samplecount = iMP4FileHandle->getSampleCountInTrack(trackID);

                    MediaClockConverter mcc(iMP4FileHandle->getTrackMediaTimescale(trackID));
                    mcc.update_clock(trackduration);
                    uint32 TrackDurationInSec = mcc.get_converted_ts(1);
                    uint32 frame_rate = 0;

                    uint32 OverflowThreshold = PVMF_MP4_MAX_UINT32 / MILLISECOND_TIMESCALE;
                    // If overflow could not happen, we calculate it in millisecond
                    if (TrackDurationInSec < OverflowThreshold && samplecount < OverflowThreshold)
                    {
                        uint32 TrackDurationInMilliSec = mcc.get_converted_ts(MILLISECOND_TIMESCALE);
                        if (TrackDurationInMilliSec > 0)
                        {
                            frame_rate = samplecount * MILLISECOND_TIMESCALE / TrackDurationInMilliSec;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else // if overflow could happen when calculate in millisecond, we calculate it in second
                    {
                        if (TrackDurationInSec > 0)
                        {
                            frame_rate = samplecount / TrackDurationInSec;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Create a value entry if past the starting index
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, startindex);
                        indexparam[15] = '\0';
                        PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal,
                                            PVMP4METADATA_TRACKINFO_FRAME_RATE_KEY,
                                            frame_rate,
                                            indexparam);
                        if (retval != PVMFSuccess && retval != PVMFErrArgument)
                        {
                            break;
                        }
                    }
                }
            }

            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_DURATION_KEY) == 0 &&
                     iMP4FileHandle->getMovieDuration() > (uint64)0 && iMP4FileHandle->getMovieTimescale() > 0)
            {
                // Movie Duration
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {
                    uint64 duration64 = iMP4FileHandle->getMovieDuration();
                    uint32 duration = Oscl_Int64_Utils::get_uint64_lower32(duration64);
                    char timescalestr[20];
                    oscl_snprintf(timescalestr, 20, ";%s%d", PVMP4METADATA_TIMESCALE, iMP4FileHandle->getMovieTimescale());
                    timescalestr[19] = '\0';
                    PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal, PVMP4METADATA_DURATION_KEY, duration, timescalestr);
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                }
            }
            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_YEAR_KEY) == 0)
            {
                // Year
                uint32 value = 0;
                uint32 countYear = 0;
                countYear = iMP4FileHandle->getNumYear();

                if (countYear > 0)
                {
                    for (idx = 0; idx < (int32)countYear ; idx++)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;

                        // Create a value entry if past the starting index
                        if (numvalentries > starting_index)
                        {

                            if (!iMP4FileHandle->getYear(idx, value))
                            {
                                PVMF_MP4FFPARSERNODE_LOGERROR((0, "PVMFMP4FFParserNode::DoGetMetadataValues - getYear Failed"));
                                return PVMFFailure;
                            }

                            PVMFStatus retval =
                                PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal,
                                        PVMP4METADATA_YEAR_KEY, value);
                            if (retval != PVMFSuccess && retval != PVMFErrArgument)
                            {
                                break;
                            }
                            // Add the KVP to the list if the key string was created
                            if (KeyVal.key != NULL)
                            {
                                leavecode = AddToValueList(*valuelistptr, KeyVal);
                                if (leavecode != 0)
                                {
                                    KeyVal.key = NULL;
                                }
                                else
                                {
                                    // Increment the value list entry counter
                                    ++numentriesadded;
                                    IsMetadataValAddedBefore = true;
                                }

                                // Check if the max number of value entries were added
                                if (max_entries > 0 && numentriesadded >= max_entries)
                                {
                                    iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                    return PVMFSuccess;
                                }
                            }

                        }
                    }

                }
            }

            else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_NUMTRACKS_KEY) == 0 &&
                     iMP4FileHandle->getNumTracks() > 0)
            {
                // Number of tracks
                // Increment the counter for the number of values found so far
                ++numvalentries;

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {
                    uint32 numtracks = iMP4FileHandle->getNumTracks();
                    PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal, PVMP4METADATA_NUMTRACKS_KEY, numtracks);
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                }
            }

            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_TYPE_KEY) != NULL)
            {
                // Track type

                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;
                    trackkvp.value.pChar_value = NULL;

                    char indexparam[16];
                    oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, i);
                    indexparam[15] = '\0';

                    PVMFStatus retval = PVMFErrArgument;

                    OSCL_HeapString<OsclMemAllocator> trackMIMEType;

                    iMP4FileHandle->getTrackMIMEType(trackidlist[i], (OSCL_String&)trackMIMEType);

                    if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_M4V, oscl_strlen(PVMF_MIME_M4V)) == 0)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;
                        // Add the value entry if past the starting index
                        if (numvalentries > starting_index)
                        {
                            retval =
                                PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp,
                                        PVMP4METADATA_TRACKINFO_TYPE_KEY,
                                        _STRLIT_CHAR(PVMF_MIME_M4V),
                                        indexparam);
                        }
                    }
                    else if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000)) == 0)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;
                        // Add the value entry if past the starting index
                        if (numvalentries > starting_index)
                        {
                            retval =
                                PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp,
                                        PVMP4METADATA_TRACKINFO_TYPE_KEY,
                                        _STRLIT_CHAR(PVMF_MIME_H2631998),
                                        indexparam);
                        }
                    }
                    else if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H264_VIDEO_MP4, oscl_strlen(PVMF_MIME_H264_VIDEO_MP4)) == 0)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;
                        // Add the value entry if past the starting index
                        if (numvalentries > starting_index)
                        {
                            retval =
                                PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp, PVMP4METADATA_TRACKINFO_TYPE_KEY, _STRLIT_CHAR(PVMF_MIME_H264_VIDEO_MP4), indexparam);
                        }
                    }
                    else if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_MPEG4_AUDIO, oscl_strlen(PVMF_MIME_MPEG4_AUDIO)) == 0)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;
                        // Add the value entry if past the starting index
                        if (numvalentries > starting_index)
                        {
                            retval =
                                PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp, PVMP4METADATA_TRACKINFO_TYPE_KEY, _STRLIT_CHAR(PVMF_MIME_MPEG4_AUDIO), indexparam);
                        }
                    }
                    else if ((oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_AMR_IETF, oscl_strlen(PVMF_MIME_AMR_IETF)) == 0) ||
                             (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_AMRWB_IETF, oscl_strlen(PVMF_MIME_AMRWB_IETF)) == 0))
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;
                        // Add the value entry if past the starting index
                        if (numvalentries > starting_index)
                        {
                            retval =
                                PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp, PVMP4METADATA_TRACKINFO_TYPE_KEY, _STRLIT_CHAR(PVMF_MIME_AMR_IETF), indexparam);
                        }
                    }
                    else if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_3GPP_TIMEDTEXT, oscl_strlen(PVMF_MIME_3GPP_TIMEDTEXT)) == 0)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;
                        // Add the value entry if past the starting index
                        if (numvalentries > starting_index)
                        {
                            retval =
                                PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp, PVMP4METADATA_TRACKINFO_TYPE_KEY, _STRLIT_CHAR(PVMF_MIME_3GPP_TIMEDTEXT), indexparam);
                        }
                    }
                    else
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;
                        // Add the value entry if past the starting index
                        if (numvalentries > starting_index)
                        {
                            if (iMP4FileHandle->getTrackMediaType(trackidlist[i]) == MEDIA_TYPE_VISUAL)
                            {
                                retval =
                                    PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp,
                                            PVMP4METADATA_TRACKINFO_TYPE_KEY,
                                            _STRLIT_CHAR(PVMF_MP4_MIME_FORMAT_VIDEO_UNKNOWN),
                                            indexparam);
                            }
                            else if (iMP4FileHandle->getTrackMediaType(trackidlist[i]) == MEDIA_TYPE_AUDIO)
                            {
                                retval =
                                    PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp,
                                            PVMP4METADATA_TRACKINFO_TYPE_KEY,
                                            _STRLIT_CHAR(PVMF_MP4_MIME_FORMAT_AUDIO_UNKNOWN),
                                            indexparam);
                            }
                            else
                            {
                                retval =
                                    PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp,
                                            PVMP4METADATA_TRACKINFO_TYPE_KEY,
                                            _STRLIT_CHAR(PVMF_MP4_MIME_FORMAT_UNKNOWN),
                                            indexparam);

                            }

                        }

                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        leavecode = AddToValueList(*valuelistptr, trackkvp);
                        if (leavecode != 0)
                        {
                            if (trackkvp.value.pChar_value != NULL)
                            {
                                OSCL_ARRAY_DELETE(trackkvp.value.pChar_value);
                                trackkvp.value.pChar_value = NULL;
                            }

                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                            IsMetadataValAddedBefore = true;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_TRACKID_KEY) != NULL)
            {
                // Track ID

                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    PVMFStatus retval = PVMFErrArgument;
                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, i);
                        indexparam[15] = '\0';

                        retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp, PVMP4METADATA_TRACKINFO_TRACKID_KEY, trackidlist[i], indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        leavecode = AddToValueList(*valuelistptr, trackkvp);
                        if (leavecode != 0)
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                            IsMetadataValAddedBefore = true;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_DURATION_KEY) != NULL)
            {
                // Track duration

                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indextimescaleparam[36];
                        uint32 timeScale = 0;

                        if (iParsingMode && iMP4FileHandle->IsMovieFragmentsPresent())
                            timeScale = iMP4FileHandle->getMovieTimescale();
                        else
                            timeScale = iMP4FileHandle->getTrackMediaTimescale(trackidlist[i]);

                        oscl_snprintf(indextimescaleparam, 36, ";%s%d;%s%d", PVMP4METADATA_INDEX, i, PVMP4METADATA_TIMESCALE, timeScale);

                        indextimescaleparam[35] = '\0';

                        uint64 trackduration64 = iMP4FileHandle->getTrackMediaDuration(trackidlist[i]);
                        uint32 trackduration = Oscl_Int64_Utils::get_uint64_lower32(trackduration64);;

                        retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp, PVMP4METADATA_TRACKINFO_DURATION_KEY, trackduration, indextimescaleparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        leavecode = AddToValueList(*valuelistptr, trackkvp);
                        if (leavecode != 0)
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                            IsMetadataValAddedBefore = true;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_BITRATE_KEY) != NULL)
            {
                // Track bitrate

                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, i);
                        indexparam[15] = '\0';

                        uint32 trackbitrate = (uint32)(iMP4FileHandle->getTrackAverageBitrate(trackidlist[i])); // Always returns unsigned value

                        retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp, PVMP4METADATA_TRACKINFO_BITRATE_KEY, trackbitrate, indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        leavecode = AddToValueList(*valuelistptr, trackkvp);
                        if (leavecode != 0)
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                            IsMetadataValAddedBefore = true;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
            else if ((oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_TRACK_NUMBER_KEY) != NULL) &&
                     iMP4FileHandle->getITunesThisTrackNo() > 0)
            {
                // iTunes Current Track Number
                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;
                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, i);
                        indexparam[15] = '\0';

                        uint32 track_number = iMP4FileHandle->getITunesThisTrackNo(); // Always returns unsigned value

                        char cdTrackNumber[6];
                        uint16 totalTrackNumber = iMP4FileHandle->getITunesTotalTracks();
                        oscl_snprintf(cdTrackNumber, 6, "%d/%d", track_number, totalTrackNumber);
                        cdTrackNumber[5] = '\0';

                        retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp, PVMP4METADATA_TRACKINFO_TRACK_NUMBER_KEY, cdTrackNumber, indexparam);
                        if ((retval != PVMFSuccess) && (retval != PVMFErrArgument))
                        {
                            break;
                        }

                        if (trackkvp.key != NULL)
                        {
                            leavecode = AddToValueList(*valuelistptr, trackkvp);
                            if (leavecode != 0)
                            {
                                OSCL_ARRAY_DELETE(trackkvp.key);
                                trackkvp.key = NULL;
                            }
                            else
                            {
                                // Increment the value list entry counter
                                ++numentriesadded;
                                IsMetadataValAddedBefore = true;
                            }

                            // Check if the max number of value entries were added
                            if (max_entries > 0 && numentriesadded >= max_entries)
                            {

                                iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                return PVMFSuccess;
                            }
                        }
                    }
                }
            }
            else if ((oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_AUDIO_FORMAT_KEY) != NULL) ||
                     (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_VIDEO_FORMAT_KEY) != NULL))
            {
                // Audio or video track format
                // Set index for track type
                uint32 tracktype = 0; // 0 unknown, 1 video, 2 audio
                if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_VIDEO_FORMAT_KEY) != NULL)
                {
                    tracktype = 1;
                }
                else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_AUDIO_FORMAT_KEY) != NULL)
                {
                    tracktype = 2;
                }

                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;
                    trackkvp.value.pChar_value = NULL;

                    char indexparam[16];
                    oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, i);
                    indexparam[15] = '\0';

                    PVMFStatus retval = PVMFErrArgument;
                    OSCL_HeapString<OsclMemAllocator> trackMIMEType;

                    iMP4FileHandle->getTrackMIMEType(trackidlist[i], trackMIMEType);

                    if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_M4V, oscl_strlen(PVMF_MIME_M4V)) == 0)
                    {
                        if (tracktype == 1)
                        {
                            ++numvalentries;
                            if (numvalentries > starting_index)
                            {
                                retval =
                                    PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp, PVMP4METADATA_TRACKINFO_VIDEO_FORMAT_KEY, _STRLIT_CHAR(PVMF_MIME_M4V), indexparam);
                            }
                        }
                    }
                    else if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H2632000, oscl_strlen(PVMF_MIME_H2632000)) == 0)
                    {
                        if (tracktype == 1)
                        {
                            ++numvalentries;
                            if (numvalentries > starting_index)
                            {
                                retval =
                                    PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp, PVMP4METADATA_TRACKINFO_VIDEO_FORMAT_KEY, _STRLIT_CHAR(PVMF_MIME_H2631998), indexparam);
                            }
                        }
                    }
                    else if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_H264_VIDEO_MP4, oscl_strlen(PVMF_MIME_H264_VIDEO_MP4)) == 0)
                    {
                        if (tracktype == 1)
                        {
                            ++numvalentries;
                            if (numvalentries > starting_index)
                            {
                                retval =
                                    PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp, PVMP4METADATA_TRACKINFO_VIDEO_FORMAT_KEY, _STRLIT_CHAR(PVMF_MIME_H264_VIDEO_MP4), indexparam);
                            }
                        }
                    }
                    else if (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_MPEG4_AUDIO, oscl_strlen(PVMF_MIME_MPEG4_AUDIO)) == 0)
                    {
                        if (tracktype == 2)
                        {
                            ++numvalentries;
                            if (numvalentries > starting_index)
                            {
                                retval =
                                    PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp, PVMP4METADATA_TRACKINFO_AUDIO_FORMAT_KEY, _STRLIT_CHAR(PVMF_MIME_MPEG4_AUDIO), indexparam);
                            }
                        }
                    }
                    else if ((oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_AMR_IETF, oscl_strlen(PVMF_MIME_AMR_IETF)) == 0) ||
                             (oscl_strncmp(trackMIMEType.get_str(), PVMF_MIME_AMRWB_IETF, oscl_strlen(PVMF_MIME_AMRWB_IETF)) == 0))
                    {
                        if (tracktype == 2)
                        {
                            ++numvalentries;
                            if (numvalentries > starting_index)
                            {
                                retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp, PVMP4METADATA_TRACKINFO_AUDIO_FORMAT_KEY, _STRLIT_CHAR(PVMF_MIME_AMR_IETF), indexparam);
                            }
                        }
                    }
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        leavecode = AddToValueList(*valuelistptr, trackkvp);
                        if (leavecode != 0)
                        {
                            if (trackkvp.value.pChar_value != NULL)
                            {
                                OSCL_ARRAY_DELETE(trackkvp.value.pChar_value);
                                trackkvp.value.pChar_value = NULL;
                            }

                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                            IsMetadataValAddedBefore = true;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_VIDEO_WIDTH_KEY) != NULL)
            {
                // Video track width

                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    if (iMP4FileHandle->getTrackMediaType(trackidlist[i]) == MEDIA_TYPE_VISUAL)
                    {
                        // Increment the counter for the number of values found so far
                        numvalentries++;

                        // Add the value entry if past the starting index
                        if (numvalentries > starting_index)
                        {
                            char indexparam[16];
                            oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, i);
                            indexparam[15] = '\0';

                            uint32 trackwidth = (uint32)(FindVideoDisplayWidth(trackidlist[i]));
                            PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp, PVMP4METADATA_TRACKINFO_VIDEO_WIDTH_KEY, trackwidth, indexparam);
                            if (retval != PVMFSuccess && retval != PVMFErrArgument)
                            {
                                break;
                            }

                            if (trackkvp.key != NULL)
                            {
                                leavecode = AddToValueList(*valuelistptr, trackkvp);
                                if (leavecode != 0)
                                {
                                    OSCL_ARRAY_DELETE(trackkvp.key);
                                    trackkvp.key = NULL;
                                }
                                else
                                {
                                    // Increment the value list entry counter
                                    ++numentriesadded;
                                    IsMetadataValAddedBefore = true;
                                }

                                // Check if the max number of value entries were added
                                if (max_entries > 0 && numentriesadded >= max_entries)
                                {
                                    iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                    return PVMFSuccess;
                                }
                            }
                        }
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_VIDEO_HEIGHT_KEY) != NULL)
            {
                // Video track height

                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    if (iMP4FileHandle->getTrackMediaType(trackidlist[i]) == MEDIA_TYPE_VISUAL)
                    {
                        // Increment the counter for the number of values found so far
                        numvalentries++;

                        // Add the value entry if past the starting index
                        if (numvalentries > starting_index)
                        {
                            char indexparam[16];
                            oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, i);
                            indexparam[15] = '\0';

                            uint32 trackheight = (uint32)(FindVideoDisplayHeight(trackidlist[i]));
                            PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp, PVMP4METADATA_TRACKINFO_VIDEO_HEIGHT_KEY, trackheight, indexparam);
                            if (retval != PVMFSuccess && retval != PVMFErrArgument)
                            {
                                break;
                            }

                            if (trackkvp.key != NULL)
                            {
                                leavecode = AddToValueList(*valuelistptr, trackkvp);
                                if (leavecode != 0)
                                {
                                    OSCL_ARRAY_DELETE(trackkvp.key);
                                    trackkvp.key = NULL;
                                }
                                else
                                {
                                    // Increment the value list entry counter
                                    ++numentriesadded;
                                    IsMetadataValAddedBefore = true;
                                }

                                // Check if the max number of value entries were added
                                if (max_entries > 0 && numentriesadded >= max_entries)
                                {
                                    iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                    return PVMFSuccess;
                                }
                            }
                        }
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_SAMPLERATE_KEY) != NULL)
            {
                // Sampling rate (only for video tracks)

                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    if (iMP4FileHandle->getTrackMediaType(trackidlist[i]) == MEDIA_TYPE_AUDIO)
                    {
                        // Increment the counter for the number of values found so far
                        numvalentries++;

                        // Add the value entry if past the starting index
                        if (numvalentries > starting_index)
                        {
                            char indexparam[16];
                            oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, i);
                            indexparam[15] = '\0';

                            uint32 samplerate = GetAudioSampleRate(trackidlist[i]);
                            PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp, PVMP4METADATA_TRACKINFO_SAMPLERATE_KEY, samplerate, indexparam);
                            if (retval != PVMFSuccess && retval != PVMFErrArgument)
                            {
                                break;
                            }

                            if (trackkvp.key != NULL)
                            {
                                leavecode = AddToValueList(*valuelistptr, trackkvp);
                                if (leavecode != 0)
                                {
                                    OSCL_ARRAY_DELETE(trackkvp.key);
                                    trackkvp.key = NULL;
                                }
                                else
                                {
                                    // Increment the value list entry counter
                                    ++numentriesadded;
                                    IsMetadataValAddedBefore = true;
                                }

                                // Check if the max number of value entries were added
                                if (max_entries > 0 && numentriesadded >= max_entries)
                                {
                                    iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                    return PVMFSuccess;
                                }
                            }
                        }
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_SAMPLECOUNT_KEY) != NULL)
            {
                // Sample count

                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, i);
                        indexparam[15] = '\0';

                        uint32 samplecount = (uint32)(iMP4FileHandle->getSampleCountInTrack(trackidlist[i])); // Always returns unsigned value

                        retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp, PVMP4METADATA_TRACKINFO_SAMPLECOUNT_KEY, samplecount, indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        leavecode = AddToValueList(*valuelistptr, trackkvp);
                        if (leavecode != 0)
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                            IsMetadataValAddedBefore = true;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_NUM_KEY_SAMPLES_KEY) != NULL)
            {
                // Num-Key-Samples

                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, i);
                        indexparam[15] = '\0';

                        uint32 keySampleCount = iMP4FileHandle->getNumKeyFrames(trackidlist[i]);

                        retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp, PVMP4METADATA_TRACKINFO_NUM_KEY_SAMPLES_KEY, keySampleCount, indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        leavecode = AddToValueList(*valuelistptr, trackkvp);
                        if (leavecode != 0)
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                            IsMetadataValAddedBefore = true;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_AUDIO_NUMCHANNELS_KEY) != NULL)
            {
                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    if (iMP4FileHandle->getTrackMediaType(trackidlist[i]) == MEDIA_TYPE_AUDIO)
                    {
                        // Increment the counter for the number of values found so far
                        numvalentries++;

                        // Add the value entry if past the starting index
                        if (numvalentries > starting_index)
                        {
                            char indexparam[16];
                            oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, i);
                            indexparam[15] = '\0';

                            uint32 numAudioChannels = (GetNumAudioChannels(trackidlist[i]));
                            PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp, PVMP4METADATA_TRACKINFO_AUDIO_NUMCHANNELS_KEY, numAudioChannels, indexparam);
                            if (retval != PVMFSuccess && retval != PVMFErrArgument)
                            {
                                break;
                            }

                            if (trackkvp.key != NULL)
                            {
                                leavecode = AddToValueList(*valuelistptr, trackkvp);
                                if (leavecode != 0)
                                {
                                    OSCL_ARRAY_DELETE(trackkvp.key);
                                    trackkvp.key = NULL;
                                }
                                else
                                {
                                    // Increment the value list entry counter
                                    ++numentriesadded;
                                    IsMetadataValAddedBefore = true;
                                }

                                // Check if the max number of value entries were added
                                if (max_entries > 0 && numentriesadded >= max_entries)
                                {
                                    iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                    return PVMFSuccess;
                                }
                            }
                        }
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_AUDIO_BITS_PER_SAMPLE_KEY) != NULL)
            {
                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    if (iMP4FileHandle->getTrackMediaType(trackidlist[i]) == MEDIA_TYPE_AUDIO)
                    {
                        // Increment the counter for the number of values found so far
                        numvalentries++;

                        // Add the value entry if past the starting index
                        if (numvalentries > starting_index)
                        {
                            char indexparam[16];
                            oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, i);
                            indexparam[15] = '\0';

                            uint32 numbitspersample = (GetAudioBitsPerSample(trackidlist[i]));
                            PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp, PVMP4METADATA_TRACKINFO_AUDIO_BITS_PER_SAMPLE_KEY, numbitspersample, indexparam);
                            if (retval != PVMFSuccess && retval != PVMFErrArgument)
                            {
                                break;
                            }

                            if (trackkvp.key != NULL)
                            {
                                leavecode = AddToValueList(*valuelistptr, trackkvp);
                                if (leavecode != 0)
                                {
                                    OSCL_ARRAY_DELETE(trackkvp.key);
                                    trackkvp.key = NULL;
                                }
                                else
                                {
                                    // Increment the value list entry counter
                                    ++numentriesadded;
                                    IsMetadataValAddedBefore = true;
                                }

                                // Check if the max number of value entries were added
                                if (max_entries > 0 && numentriesadded >= max_entries)
                                {
                                    iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                                    return PVMFSuccess;
                                }
                            }
                        }
                    }
                }
            }
            else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_TRACKINFO_SELECTED_KEY) != NULL)
            {
                // Track selected info

                // Determine the index requested. Default to all tracks
                // Check if the file has at least one track
                int32 numtracks = iMP4FileHandle->getNumTracks();
                if (numtracks <= 0)
                {
                    break;
                }
                uint32 startindex = 0;
                uint32 endindex = (uint32)numtracks - 1;
                // Check if the index parameter is present
                const char* indexstr = oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMP4METADATA_INDEX);
                if (indexstr != NULL)
                {
                    // Retrieve the index values
                    GetIndexParamValues(indexstr, startindex, endindex);
                }
                // Validate the indices
                if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
                {
                    break;
                }

                // Return a KVP for each index
                for (uint32 i = startindex; i <= endindex; ++i)
                {
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    PVMFStatus retval = PVMFErrArgument;
                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMP4METADATA_INDEX, i);
                        indexparam[15] = '\0';

                        // Check if the track has been selected by looking up
                        // the current index's track ID in the NodeTrackPort vector
                        bool trackselected = false;
                        for (uint32 j = 0; j < iNodeTrackPortList.size(); ++j)
                        {
                            if ((uint32)iNodeTrackPortList[j].iTrackId == trackidlist[i])
                            {
                                trackselected = true;
                                break;
                            }
                        }
                        retval = PVMFCreateKVPUtils::CreateKVPForBoolValue(trackkvp, PVMP4METADATA_TRACKINFO_SELECTED_KEY, trackselected, indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        leavecode = AddToValueList(*valuelistptr, trackkvp);
                        if (leavecode != 0)
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                            IsMetadataValAddedBefore = true;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }

            // Add the KVP to the list if the key string was created
            if ((KeyVal.key != NULL) && (!IsMetadataValAddedBefore))
            {
                leavecode = AddToValueList(*valuelistptr, KeyVal);
                if (leavecode != 0)
                {
                    switch (GetValTypeFromKeyString(KeyVal.key))
                    {
                        case PVMI_KVPVALTYPE_CHARPTR:
                            if (KeyVal.value.pChar_value != NULL)
                            {
                                OSCL_ARRAY_DELETE(KeyVal.value.pChar_value);
                                KeyVal.value.pChar_value = NULL;
                            }
                            break;

                        case PVMI_KVPVALTYPE_WCHARPTR:
                            if (KeyVal.value.pWChar_value != NULL)
                            {
                                OSCL_ARRAY_DELETE(KeyVal.value.pWChar_value);
                                KeyVal.value.pWChar_value = NULL;
                            }
                            break;

                        default:
                            // Add more case statements if other value types are returned
                            break;
                    }

                    OSCL_ARRAY_DELETE(KeyVal.key);
                    KeyVal.key = NULL;
                }
                else
                {
                    // Increment the counter for number of value entries added to the list
                    ++numentriesadded;
                }

                // Check if the max number of value entries were added
                if (max_entries > 0 && numentriesadded >= max_entries)
                {
                    // Maximum number of values added so break out of the loop
                    //return PVMFSuccess;
                    break;
                }
            }
        }
        iMP4ParserNodeMetadataValueCount = (*valuelistptr).size();
    }

    PVMF_MP4FFPARSERNODE_LOGERROR((0, "PVMFMP4FFParserNode::DoGetMetadataValues - NumParserNodeValues=%d", iMP4ParserNodeMetadataValueCount));

    if ((iCPMMetaDataExtensionInterface != NULL) &&
            (iProtectedFile == true))
    {
        iCPMGetMetaDataValuesCmdId =
            iCPMMetaDataExtensionInterface->GetNodeMetadataValues(iCPMSessionID,
                    (*keylistptr_in),
                    (*valuelistptr),
                    0);
        return PVMFPending;
    }
    return PVMFSuccess;
}

void PVMFMP4FFParserNode::CompleteGetMetaDataValues()
{
    PVMFMetadataList* keylistptr = NULL;
    Oscl_Vector<PvmiKvp, OsclMemAllocator>* valuelistptr = NULL;
    uint32 starting_index;
    int32 max_entries;

    iCurrentCommand.front().PVMFMP4FFParserNodeCommand::Parse(keylistptr, valuelistptr, starting_index, max_entries);

    for (uint32 i = 0; i < (*valuelistptr).size(); i++)
    {
        PVMF_MP4FFPARSERNODE_LOGINFO((0, "PVMFMP4FFParserNode::CompleteGetMetaDataValues - Index=%d, Key=%s", i, (*valuelistptr)[i].key));
    }

    CommandComplete(iCurrentCommand,
                    iCurrentCommand.front(),
                    PVMFSuccess);
}

int32 PVMFMP4FFParserNode::AddToValueList(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, PvmiKvp& aNewValue)
{
    int32 leavecode = 0;
    OSCL_TRY(leavecode, aValueList.push_back(aNewValue));
    return leavecode;
}

void PVMFMP4FFParserNode::DeleteAPICStruct(PvmfApicStruct*& aAPICStruct)
{
    OSCL_ARRAY_DELETE(aAPICStruct->iGraphicData);
    OSCL_DELETE(aAPICStruct);
    aAPICStruct = NULL;
}

PVMFStatus PVMFMP4FFParserNode::GetIndexParamValues(const char* aString, uint32& aStartIndex, uint32& aEndIndex)
{
    // This parses a string of the form "index=N1...N2" and extracts the integers N1 and N2.
    // If string is of the format "index=N1" then N2=N1

    if (aString == NULL)
    {
        return PVMFErrArgument;
    }

    // Go to end of "index="
    char* n1string = (char*)aString + 6;

    PV_atoi(n1string, 'd', oscl_strlen(n1string), aStartIndex);

    const char* n2string = oscl_strstr(aString, _STRLIT_CHAR("..."));

    if (n2string == NULL)
    {
        aEndIndex = aStartIndex;
    }
    else
    {
        // Go to end of "index=N1..."
        n2string += 3;

        PV_atoi(n2string, 'd', oscl_strlen(n2string), aEndIndex);
    }

    return PVMFSuccess;
}

void PVMFMP4FFParserNode::getLanguageCode(uint16 langcode, int8 *LangCode)
{
    //ISO-639-2/T 3-char Lang Code
    oscl_memset(LangCode, 0, 4);
    LangCode[0] = 0x60 + ((langcode >> 10) & 0x1F);
    LangCode[1] = 0x60 + ((langcode >> 5) & 0x1F);
    LangCode[2] = 0x60 + ((langcode) & 0x1F);
}

void PVMFMP4FFParserNode::CreateDurationInfoMsg(uint32 adurationms)
{
    int32 leavecode = 0;
    PVMFDurationInfoMessage* eventmsg = NULL;
    OSCL_TRY(leavecode, eventmsg = OSCL_NEW(PVMFDurationInfoMessage, (adurationms)));
    PVMFNodeInterface::ReportInfoEvent(PVMFInfoDurationAvailable, NULL, OSCL_STATIC_CAST(PVInterface*, eventmsg));
    if (eventmsg)
    {
        eventmsg->removeRef();
    }
}

PVMFStatus PVMFMP4FFParserNode::PushKVPToMetadataValueList(Oscl_Vector<PvmiKvp, OsclMemAllocator>* aVecPtr, PvmiKvp& aKvpVal)
{
    if (aVecPtr == NULL)
    {
        return PVMFErrArgument;
    }
    int32 leavecode = 0;
    OSCL_TRY(leavecode, aVecPtr->push_back(aKvpVal););
    if (leavecode != 0)
    {
        OSCL_ARRAY_DELETE(aKvpVal.key);
        aKvpVal.key = NULL;
        return PVMFErrNoMemory;
    }
    return PVMFSuccess;
}

PVMFStatus PVMFMP4FFParserNode::CreateNewArray(uint32** aTrackidList, uint32 aNumTracks)
{
    int32 leavecode = 0;
    OSCL_TRY(leavecode, *aTrackidList = OSCL_ARRAY_NEW(uint32, aNumTracks););
    OSCL_FIRST_CATCH_ANY(leavecode, return PVMFErrNoMemory;);
    return PVMFSuccess;
}

PVMFStatus PVMFMP4FFParserNode::PushValueToList(Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> &aRefMetaDataKeys, PVMFMetadataList *&aKeyListPtr, uint32 aLcv)
{
    int32 leavecode = 0;
    OSCL_TRY(leavecode, aKeyListPtr->push_back(aRefMetaDataKeys[aLcv]));
    OSCL_FIRST_CATCH_ANY(leavecode, PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFMP4FFParserNode::PushValueToList() Memory allocation failure when copying metadata key"));return PVMFErrNoMemory);
    return PVMFSuccess;
}
