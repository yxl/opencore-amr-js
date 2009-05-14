/*
 * Copyright (C) 2008, Google Inc.
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

#include <media/mediascanner.h>
#include <stdio.h>


#include "pvlogger.h"
#include "pv_id3_parcom.h"
#include "oscl_string_containers.h"
#include "oscl_file_io.h"
#include "oscl_assert.h"
#include "oscl_lock_base.h"
#include "oscl_snprintf.h"
#include "oscl_string_utf8.h"
#include "pvmf_return_codes.h"
#include "pv_mime_string_utils.h"
#include "pv_id3_parcom_constants.h"
#include "oscl_utf8conv.h"
#include "imp3ff.h"
#include "impeg4file.h"
#include "autodetect.h"

// Ogg Vorbis includes
#include "ivorbiscodec.h"
#include "ivorbisfile.h"

// Sonivox includes
#include <libsonivox/eas.h>

// used for WMA support
#include "media/mediametadataretriever.h"

#include <media/thread_init.h>
#include <utils/string_array.h>

#define MAX_BUFF_SIZE   1024

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include "unicode/ucnv.h"
#include "unicode/ustring.h"

#undef LOG_TAG
#define LOG_TAG "MediaScanner"
#include "utils/Log.h"

#define MAX_STR_LEN    1000


namespace android {


MediaScanner::MediaScanner()
    :   mLocale(NULL)
{
}

MediaScanner::~MediaScanner()
{
    free(mLocale);
}

static PVMFStatus parseMP3(const char *filename, MediaScannerClient& client)
{
    PVID3ParCom pvId3Param;
    PVFile fileHandle;
    Oscl_FileServer iFs;
    uint32 duration;

    if (iFs.Connect() != 0)
    {
        LOGE("iFs.Connect failed\n");
        return PVMFFailure;
    }

    oscl_wchar output[MAX_BUFF_SIZE];
    oscl_UTF8ToUnicode((const char *)filename, oscl_strlen((const char *)filename), (oscl_wchar *)output, MAX_BUFF_SIZE);
    if (0 != fileHandle.Open((oscl_wchar *)output, Oscl_File::MODE_READ | Oscl_File::MODE_BINARY, iFs) )
    {
        LOGE("Could not open the input file for reading(Test: parse id3).\n");
        return PVMFFailure;
    }

    fileHandle.Seek(0, Oscl_File::SEEKSET);
    pvId3Param.ParseID3Tag(&fileHandle);
    fileHandle.Close();
    iFs.Close();

    //Get the frames information from ID3 library
    PvmiKvpSharedPtrVector framevector;
    pvId3Param.GetID3Frames(framevector);

    uint32 num_frames = framevector.size();

    for (uint32 i = 0; i < num_frames;i++)
    {
        const char* key = framevector[i]->key;
        bool validUtf8 = true;

        // type should follow first semicolon
        const char* type = strchr(key, ';') + 1;
        if (type == 0) continue;

        
        const char* value = framevector[i]->value.pChar_value;

        // KVP_VALTYPE_UTF8_CHAR check must be first, since KVP_VALTYPE_ISO88591_CHAR 
        // is a substring of KVP_VALTYPE_UTF8_CHAR.
        // Similarly, KVP_VALTYPE_UTF16BE_WCHAR must be checked before KVP_VALTYPE_UTF16_WCHAR
        if (oscl_strncmp(type, KVP_VALTYPE_UTF8_CHAR, KVP_VALTYPE_UTF8_CHAR_LEN) == 0) {
            // utf8 can be passed through directly
            // but first validate to make sure it is legal utf8
            uint32 valid_chars;
            validUtf8 = oscl_str_is_valid_utf8((const uint8 *)value, valid_chars);
            if (validUtf8 && !client.handleStringTag(key, value)) goto failure;
        } 

        // if the value is not valid utf8, then we will treat it as iso-8859-1 
        // and our native encoding detection will try to figure out what it is
        if (oscl_strncmp(type, KVP_VALTYPE_ISO88591_CHAR, KVP_VALTYPE_ISO88591_CHAR_LEN) == 0 
                || !validUtf8) {
            // iso-8859-1
            // convert to utf8
            // worse case is 2x inflation
            const unsigned char* src = (const unsigned char *)value;
            char* temp = (char *)alloca(strlen(value) * 2 + 1);
            if (temp) {
                char* dest = temp;
                unsigned int uch;
                while ((uch = *src++) != 0) {
                    if (uch & 0x80) {
                        *dest++ = (uch >> 6) | 0xc0;
                        *dest++ = (uch & 0x3f) | 0x80;
                    } else *dest++ = uch;
                }
                *dest = 0;
                if (!client.addStringTag(key, temp)) goto failure;           
            }
        } else if (oscl_strncmp(type, KVP_VALTYPE_UTF16BE_WCHAR, KVP_VALTYPE_UTF16BE_WCHAR_LEN) == 0 ||
                oscl_strncmp(type, KVP_VALTYPE_UTF16_WCHAR, KVP_VALTYPE_UTF16_WCHAR_LEN) == 0) {
            // convert wchar to utf8
            // the id3parcom library has already taken care of byteswapping
            const oscl_wchar*  src = framevector[i]->value.pWChar_value;
            int srcLen = oscl_strlen(src);
            // worse case is 3 bytes per character, plus zero termination
            int destLen = srcLen * 3 + 1;
            char* dest = (char *)alloca(destLen);

            if (oscl_UnicodeToUTF8(src, oscl_strlen(src), dest, destLen) > 0) {
                if (!client.addStringTag(key, dest)) goto failure;           
            }                 
        } else if (oscl_strncmp(type, KVP_VALTYPE_UINT32, KVP_VALTYPE_UINT32_LEN) == 0) {
            char temp[20];
            snprintf(temp, sizeof(temp), "%d", (int)framevector[i]->value.uint32_value);
            if (!client.addStringTag(key, temp)) goto failure;
        } else {
            //LOGE("unknown tag type %s for key %s\n", type, key);
        }
    }

    // extract non-ID3 properties below
    {
        OSCL_wHeapString<OsclMemAllocator> mp3filename(output);
        MP3ErrorType    err;
        IMpeg3File mp3File(mp3filename, err);
        if (err != MP3_SUCCESS) {
            LOGE("IMpeg3File constructor returned %d.\n", err);
            return err;
        }
        err = mp3File.ParseMp3File();
        if (err != MP3_SUCCESS) {
            LOGE("IMpeg3File::ParseMp3File returned %d.\n", err);
            return err;
        }

        char buffer[20];
        duration = mp3File.GetDuration();
        sprintf(buffer, "%d", duration);
        if (!client.addStringTag("duration", buffer)) goto failure;
    }

    return PVMFSuccess;

failure:
    return PVMFFailure;
}

static PVMFStatus reportM4ATags(IMpeg4File *mp4Input, MediaScannerClient& client)
{

    OSCL_wHeapString<OsclMemAllocator> valuestring=NULL;
    MP4FFParserOriginalCharEnc charType = ORIGINAL_CHAR_TYPE_UNKNOWN;
    uint16 iLangCode=0;
    uint64 duration;
    uint32 timeScale;
    uint16 trackNum;
    uint16 totalTracks;
    uint32 val;

    char buffer[MAX_STR_LEN];

    // Title
    uint32 i = 0;
    for (i = 0; i < mp4Input->getNumTitle(); ++i)
    {
        mp4Input->getTitle(i,valuestring,iLangCode,charType);
        if (oscl_UnicodeToUTF8(valuestring.get_cstr(),valuestring.get_size(),
            buffer,sizeof(buffer)) > 0)
        {
            if (!client.addStringTag("title", buffer)) goto failure;
            break;
        }
    }

    // Artist
    for (i = 0; i < mp4Input->getNumArtist(); ++i)
    {
        mp4Input->getArtist(i,valuestring,iLangCode,charType);
        if (oscl_UnicodeToUTF8(valuestring.get_cstr(),valuestring.get_size(),
            buffer,sizeof(buffer)) > 0)
        {
            if (!client.addStringTag("artist", buffer)) goto failure; 
            break;
        }
    }

    // Album
    for (i = 0; i < mp4Input->getNumAlbum(); ++i)
    {
        mp4Input->getAlbum(i,valuestring,iLangCode,charType);
        if (oscl_UnicodeToUTF8(valuestring.get_cstr(),valuestring.get_size(),
            buffer,sizeof(buffer)) > 0)
        {
            if (!client.addStringTag("album", buffer)) goto failure;
            break;
        }
    }

    // Year
    val = 0;
    for (i = 0; i < mp4Input->getNumYear(); ++i)
    {
        mp4Input->getYear(i,val);
        sprintf(buffer, "%d", val);
        if (buffer[0])
        {
            if (!client.addStringTag("year", buffer)) goto failure;
            break;
        }
    }

    // Writer/Composer
    if (oscl_UnicodeToUTF8(mp4Input->getITunesWriter().get_cstr(),
        mp4Input->getITunesWriter().get_size(),buffer,sizeof(buffer)) > 0)
        if (!client.addStringTag("composer", buffer)) goto failure;

    // Track Data
    trackNum = mp4Input->getITunesThisTrackNo();
    totalTracks = mp4Input->getITunesTotalTracks();
    sprintf(buffer, "%d/%d", trackNum, totalTracks);
    if (!client.addStringTag("tracknumber", buffer)) goto failure;

    // Duration
    duration = mp4Input->getMovieDuration();
    timeScale =  mp4Input->getMovieTimescale();
    // adjust duration to milliseconds if necessary
    if (timeScale != 1000)
        duration = (duration * 1000) / timeScale;
    sprintf(buffer, "%lld", duration);
    if (!client.addStringTag("duration", buffer)) goto failure;

    // Genre
    buffer[0] = 0;
    for(i=0; i<mp4Input->getNumGenre(); i++)
    {
        mp4Input->getGenre(i,valuestring,iLangCode,charType);
        if (oscl_UnicodeToUTF8(valuestring.get_cstr(),valuestring.get_size(), buffer,sizeof(buffer)) > 0)
            break;
    }
    if (buffer[0]) {
        if (!client.addStringTag("genre", buffer)) goto failure;
    } else {
        uint16 id = mp4Input->getITunesGnreID();
        if (id > 0) {
            sprintf(buffer, "(%d)", id - 1);
            if (!client.addStringTag("genre", buffer)) goto failure;
        }
    }

    return PVMFSuccess;

failure:
    return PVMFFailure;
}

static PVMFStatus parseMP4(const char *filename, MediaScannerClient& client)
{
    PVFile fileHandle;
    Oscl_FileServer iFs;

    if (iFs.Connect() != 0)
    {
        LOGE("Connection with the file server for the parse id3 test failed.\n");
        return PVMFFailure;
    }

    oscl_wchar output[MAX_BUFF_SIZE];
    oscl_UTF8ToUnicode((const char *)filename, oscl_strlen((const char *)filename), (oscl_wchar *)output, MAX_BUFF_SIZE);
    OSCL_wHeapString<OsclMemAllocator> mpegfilename(output);

    IMpeg4File *mp4Input = IMpeg4File::readMP4File(mpegfilename, NULL, NULL, 1 /* parsing_mode */, &iFs);
    if (mp4Input)
    {
        // check to see if the file contains video
        int32 count = mp4Input->getNumTracks();
        uint32* tracks = new uint32[count];
        bool hasAudio = false;
        bool hasVideo = false;
        if (tracks) {
            mp4Input->getTrackIDList(tracks, count);
            for (int i = 0; i < count; ++i) {
                uint32 trackType = mp4Input->getTrackMediaType(tracks[i]);
                OSCL_HeapString<OsclMemAllocator> streamtype;
                mp4Input->getTrackMIMEType(tracks[i], streamtype);
                char streamtypeutf8[128];
		strncpy (streamtypeutf8, streamtype.get_str(), streamtype.get_size());
                if (streamtypeutf8[0])
		{                                                                           
                    if (strcmp(streamtypeutf8,"FORMATUNKNOWN") != 0) {
							if (trackType ==  MEDIA_TYPE_AUDIO) {
								hasAudio = true;
							} else if (trackType ==  MEDIA_TYPE_VISUAL) {
								hasVideo = true;
							}
                    } else {
                        //LOGI("@@@@@@@@ %100s: %s\n", filename, streamtypeutf8);
                    }
                }
            }

            delete[] tracks;
        }

        if (hasVideo) {
            if (!client.setMimeType("video/mp4")) return PVMFFailure;
        } else if (hasAudio) {
            if (!client.setMimeType("audio/mp4")) return PVMFFailure;
        } else {
            iFs.Close();
            IMpeg4File::DestroyMP4FileObject(mp4Input);
            return PVMFFailure;
        }

        PVMFStatus result = reportM4ATags(mp4Input, client);
        iFs.Close();
        IMpeg4File::DestroyMP4FileObject(mp4Input);
        return result;
    }

    return PVMFSuccess;
}

static PVMFStatus parseOgg(const char *filename, MediaScannerClient& client)
{
    int duration;

    FILE *file = fopen(filename,"r");
    if (!file)
        return PVMFFailure;

    OggVorbis_File vf;
    if (ov_open(file, &vf, NULL, 0) < 0) {
        return PVMFFailure;
    }

    char **ptr=ov_comment(&vf,-1)->user_comments;
    while(*ptr){
        char *val = strstr(*ptr, "=");
        if (val) {
            int keylen = val++ - *ptr;
            char key[keylen + 1];
            strncpy(key, *ptr, keylen);
            key[keylen] = 0;
            if (!client.addStringTag(key, val)) goto failure;
        }
        ++ptr;
    }

    // Duration
    duration = ov_time_total(&vf, -1);
    if (duration > 0) {
        char buffer[20];
        sprintf(buffer, "%d", duration);
        if (!client.addStringTag("duration", buffer)) goto failure;
    }

    ov_clear(&vf); // this also closes the FILE
    return PVMFSuccess;

failure:
    ov_clear(&vf); // this also closes the FILE
    return PVMFFailure;
}

static PVMFStatus parseMidi(const char *filename, MediaScannerClient& client) {

    // get the library configuration and do sanity check
    const S_EAS_LIB_CONFIG* pLibConfig = EAS_Config();
    if ((pLibConfig == NULL) || (LIB_VERSION != pLibConfig->libVersion)) {
        LOGE("EAS library/header mismatch\n");
        return PVMFFailure;
    }
    EAS_I32 temp;

    // spin up a new EAS engine
    EAS_DATA_HANDLE easData = NULL;
    EAS_HANDLE easHandle = NULL;
    EAS_RESULT result = EAS_Init(&easData);
    if (result == EAS_SUCCESS) {
        EAS_FILE file;
        file.path = filename;
        file.fd = 0;
        file.offset = 0;
        file.length = 0;
        result = EAS_OpenFile(easData, &file, &easHandle);
    }
    if (result == EAS_SUCCESS) {
        result = EAS_Prepare(easData, easHandle);
    }
    if (result == EAS_SUCCESS) {
        result = EAS_ParseMetaData(easData, easHandle, &temp);
    }
    if (easHandle) {
        EAS_CloseFile(easData, easHandle);
    }
    if (easData) {
        EAS_Shutdown(easData);
    }

    if (result != EAS_SUCCESS) {
        return PVMFFailure;
    }

    char buffer[20];
    sprintf(buffer, "%ld", temp);
    if (!client.addStringTag("duration", buffer)) return PVMFFailure;
    return PVMFSuccess;
}

static PVMFStatus parseWMA(const char *filename, MediaScannerClient& client)
{
    sp<MediaMetadataRetriever> retriever = new MediaMetadataRetriever();
    retriever->setMode( 1 /*MediaMetadataRetriever.MODE_GET_METADATA_ONLY*/);
    status_t status = retriever->setDataSource(filename);
    if (status != NO_ERROR) {
        LOGE("parseWMA setDataSource failed (%d)", status);
        retriever->disconnect();
        return PVMFFailure;
    }

    const char* value;

    value = retriever->extractMetadata(METADATA_KEY_IS_DRM_CRIPPLED);
    if (value && strcmp(value, "true") == 0) {
        // we don't support WMDRM currently
        // setting this invalid mimetype will make the java side ignore this file
        client.setMimeType("audio/x-wma-drm");
    }
    value = retriever->extractMetadata(METADATA_KEY_CODEC);
    if (value && strcmp(value, "Windows Media Audio 10 Professional") == 0) {
        // we don't support WM 10 Professional currently
        // setting this invalid mimetype will make the java side ignore this file
        client.setMimeType("audio/x-wma-10-professional");
    }

    value = retriever->extractMetadata(METADATA_KEY_ALBUM);
    if (value)
        client.addStringTag("album", value);

    // Look for "author" tag first, if it is not found, try "artist" tag
    value = retriever->extractMetadata(METADATA_KEY_AUTHOR);
    if (!value) {
        value = retriever->extractMetadata(METADATA_KEY_ARTIST);
    }
    if (value)
        client.addStringTag("artist", value);
    value = retriever->extractMetadata(METADATA_KEY_COMPOSER);
    if (value)
        client.addStringTag("composer", value);
    value = retriever->extractMetadata(METADATA_KEY_GENRE);
    if (value)
        client.addStringTag("genre", value);
    value = retriever->extractMetadata(METADATA_KEY_TITLE);
    if (value)
        client.addStringTag("title", value);
    value = retriever->extractMetadata(METADATA_KEY_YEAR);
    if (value)
        client.addStringTag("year", value);
    value = retriever->extractMetadata(METADATA_KEY_CD_TRACK_NUMBER);
    if (value)
        client.addStringTag("tracknumber", value);

    retriever->disconnect();
    return PVMFSuccess;
}

status_t MediaScanner::processFile(const char *path, const char* mimeType, MediaScannerClient& client)
{
    status_t result;
    InitializeForThread();

    client.setLocale(mLocale);
    client.beginFile();
    
    //LOGD("processFile %s mimeType: %s\n", path, mimeType);
    const char* extension = strrchr(path, '.');

    if (extension && strcasecmp(extension, ".mp3") == 0) {
        result = parseMP3(path, client);
    } else if (extension &&
        (strcasecmp(extension, ".mp4") == 0 || strcasecmp(extension, ".m4a") == 0 ||
         strcasecmp(extension, ".3gp") == 0 || strcasecmp(extension, ".3gpp") == 0 ||
         strcasecmp(extension, ".3g2") == 0 || strcasecmp(extension, ".3gpp2") == 0)) {
        result = parseMP4(path, client);
    } else if (extension && strcasecmp(extension, ".ogg") == 0) {
        result = parseOgg(path, client);
    } else if (extension &&
        ( strcasecmp(extension, ".mid") == 0 || strcasecmp(extension, ".smf") == 0
        || strcasecmp(extension, ".imy") == 0)) {
        result = parseMidi(path, client);
    } else if (extension && strcasecmp(extension, ".wma") == 0) {
        result = parseWMA(path, client);
    } else {
        result = PVMFFailure;
    }

    client.endFile();

    return result;
}

static bool fileMatchesExtension(const char* path, const char* extensions) {
    char* extension = strrchr(path, '.');
    if (!extension) return false;
    ++extension;    // skip the dot
    if (extension[0] == 0) return false;

    while (extensions[0]) {
        char* comma = strchr(extensions, ',');
        size_t length = (comma ? comma - extensions : strlen(extensions));
        if (length == strlen(extension) && strncasecmp(extension, extensions, length) == 0) return true;
        extensions += length;
        if (extensions[0] == ',') ++extensions;
    }

    return false;
}

status_t MediaScanner::doProcessDirectory(char *path, int pathRemaining, const char* extensions,
        MediaScannerClient& client, ExceptionCheck exceptionCheck, void* exceptionEnv)
{
    // place to copy file or directory name
    char* fileSpot = path + strlen(path);
    struct dirent* entry;

    // ignore directories that contain a  ".nomedia" file
    if (pathRemaining >= 8 /* strlen(".nomedia") */ ) {
        strcpy(fileSpot, ".nomedia");
        if (access(path, F_OK) == 0) {
            LOGD("found .nomedia, skipping directory\n");
            return OK;
        }

        // restore path
        fileSpot[0] = 0;
    }

    DIR* dir = opendir(path);
    if (!dir) {
        LOGD("opendir %s failed, errno: %d", path, errno);
        return PVMFFailure;
    }

    while ((entry = readdir(dir))) {
        const char* name = entry->d_name;

        // ignore "." and ".."
        if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0))) {
            continue;
        }

        int type = entry->d_type;
        if (type == DT_REG || type == DT_DIR) {
            int nameLength = strlen(name);
            bool isDirectory = (type == DT_DIR);

            if (nameLength > pathRemaining || (isDirectory && nameLength + 1 > pathRemaining)) {
                // path too long!
                continue;
            }

            strcpy(fileSpot, name);
            if (isDirectory) {
                // ignore directories with a name that starts with '.'
                // for example, the Mac ".Trashes" directory
                if (name[0] == '.') continue;

                strcat(fileSpot, "/");
                int err = doProcessDirectory(path, pathRemaining - nameLength - 1, extensions, client, exceptionCheck, exceptionEnv);
                if (err) {
                    LOGE("Error processing '%s' - skipping\n", path);
                    continue;
                }
            } else if (fileMatchesExtension(path, extensions)) {
                struct stat statbuf;
                stat(path, &statbuf);
                if (statbuf.st_size > 0) {
                    client.scanFile(path, statbuf.st_mtime, statbuf.st_size);
                }
                if (exceptionCheck && exceptionCheck(exceptionEnv)) goto failure;
            }
        }
    }

    closedir(dir);
    return OK;
failure:
    closedir(dir);
    return -1;
}

status_t MediaScanner::processDirectory(const char *path, const char* extensions,
        MediaScannerClient& client, ExceptionCheck exceptionCheck, void* exceptionEnv)
{
    InitializeForThread();

    int pathLength = strlen(path);
    if (pathLength >= PATH_MAX) {
        return PVMFFailure;
    }
    char* pathBuffer = (char *)malloc(PATH_MAX + 1);
    if (!pathBuffer) {
        return PVMFFailure;
    }

    int pathRemaining = PATH_MAX - pathLength;
    strcpy(pathBuffer, path);
    if (pathBuffer[pathLength - 1] != '/') {
        pathBuffer[pathLength] = '/';
        pathBuffer[pathLength + 1] = 0;
        --pathRemaining;
    }

    client.setLocale(mLocale);
    status_t result = doProcessDirectory(pathBuffer, pathRemaining, extensions, client, exceptionCheck, exceptionEnv);

    free(pathBuffer);
    return result;
}

void MediaScanner::setLocale(const char* locale)
{
    if (mLocale) {
        free(mLocale);
        mLocale = NULL;
    }
    if (locale) {
        mLocale = strdup(locale);
    }
}

static char* doExtractAlbumArt(PvmfApicStruct* aApic)
{
    char *data = (char*)malloc(aApic->iGraphicDataLen + 4);
    if (data) {
        long *len = (long*)data;
        *len = aApic->iGraphicDataLen;
        memcpy(data + 4, aApic->iGraphicData, *len);
    }
    return data;
}

static char* extractMP3AlbumArt(int fd)
{
    PVID3ParCom pvId3Param;
    PVFile file;
    OsclFileHandle *filehandle;
    Oscl_FileServer iFs;

    if(iFs.Connect() != 0)
    {
        LOGE("Connection with the file server for the parse id3 test failed.\n");
        return NULL;
    }

    FILE *f = fdopen(fd, "r");
    filehandle = new OsclFileHandle(f);
    file.SetFileHandle(filehandle);

    if( 0 != file.Open(NULL, Oscl_File::MODE_READ | Oscl_File::MODE_BINARY, iFs) )
    {
        LOGE("Could not open the input file for reading(Test: parse id3).\n");
        return NULL;
    }

    file.Seek(0, Oscl_File::SEEKSET);
    pvId3Param.ParseID3Tag(&file);
    file.Close();
    iFs.Close();

    //Get the frames information from ID3 library
    PvmiKvpSharedPtrVector framevector;
    pvId3Param.GetID3Frames(framevector);

    uint32 num_frames = framevector.size();
    for (uint32 i = 0; i < num_frames; i++)
    {
        const char* key = framevector[i]->key;

        // type should follow first semicolon
        const char* type = strchr(key, ';') + 1;
        if (type == 0) continue;
        const char* value = framevector[i]->value.pChar_value;
        const unsigned char* src = (const unsigned char *)value;

        if (oscl_strncmp(key,KVP_KEY_ALBUMART,oscl_strlen(KVP_KEY_ALBUMART)) == 0)
        {
            PvmfApicStruct* aApic = (PvmfApicStruct*)framevector[i]->value.key_specific_value;
            if (aApic) {
                char* result = doExtractAlbumArt(aApic);
                if (result)
                    return result;
            }
        }
    }

    return NULL;
}

static char* extractM4AAlbumArt(int fd)
{
    PVFile file;
    OsclFileHandle *filehandle;
    Oscl_FileServer iFs;
    char* result = NULL;

    if(iFs.Connect() != 0)
    {
         LOGE("Connection with the file server for the parse id3 test failed.\n");
        return NULL;
    }

    FILE *f = fdopen(fd, "r");
    filehandle = new OsclFileHandle(f);
    file.SetFileHandle(filehandle);

    oscl_wchar output[MAX_BUFF_SIZE];
    oscl_UTF8ToUnicode("", 0, (oscl_wchar *)output, MAX_BUFF_SIZE);
    OSCL_wHeapString<OsclMemAllocator> mpegfilename(output);
    IMpeg4File *mp4Input = IMpeg4File::readMP4File(
            mpegfilename, /* name */
            NULL, /* plugin access interface factory */
            filehandle,
            0, /* parsing_mode */
            &iFs);

    if (!mp4Input)
        return NULL;

    PvmfApicStruct* aApic = mp4Input->getITunesImageData();
    if (aApic) {
        result = doExtractAlbumArt(aApic);
    }

    IMpeg4File::DestroyMP4FileObject(mp4Input);
    return result;
}


char* MediaScanner::extractAlbumArt(int fd)
{
    InitializeForThread();

    int32 ident;
    lseek(fd, 4, SEEK_SET);
    read(fd, &ident, sizeof(ident));

    if (ident == 0x70797466) {
        // some kind of mpeg 4 stream
        lseek(fd, 0, SEEK_SET);
        return extractM4AAlbumArt(fd);
    } else {
        // might be mp3
        return extractMP3AlbumArt(fd);
    }
}

MediaScannerClient::MediaScannerClient()
    :   mNames(NULL),
        mValues(NULL),
        mLocaleEncoding(kEncodingNone)
{
}

MediaScannerClient::~MediaScannerClient()
{
    delete mNames;
    delete mValues;
}

void MediaScannerClient::setLocale(const char* locale)
{
    if (!locale) return;
    
    if (!strncmp(locale, "ja", 2))
        mLocaleEncoding = kEncodingShiftJIS;
    else if (!strncmp(locale, "ko", 2))
        mLocaleEncoding = kEncodingEUCKR;
    else if (!strncmp(locale, "zh", 2)) {
        if (!strcmp(locale, "zh_CN")) {
            // simplified chinese for mainland China
            mLocaleEncoding = kEncodingGBK;
        } else {
            // assume traditional for non-mainland Chinese locales (Taiwan, Hong Kong, Singapore)
            mLocaleEncoding = kEncodingBig5;
        }
    }
}

void MediaScannerClient::beginFile()
{
    mNames = new StringArray;
    mValues = new StringArray;
}

bool MediaScannerClient::addStringTag(const char* name, const char* value)
{
    if (mLocaleEncoding != kEncodingNone) {
        // don't bother caching strings that are all ASCII.
        // call handleStringTag directly instead.
        // check to see if value (which should be utf8) has any non-ASCII characters
        bool nonAscii = false;
        const char* chp = value;
        char ch;
        while ((ch = *chp++)) {
            if (ch & 0x80) {
                nonAscii = true;
                break;
            }
        }

        if (nonAscii) {
            // save the strings for later so they can be used for native encoding detection
            mNames->push_back(name);
            mValues->push_back(value); 
            return true;
        }
        // else fall through
    }

    // autodetection is not necessary, so no need to cache the values
    // pass directly to the client instead
    return handleStringTag(name, value);
}

static uint32_t possibleEncodings(const char* s)
{
    uint32_t result = kEncodingAll;
    // if s contains a native encoding, then it was mistakenly encoded in utf8 as if it were latin-1
    // so we need to reverse the latin-1 -> utf8 conversion to get the native chars back
    uint8 ch1, ch2;
    uint8* chp = (uint8 *)s;
    
    while ((ch1 = *chp++)) {
        if (ch1 & 0x80) {
            ch2 = *chp++;
            ch1 = ((ch1 << 6) & 0xC0) | (ch2 & 0x3F);
            // ch1 is now the first byte of the potential native char 
            
            ch2 = *chp++;
            if (ch2 & 0x80)
                ch2 = ((ch2 << 6) & 0xC0) | (*chp++ & 0x3F);
            // ch2 is now the second byte of the potential native char
            int ch = (int)ch1 << 8 | (int)ch2;
            result &= findPossibleEncodings(ch);
        }
        // else ASCII character, which could be anything
    }

    return result;
}

void MediaScannerClient::convertValues(uint32_t encoding)
{
    const char* enc = NULL;
    switch (encoding) {
        case kEncodingShiftJIS:
            enc = "shift-jis";
            break;
        case kEncodingGBK:
            enc = "gbk";
            break;
        case kEncodingBig5:
            enc = "Big5";
            break;
        case kEncodingEUCKR:
            enc = "EUC-KR";
            break;
    }

    if (enc) {
        UErrorCode status = U_ZERO_ERROR;

        UConverter *conv = ucnv_open(enc, &status);
        if (U_FAILURE(status)) {
            LOGE("could not create UConverter for %s\n", enc);
            return;
        }
        UConverter *utf8Conv = ucnv_open("UTF-8", &status);
        if (U_FAILURE(status)) {
            LOGE("could not create UConverter for UTF-8\n");
            ucnv_close(conv);
            return;
        }

        // for each value string, convert from native encoding to UTF-8
        for (int i = 0; i < mNames->size(); i++) {
            // first we need to untangle the utf8 and convert it back to the original bytes
            // since we are reducing the length of the string, we can do this in place
            uint8* src = (uint8 *)mValues->getEntry(i);
            int len = strlen((char *)src);
            uint8* dest = src;

            uint8 uch;
            while ((uch = *src++)) {
                if (uch & 0x80)
                    *dest++ = ((uch << 6) & 0xC0) | (*src++ & 0x3F);
                else
                    *dest++ = uch;
            }
            *dest = 0;

            // now convert from native encoding to UTF-8
            const char* source = mValues->getEntry(i);
            int targetLength = len * 3 + 1;
            char* buffer = new char[targetLength];
            if (!buffer)
                break;
            char* target = buffer;

            ucnv_convertEx(utf8Conv, conv, &target, target + targetLength,
                    &source, (const char *)dest, NULL, NULL, NULL, NULL, TRUE, TRUE, &status);
            if (U_FAILURE(status)) {
                LOGE("ucnv_convertEx failed: %d\n", status);
                mValues->setEntry(i, "???");
            } else {
                // zero terminate
                *target = 0;
                mValues->setEntry(i, buffer);
            }         

            delete[] buffer;
        }

        ucnv_close(conv);
        ucnv_close(utf8Conv);
    }
}

void MediaScannerClient::endFile()
{
    if (mLocaleEncoding != kEncodingNone) {
        int size = mNames->size();
        uint32_t encoding = kEncodingAll;
        
        // compute a bit mask containing all possible encodings
        for (int i = 0; i < mNames->size(); i++)
            encoding &= possibleEncodings(mValues->getEntry(i));
        
        // if the locale encoding matches, then assume we have a native encoding.
        if (encoding & mLocaleEncoding)
            convertValues(mLocaleEncoding);
        
        // finally, push all name/value pairs to the client
        for (int i = 0; i < mNames->size(); i++) {
            if (!handleStringTag(mNames->getEntry(i), mValues->getEntry(i)))
                break;
        }
    }
    // else addStringTag() has done all the work so we have nothing to do
    
    delete mNames;
    delete mValues;
    mNames = NULL;
    mValues = NULL;
}

}; // namespace android
