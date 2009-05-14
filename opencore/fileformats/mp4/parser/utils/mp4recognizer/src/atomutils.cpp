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
/*                            MPEG-4 Atom Utils Class                            */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This AtomUtils Class contains sime useful methods for operating on Atoms
*/


#define IMPLEMENT_AtomUtils
#include "oscl_base.h"
#include "oscl_int64_utils.h"
#include "atomutils.h"
#include "atomdefs.h"
#include "oscl_utf8conv.h"

// Read in the 64 bits byte by byte and take most significant byte first
OSCL_EXPORT_REF bool
AtomUtils::read64(MP4_FF_FILE *fp, uint64 &data)
{
    const int32 N = 8;
    uint8 bytes[N];
    data = 0;

    int32 retVal = 0;

    retVal = (int32) fp->_pvfile.Read((void*)bytes, 1, N);

    if (retVal < N)
    {
        return false;
    }

    uint32 low  = 0;
    uint32 high = 0;

    int32 i;
    for (i = 0; i < N / 2; i++)
    {
        high = (high << 8) | bytes[i];
    }
    for (i = N / 2; i < N; i++)
    {
        low = (low << 8) | bytes[i];
    }
    Oscl_Int64_Utils::set_uint64(data, high, low);
    return true;
}


// Read in the 32 bits byte by byte and take most significant byte first
OSCL_EXPORT_REF bool
AtomUtils::read32(MP4_FF_FILE *fp, uint32 &data)
{
    const int32 N = 4;
    uint8 bytes[N];
    data = 0;

    int32 retVal = 0;

    retVal = (int32)(fp->_pvfile.Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    for (int32 i = 0;i < N;i++)
        data = (data << 8) | bytes[i];

    return true;
}

// Read in the 32 bits byte by byte and take most significant byte first.
// This is equivalent to two read32 calls.
OSCL_EXPORT_REF bool
AtomUtils::read32read32(MP4_FF_FILE *fp, uint32 &data1, uint32 &data2)
{
    const int32 N = 8;
    uint8 bytes[N];
    data1 = 0;
    data2 = 0;

    int32 retVal = 0;

    retVal = (int32)(fp->_pvfile.Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    int32 i;
    for (i = 0;i < 4;i++)
        data1 = (data1 << 8) | bytes[i];

    for (i = 4;i < 8;i++)
        data2 = (data2 << 8) | bytes[i];

    return true;
}

// Read in the 24 bits byte by byte and take most significant byte first
OSCL_EXPORT_REF bool
AtomUtils::read24(MP4_FF_FILE *fp, uint32 &data)
{
    const int32 N = 3;
    uint8 bytes[N];
    data = 0;

    int32 retVal = 0;

    retVal = (int32)(fp->_pvfile.Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    for (int32 i = 0;i < N;i++)
        data = (data << 8) | bytes[i];

    return true;
}

// Read in the 16 bits byte by byte and take most significant byte first
OSCL_EXPORT_REF bool
AtomUtils::read16(MP4_FF_FILE *fp, uint16 &data)
{
    const int32 N = 2;
    uint8 bytes[N];
    data = 0;

    int32 retVal = 0;

    retVal = (int32)(fp->_pvfile.Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    for (int32 i = 0;i < N;i++)
        data = (uint16)((data << 8) | (uint16) bytes[i]);

    return true;
}

// Read in the 16 bits byte by byte and take most significant byte first
// This is equivalent to two read16 calls
OSCL_EXPORT_REF bool
AtomUtils::read16read16(MP4_FF_FILE *fp, uint16 &data1, uint16 &data2)
{
    const int32 N = 4;
    uint8 bytes[N];
    data1 = 0;
    data2 = 0;

    int32 retVal = 0;

    retVal = (int32)(fp->_pvfile.Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    int32 i;
    for (i = 0;i < 2;i++)
        data1 = (uint16)((data1 << 8) | (uint16) bytes[i]);

    for (i = 2;i < 4;i++)
        data2 = (uint16)((data2 << 8) | (uint16) bytes[i]);

    return true;
}

// Read in the 8 bit byte
OSCL_EXPORT_REF bool
AtomUtils::read8(MP4_FF_FILE *fp, uint8 &data)
{
    data = 0;

    int32 retVal = 0;

    retVal = (int32)(fp->_pvfile.Read((void*) & data, 1, 1));

    if (retVal < 1)
        return false;

    return true;
}

// Read in the 8 bit byte
// This is equivalent to two read8 calls
OSCL_EXPORT_REF bool
AtomUtils::read8read8(MP4_FF_FILE *fp, uint8 &data1, uint8 &data2)
{
    const int32 N = 2;
    uint8 bytes[N];
    data1 = 0;
    data2 = 0;

    int32 retVal = 0;

    retVal = (int32)(fp->_pvfile.Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    data1 = bytes[0];
    data2 = bytes[1];

    return true;
}
// Read in a NULL terminated string byte by byte and take most significant byte first
// and convert to a OSCL_wString
OSCL_EXPORT_REF bool
AtomUtils::readNullTerminatedString(MP4_FF_FILE *fp, OSCL_wString& data)
{
    const int MAX_BUFF_SIZE = 1024;
    uint8 buf[MAX_BUFF_SIZE];
    int32 index = 0;

    if (!AtomUtils::read8(fp, buf[index]))
        return false;

    bool nextChar = (buf[index] == 0) ? false : true;

    while (nextChar && (index < MAX_BUFF_SIZE))
    {
        index++;

        if (!AtomUtils::read8(fp, buf[index]))
            return false;

        nextChar = (buf[index] == 0) ? false : true;
    }
    // String buffer filled - now create OSCL_wString

    OSCL_TCHAR outbuf[MAX_BUFF_SIZE];
    oscl_UTF8ToUnicode((const char *)buf, index, outbuf, MAX_BUFF_SIZE);
    OSCL_wHeapString<OsclMemAllocator> temp(outbuf);

    data = temp;
    return true;
}

// Read in a NULL terminated UNICODE string byte by byte and take most significant byte first
// and convert to a OSCL_wString
OSCL_EXPORT_REF bool
AtomUtils::readNullTerminatedUnicodeString(MP4_FF_FILE *fp, OSCL_wString& data)
{
    const int MAX_BUFF_SIZE = 1024;
    oscl_wchar buf[MAX_BUFF_SIZE];
    int32 index = 0;

    // Need to be careful of the byte-ordering when creating the oscl_wchar array
    uint8 firstbyte;
    uint8 secondbyte;

    if (!AtomUtils::read8read8(fp, firstbyte, secondbyte))
        return false;

    // Allow the OS to do the bit shifting to get the correct byte ordering
    // for the CHAR value
    oscl_wchar *wptr = &buf[index];

    *wptr = (uint16)(firstbyte << 8 | (uint16) secondbyte);

    bool nextChar = (buf[index] == 0) ? false : true;
    index += 1;

    while (nextChar && (index < (int32)(MAX_BUFF_SIZE)))
    {
        if (!AtomUtils::read8read8(fp, firstbyte, secondbyte))
            return false;

        // Allow the OS to do the bit shifting to get the correct byte ordering
        // for the CHAR value
        wptr  = &buf[index];
        *wptr = (uint16)(firstbyte << 8 | (uint16) secondbyte);
        nextChar = (buf[index] == 0) ? false : true;
        index++;
    }

    // String (oscl_wchar) buffer filled - now create OSCL_STRING

    // OSCL_STRING is a oscl_wchar string so no conversion needed
    OSCL_wHeapString<OsclMemAllocator> temp((const OSCL_TCHAR *)buf, index - 1);
    data = temp;

    return true;
}

// Read in a NULL terminated ascii (8-bit char) string byte by byte and take most
// significant byte first and convert to a OSCL_wString
OSCL_EXPORT_REF bool AtomUtils::readNullTerminatedAsciiString(MP4_FF_FILE *fp, OSCL_wString& data)
{
    return readNullTerminatedString(fp, data);
}

// Read in a first N Characters of string byte by byte
OSCL_EXPORT_REF bool
AtomUtils::readByteData(MP4_FF_FILE *fp, uint32 length, uint8 *data)
{
    uint32 bytesRead = 0;

    if (length > 0)
    {
        bytesRead = fp->_pvfile.Read(data, 1, length);
    }
    if (bytesRead < (uint32)length)
    {
        // read byte data failed
        return false;
    }
    return true;
}

// Read in byte data and take most significant byte first
OSCL_EXPORT_REF bool
AtomUtils::readUnicodeData(MP4_FF_FILE *fp, uint32 length, uint16 *data)
{
    uint32 wordsRead;

    if (length == 0)
        return false;

    wordsRead = (int32)(fp->_pvfile.Read((void*)data, 2, length));

    // read byte data failed
    if (wordsRead < (uint32)length)
    {
        return false;
    }

    return true;
}

OSCL_EXPORT_REF bool
AtomUtils::readString(MP4_FF_FILE *fp, uint32 inLength, MP4FFParserOriginalCharEnc &CharType, OSCL_wString& data)
{
    uint32 temp = AtomUtils::peekNextNthBytes(fp, 1);
    uint16 byteOrderMask = (uint16)((temp >> 16) & 0xFFFF);

    if (byteOrderMask == BYTE_ORDER_MASK)
    {
        // UTF16
        CharType = ORIGINAL_CHAR_TYPE_UTF16;
        if (!AtomUtils::read16(fp, byteOrderMask))
        {
            return false;
        }

        if (inLength < BYTE_ORDER_MASK_SIZE)
        {
            return false;
        }

        uint32 delta = (inLength - BYTE_ORDER_MASK_SIZE);

        int32 filePos = AtomUtils::getCurrentFilePosition(fp);

        if (!AtomUtils::readUnicodeString(fp, delta, data))
        {
            return false;
        }
        int32 newfilePos = AtomUtils::getCurrentFilePosition(fp);
        if (newfilePos != (int32)(filePos + delta))
        {
            AtomUtils::seekFromStart(fp, filePos + delta);
        }
    }
    else
    {
        // UTF8 or Ascii
        // Check to see if the string is actually null-terminated
        CharType = ORIGINAL_CHAR_TYPE_UTF8;

        uint32 delta = inLength;

        int32 filePos = AtomUtils::getCurrentFilePosition(fp);

        if (!AtomUtils::readUTF8String(fp, delta, data))
        {
            return false;
        }
        int32 newfilePos = AtomUtils::getCurrentFilePosition(fp);
        if (newfilePos != (int32)(filePos + delta))
        {
            AtomUtils::seekFromStart(fp, filePos + delta);
        }
    }
    return true;
}

OSCL_EXPORT_REF bool
AtomUtils::readUTF8String(MP4_FF_FILE *fp, uint32 inLength, OSCL_wString& data)
{
    const int MAX_BUFF_SIZE = 1024;
    uint8 buf[MAX_BUFF_SIZE];

    uint32 max_length = (uint32)MAX_BUFF_SIZE;
    if (inLength > max_length)
    {
        inLength = max_length;
    }

    uint32 index = 0;
    bool nextChar = true;

    while (nextChar && (index < inLength))
    {

        if (!AtomUtils::read8(fp, buf[index]))
            return false;

        nextChar = (buf[index] == 0) ? false : true;

        index++;

    }
    // String buffer filled - now create ZString

    OSCL_TCHAR outbuf[MAX_BUFF_SIZE];
    oscl_UTF8ToUnicode((const char *)buf, (int32)index, outbuf, MAX_BUFF_SIZE);
    OSCL_wHeapString<OsclMemAllocator> temp(outbuf);

    data = temp;
    return true;
}

OSCL_EXPORT_REF bool
AtomUtils::readAsciiString(MP4_FF_FILE *fp, uint32 inLength, OSCL_wString& data)
{
    return (AtomUtils::readUTF8String(fp, inLength, data));
}

OSCL_EXPORT_REF bool
AtomUtils::readUnicodeString(MP4_FF_FILE *fp, uint32 inLength, OSCL_wString& data)
{
    // make inLength an even.
    inLength -= (inLength & 1);

    if (inLength == 0)
    {
        data = NULL;
        return true;
    }

    const int MAX_BUFF_SIZE = 1024;
    oscl_wchar buf[MAX_BUFF_SIZE];
    if (inLength > MAX_BUFF_SIZE * sizeof(oscl_wchar))
    {
        inLength = MAX_BUFF_SIZE;
    }

    uint32 wlength = inLength / 2;

    uint32 index = 0;

    // Need to be careful of the byte-ordering when creating the oscl_wchar array
    uint8 firstbyte;
    uint8 secondbyte;

    oscl_wchar *wptr;

    bool nextChar = true;

    while (nextChar && (index < wlength))
    {
        if (!AtomUtils::read8read8(fp, firstbyte, secondbyte))
            return false;

        // Allow the OS to do the bit shifting to get the correct byte ordering
        // for the CHAR value
        wptr  = &buf[index];
        *wptr = (uint16)(firstbyte << 8 | (uint16) secondbyte);
        nextChar = (buf[index] == 0) ? false : true;
        index++;
    }

    // if it is a NULL terminated string, don't count the last NULL as length.
    if (nextChar == false)
    {
        index --;
    }

    // OSCL_STRING is a oscl_wchar string so no conversion needed
    OSCL_wHeapString<OsclMemAllocator> temp((const OSCL_TCHAR *)buf, index);
    data = temp;

    return true;
}

OSCL_EXPORT_REF uint32
AtomUtils::getNumberOfBytesUsedToStoreSizeOfClass(uint32 contentSize)
{
    // The actual _sizeOfClass value includes the size of the class's contents PLUS
    // the number of bytes needed to store the _sizeOfClass field. The parameter
    // contentSize represents the number of bytes needed to store ONLY the members
    // of the class NOT including the _sizeOfClass field.
    if (contentSize <= 0x7e) return 1; // _sizeOfClass field can be rendered in 1 byte (7 LS bits)
    else if (contentSize <= 0x3ffd) return 2; // _sizeOfClass field can be rendered in 2 bytes (7 LS bits each)
    else if (contentSize <= 0x1ffffc) return 3; // _sizeOfClass field can be rendered in 3 bytes (7 LS bits each)
    else if (contentSize <= 0xfffffffb) return 4; // _sizeOfClass field can be rendered in 4 bytes (7 LS bits each)
    else return 0; // ERROR condition
}


// Returns the atom type from parsing the input stream
OSCL_EXPORT_REF void
AtomUtils::getNextAtomType(MP4_FF_FILE *fp, uint32 &size, uint32 &type)
{
    size = 0;
    type = UNKNOWN_ATOM;

    int32 filePointer;
    filePointer = AtomUtils::getCurrentFilePosition(fp);

    if (filePointer > (fp->_fileSize - 8))
    {
        return;
    }

    if (!AtomUtils::read32read32(fp, size, type))
    {
        size = 0;
        type = UNKNOWN_ATOM;
        return;
    }

    if (type == MOVIE_ATOM ||
            type == MOVIE_HEADER_ATOM ||
            type == TRACK_ATOM ||
            type == TRACK_HEADER_ATOM ||
            type == TRACK_REFERENCE_ATOM ||
            type == MEDIA_ATOM ||
            type == EDIT_ATOM ||
            type == EDIT_LIST_ATOM ||
            type == MEDIA_HEADER_ATOM ||
            type == HANDLER_ATOM ||
            type == MEDIA_INFORMATION_ATOM ||
            type == VIDEO_MEDIA_HEADER_ATOM ||
            type == SOUND_MEDIA_HEADER_ATOM ||
            type == HINT_MEDIA_HEADER_ATOM ||
            type == MPEG4_MEDIA_HEADER_ATOM ||
            type == NULL_MEDIA_HEADER_ATOM ||
            type == DATA_INFORMATION_ATOM ||
            type == DATA_REFERENCE_ATOM ||
            type == DATA_ENTRY_URL_ATOM ||
            type == DATA_ENTRY_URN_ATOM ||
            type == SAMPLE_TABLE_ATOM ||
            type == TIME_TO_SAMPLE_ATOM ||
            type == COMPOSITION_OFFSET_ATOM ||
            type == SAMPLE_DESCRIPTION_ATOM ||
            type == ESD_ATOM ||
            type == SAMPLE_SIZE_ATOM ||
            type == SAMPLE_TO_CHUNK_ATOM ||
            type == CHUNK_OFFSET_ATOM ||
            type == SYNC_SAMPLE_ATOM ||
            type == SHADOW_SYNC_SAMPLE_ATOM ||
            type == DEGRADATION_PRIORITY_ATOM ||
            type == OBJECT_DESCRIPTOR_ATOM ||
            type == MEDIA_DATA_ATOM ||
            type == FREE_SPACE_ATOM ||
            type == SKIP_ATOM ||
            type == USER_DATA_ATOM ||
            type == FILE_TYPE_ATOM ||
            type == PVUSER_DATA_ATOM ||
            type == PV_CONTENT_TYPE_ATOM ||
            type == AMR_SAMPLE_ENTRY_ATOM ||
            type == AMRWB_SAMPLE_ENTRY_ATOM ||
            type == H263_SAMPLE_ENTRY_ATOM ||
            type == AUDIO_SAMPLE_ENTRY ||
            type == VIDEO_SAMPLE_ENTRY ||
            type == MPEG_SAMPLE_ENTRY ||
            type == UUID_ATOM ||
            type == AMR_SPECIFIC_ATOM ||
            type == H263_SPECIFIC_ATOM ||


            type == COPYRIGHT_ATOM ||
            type == FONT_TABLE_ATOM ||
            type == HINT_TRACK_REFERENCE_TYPE ||
            type == DPND_TRACK_REFERENCE_TYPE ||
            type == IPIR_TRACK_REFERENCE_TYPE ||
            type == MPOD_TRACK_REFERENCE_TYPE ||
            type == SYNC_TRACK_REFERENCE_TYPE ||
            type == ASSET_INFO_TITLE_ATOM ||
            type == ASSET_INFO_DESCP_ATOM ||
            type == ASSET_INFO_PERF_ATOM ||
            type == ASSET_INFO_AUTHOR_ATOM ||
            type == ASSET_INFO_GENRE_ATOM ||
            type == ASSET_INFO_RATING_ATOM ||
            type == ASSET_INFO_CLSF_ATOM ||
            type == ASSET_INFO_KEYWORD_ATOM ||
            type == ASSET_INFO_LOCATION_ATOM ||
            type == ASSET_INFO_ALBUM_ATOM ||
            type == ASSET_INFO_YRRC_ATOM ||
            type == TEXT_SAMPLE_ENTRY ||
            type == AVC_SAMPLE_ENTRY ||
            type == AVC_CONFIGURATION_BOX ||
            type == MPEG4_BITRATE_BOX ||
            type == MPEG4_EXTENSION_DESCRIPTORS_BOX ||
            type == ENCRYPTED_AUDIO_SAMPLE_ENTRY ||
            type == ENCRYPTED_VIDEO_SAMPLE_ENTRY ||
            type == PROTECTION_SCHEME_INFO_BOX ||
            type == ORIGINAL_FORMAT_BOX ||
            type == SCHEME_TYPE_BOX ||
            type == SCHEME_INFORMATION_BOX ||
            type == MUTABLE_DRM_INFORMATION ||
            type == OMADRM_TRANSACTION_TRACKING_BOX ||
            type == OMADRM_RIGHTS_OBJECT_BOX ||
            type == OMADRM_KMS_BOX ||


            type == MOVIE_EXTENDS_ATOM ||
            type == MOVIE_EXTENDS_HEADER_ATOM ||
            type == TRACK_EXTENDS_ATOM ||
            type == MOVIE_FRAGMENT_ATOM ||
            type == MOVIE_FRAGMENT_HEADER_ATOM ||
            type == TRACK_FRAGMENT_ATOM ||
            type == TRACK_FRAGMENT_HEADER_ATOM ||
            type == TRACK_FRAGMENT_RUN_ATOM ||
            type == MOVIE_FRAGMENT_RANDOM_ACCESS_ATOM ||
            type == MOVIE_FRAGMENT_RANDOM_ACCESS_OFFSET_ATOM ||
            type == TRACK_FRAGMENT_RANDOM_ACCESS_ATOM ||

            type == META_DATA_ATOM ||
            type == ITUNES_ILST_ATOM ||
            type == ITUNES_ILST_DATA_ATOM ||
            type == ITUNES_SONG_TITLE_ATOM ||
            type == ITUNES_TRACK_SUBTITLE_ATOM ||
            type == ITUNES_COMPILATION_ATOM ||
            type == ITUNES_ENCODER_TOOL_ATOM ||
            type == ITUNES_ENCODEDBY_ATOM ||
            type == ITUNES_FREE_FORM_ATOM ||
            type == ITUNES_ALBUM_ARTIST_ATOM ||
            type == ITUNES_COMPOSER_ATOM ||
            type == ITUNES_ALBUM_ATOM ||
            type == ITUNES_GROUPING1_ATOM ||
            type == ITUNES_GROUPING2_ATOM ||
            type == ITUNES_GENRE1_ATOM ||
            type == ITUNES_GENRE2_ATOM ||
            type == ITUNES_TRACK_NUMBER_ATOM ||
            type == ITUNES_DISK_NUMBER_ATOM ||
            type == ITUNES_YEAR_ATOM ||
            type == ITUNES_COMMENT_ATOM ||
            type == ITUNES_LYRICS_ATOM  ||
            type == ITUNES_ART_WORK_ATOM ||
            type == ITUNES_BPM_ATOM ||
            type == ITUNES_MEAN_ATOM ||
            type == ITUNES_FREE_FORM_DATA_NAME_ATOM ||
            type == ITUNES_COPYRIGHT_ATOM ||
            type == ITUNES_DESCRIPTION_ATOM	||
            type == ITUNES_CONTENT_RATING_ATOM ||
            type == ITUNES_ARTIST1_ATOM ||
            type == ITUNES_ARTIST2_ATOM ||



            type == PIXELASPECTRATIO_BOX)
    {
        return;
    }
    else
    {
        type = UNKNOWN_ATOM;
    }
    return;
}

OSCL_EXPORT_REF uint32
AtomUtils::getMediaTypeFromHandlerType(uint32 handlerType)
{
    if (
        handlerType == MEDIA_TYPE_AUDIO ||
        handlerType == MEDIA_TYPE_VISUAL ||
        handlerType == MEDIA_TYPE_HINT ||
        handlerType == MEDIA_TYPE_OBJECT_DESCRIPTOR ||
        handlerType == MEDIA_TYPE_CLOCK_REFERENCE ||
        handlerType == MEDIA_TYPE_SCENE_DESCRIPTION ||
        handlerType == MEDIA_TYPE_MPEG7 ||
        handlerType == MEDIA_TYPE_OBJECT_CONTENT_INFO ||
        handlerType == MEDIA_TYPE_IPMP ||
        handlerType == MEDIA_TYPE_MPEG_J ||
        handlerType == MEDIA_TYPE_TEXT
    )
    {
        return handlerType;
    }
    else
    {
        return MEDIA_TYPE_UNKNOWN;
    }
}

OSCL_EXPORT_REF uint32
AtomUtils::getNumberOfBytesUsedToStoreContent(uint32 sizeOfClass)
{
    // The content in a descriptor class is stored immediately after the descriptor tag
    if (sizeOfClass <= 0x7f) return sizeOfClass - 2; // _sizeOfClass field is 1 byte (7 LS bits)
    else if (sizeOfClass <= 0x3fff) return sizeOfClass - 3; // _sizeOfClass is 2 bytes (7 LS bits each)
    else if (sizeOfClass <= 0x1fffff) return sizeOfClass - 4; // _sizeOfClass is 3 bytes (7 LS bits each)
    else if (sizeOfClass <= 0x0fffffff) return sizeOfClass - 5; // _sizeOfClass is 4 bytes (7 LS bits each)
    else return 0; // ERROR condition
}

OSCL_EXPORT_REF int32 AtomUtils::getNextAtomSize(MP4_FF_FILE *fp)
{
    uint32 size;
    AtomUtils::read32(fp, size);

    AtomUtils::rewindFilePointerByN(fp, 4);
    return size;
}

// Peeks and returns the next Nth tag (32 bits) from the file
OSCL_EXPORT_REF uint32 AtomUtils::peekNextNthBytes(MP4_FF_FILE *fp, int32 n)
{
    uint32 tag = 0;
    for (int32 i = 0; i < n; i++)
    {
        AtomUtils::read32(fp, tag);
    }
    AtomUtils::rewindFilePointerByN(fp, (4*n));
    return tag;
}

// Peeks and returns the next Nth bytes (8 bits) from the file
OSCL_EXPORT_REF uint8 AtomUtils::peekNextByte(MP4_FF_FILE *fp)
{
    uint8 tag = 0;
    AtomUtils::read8(fp, tag);
    AtomUtils::rewindFilePointerByN(fp, 1);
    return tag;
}

OSCL_EXPORT_REF void AtomUtils::seekFromCurrPos(MP4_FF_FILE *fp, uint32 n)
{
    fp->_pvfile.Seek(n, Oscl_File::SEEKCUR);
}

OSCL_EXPORT_REF void AtomUtils::seekFromStart(MP4_FF_FILE *fp, uint32 n)
{
    fp->_pvfile.Seek(n, Oscl_File::SEEKSET);
}

OSCL_EXPORT_REF void AtomUtils::seekToEnd(MP4_FF_FILE *fp)
{
    fp->_pvfile.Seek(0, Oscl_File::SEEKEND);
}

OSCL_EXPORT_REF void AtomUtils::rewindFilePointerByN(MP4_FF_FILE *fp, uint32 n)
{
    fp->_pvfile.Seek((-1 * (int32) n), Oscl_File::SEEKCUR);
}

OSCL_EXPORT_REF int32 AtomUtils::getCurrentFilePosition(MP4_FF_FILE *fp)
{
    return (fp->_pvfile.Tell());
}


OSCL_EXPORT_REF int32 AtomUtils::OpenMP4File(OSCL_wString& filename,
        uint32 mode,
        MP4_FF_FILE *fp)
{
    OSCL_UNUSED_ARG(mode);
    if (fp != NULL)
    {
        return (fp->_pvfile.Open(filename.get_cstr(),
                                 Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,
                                 *(fp->_fileServSession)));
    }
    return -1;
}

OSCL_EXPORT_REF int32  AtomUtils::CloseMP4File(MP4_FF_FILE *fp)
{
    if (fp != NULL)
    {
        return (fp->_pvfile.Close());
    }
    return -1;
}

OSCL_EXPORT_REF int32  AtomUtils::Flush(MP4_FF_FILE *fp)
{
    if (fp != NULL)
    {
        return (fp->_pvfile.Flush());
    }
    return -1;
}

OSCL_EXPORT_REF bool AtomUtils::getCurrentFileSize(MP4_FF_FILE *fp, uint32& aCurrentSize)
{
    if (fp != NULL)
    {
        aCurrentSize = 0;
        uint32 aRemBytes = 0;
        if (fp->_pvfile.GetRemainingBytes(aRemBytes))
        {
            uint32 currPos = (uint32)(fp->_pvfile.Tell());
            aCurrentSize = currPos + aRemBytes;
            fp->_fileSize = aCurrentSize;
            return true;
        }
    }
    return false;
}

OSCL_EXPORT_REF bool
AtomUtils::read32(uint8 *&buf, uint32 &data)
{
    const int32 N = 4;
    data = 0;

    for (int32 i = 0;i < N;i++)
        data = (data << 8) | buf[i];

    buf += N;
    return true;
}

// Read in the 32 bits byte by byte and take most significant byte first.
// This is equivalent to two read32 calls.
OSCL_EXPORT_REF bool
AtomUtils::read32read32(uint8 *&buf, uint32 &data1, uint32 &data2)
{
    const int32 N = 8;
    data1 = 0;
    data2 = 0;

    int32 i;
    for (i = 0;i < 4;i++)
        data1 = (data1 << 8) | buf[i];

    for (i = 4;i < 8;i++)
        data2 = (data2 << 8) | buf[i];

    buf += N;
    return true;
}

// Read in the 16 bits byte by byte and take most significant byte first
OSCL_EXPORT_REF bool
AtomUtils::read16(uint8 *&buf, uint16 &data)
{
    const int32 N = 2;
    data = 0;

    for (int32 i = 0;i < N;i++)
        data = (uint16)((data << 8) | (uint16) buf[i]);

    buf += N;
    return true;
}

// Read in the 8 bit byte
OSCL_EXPORT_REF bool
AtomUtils::read8(uint8 *&buf, uint8 &data)
{
    data = 0;
    data = *buf;
    buf++;
    return true;
}

// Read in byte data and take most significant byte first
OSCL_EXPORT_REF bool
AtomUtils::readByteData(uint8 *&buf, uint32 length, uint8 *data)
{
    oscl_memcpy(data, buf, length);

    buf += length;
    return true;
}

OSCL_EXPORT_REF uint32
AtomUtils::getNextAtomType(uint8 *buf)
{
    uint32 size;
    uint32 type;
    if (!AtomUtils::read32read32(buf, size, type))
    {
        return UNKNOWN_ATOM;
    }

    // Rewinding the stream back to atom start
    buf -= 8;

    if (type == MOVIE_ATOM ||
            type == MOVIE_HEADER_ATOM ||
            type == TRACK_ATOM ||
            type == TRACK_HEADER_ATOM ||
            type == TRACK_REFERENCE_ATOM ||
            type == MEDIA_ATOM ||
            type == EDIT_ATOM ||
            type == EDIT_LIST_ATOM ||
            type == MEDIA_HEADER_ATOM ||
            type == HANDLER_ATOM ||
            type == MEDIA_INFORMATION_ATOM ||
            type == VIDEO_MEDIA_HEADER_ATOM ||
            type == SOUND_MEDIA_HEADER_ATOM ||
            type == HINT_MEDIA_HEADER_ATOM ||
            type == MPEG4_MEDIA_HEADER_ATOM ||
            type == NULL_MEDIA_HEADER_ATOM ||
            type == DATA_INFORMATION_ATOM ||
            type == DATA_REFERENCE_ATOM ||
            type == DATA_ENTRY_URL_ATOM ||
            type == DATA_ENTRY_URN_ATOM ||
            type == SAMPLE_TABLE_ATOM ||
            type == TIME_TO_SAMPLE_ATOM ||
            type == COMPOSITION_OFFSET_ATOM ||
            type == SAMPLE_DESCRIPTION_ATOM ||
            type == ESD_ATOM ||
            type == SAMPLE_SIZE_ATOM ||
            type == SAMPLE_TO_CHUNK_ATOM ||
            type == CHUNK_OFFSET_ATOM ||
            type == SYNC_SAMPLE_ATOM ||
            type == SHADOW_SYNC_SAMPLE_ATOM ||
            type == DEGRADATION_PRIORITY_ATOM ||
            type == OBJECT_DESCRIPTOR_ATOM ||
            type == MEDIA_DATA_ATOM ||
            type == FREE_SPACE_ATOM ||
            type == SKIP_ATOM ||
            type == USER_DATA_ATOM ||
            type == FILE_TYPE_ATOM ||
            type == PVUSER_DATA_ATOM ||
            type == PVUSER_DATA_ATOM ||
            type == AMR_SAMPLE_ENTRY_ATOM ||
            type == AMRWB_SAMPLE_ENTRY_ATOM ||
            type == H263_SAMPLE_ENTRY_ATOM ||
            type == AUDIO_SAMPLE_ENTRY ||
            type == VIDEO_SAMPLE_ENTRY ||
            type == MPEG_SAMPLE_ENTRY ||
            type == UUID_ATOM ||
            type == AMR_SPECIFIC_ATOM ||
            type == H263_SPECIFIC_ATOM ||
            type == COPYRIGHT_ATOM ||
            type == TEXT_SAMPLE_ENTRY ||
            type == FONT_TABLE_ATOM ||
            type == TEXT_STYLE_BOX ||
            type == TEXT_HIGHLIGHT_BOX ||
            type == TEXT_HILIGHT_COLOR_BOX ||
            type == TEXT_KARAOKE_BOX ||
            type == TEXT_SCROLL_DELAY_BOX ||
            type == TEXT_HYPER_TEXT_BOX ||
            type == TEXT_OVER_RIDE_BOX ||
            type == TEXT_BLINK_BOX ||
            type == HINT_TRACK_REFERENCE_TYPE ||
            type == DPND_TRACK_REFERENCE_TYPE ||
            type == IPIR_TRACK_REFERENCE_TYPE ||
            type == MPOD_TRACK_REFERENCE_TYPE ||
            type == SYNC_TRACK_REFERENCE_TYPE ||
            type == ASSET_INFO_TITLE_ATOM ||
            type == ASSET_INFO_DESCP_ATOM ||
            type == ASSET_INFO_PERF_ATOM ||
            type == ASSET_INFO_AUTHOR_ATOM ||
            type == ASSET_INFO_GENRE_ATOM ||
            type == ASSET_INFO_RATING_ATOM ||
            type == ASSET_INFO_CLSF_ATOM ||
            type == ASSET_INFO_KEYWORD_ATOM ||
            type == ASSET_INFO_LOCATION_ATOM ||
            type == ASSET_INFO_ALBUM_ATOM ||
            type == ASSET_INFO_YRRC_ATOM)
    {
        return type;
    }
    else
    {
        return UNKNOWN_ATOM; // ERROR condition
    }
}

OSCL_EXPORT_REF int32 AtomUtils::getNextAtomSize(uint8 *buf)
{
    uint32 size;
    AtomUtils::read32(buf, size);

    return size;
}


OSCL_EXPORT_REF uint32 AtomUtils::getContentLength(MP4_FF_FILE *fp)
{
    // this returns the content length if known
    return fp->_pvfile.GetContentLength();
}


OSCL_EXPORT_REF uint32 AtomUtils::getFileBufferingCapacity(MP4_FF_FILE *fp)
{
    // this returns the data stream cache size
    return fp->_pvfile.GetFileBufferingCapacity();
}


OSCL_EXPORT_REF void AtomUtils::skipFromStart(MP4_FF_FILE *fp, uint32 n)
{
    fp->_pvfile.Skip(n, Oscl_File::SEEKSET);
}


OSCL_EXPORT_REF void AtomUtils::getCurrentByteRange(MP4_FF_FILE *fp, uint32& aCurrentFirstByteOffset, uint32& aCurrentLastByteOffset)
{
    // this returns the byte range in the data stream cache
    // first and last offset inclusive
    fp->_pvfile.GetCurrentByteRange(aCurrentFirstByteOffset, aCurrentLastByteOffset);
}



