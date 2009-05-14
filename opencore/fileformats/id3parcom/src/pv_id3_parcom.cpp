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
* @file pv_id3_parcom_types.cpp
* @brief Type definitions for ID3 Parser-Composer
*/

#ifndef PV_ID3_PARCOM_H_INCLUDED
#include "pv_id3_parcom.h"
#endif
#ifndef PV_ID3_PARCOM_CONSTANTS_H_INCLUDED
#include "pv_id3_parcom_constants.h"
#endif
#ifndef OSCL_SNPRINTF_H_INCLUDED
#include "oscl_snprintf.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMI_KVP_UTIL_H_INCLUDED
#include "pvmi_kvp_util.h"
#endif
#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif
#ifndef WCHAR_SIZE_UTILS_H_INCLUDED
#include "wchar_size_utils.h"
#endif

#define LOG_STACK_TRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, m);
#define LOG_DEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, m);
#define LOG_ERR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);

#ifndef NULL_TERM_CHAR
#define NULL_TERM_CHAR '\0'
#endif

// DLL entry point
OSCL_DLL_ENTRY_POINT_DEFAULT()

////////////////////////////////////////////////////////////////////////////
class PVID3ParComKvpCleanupDA : public OsclDestructDealloc
{
    public:
        PVID3ParComKvpCleanupDA(Oscl_DefAlloc* in_gen_alloc) :
                gen_alloc(in_gen_alloc) {};
        virtual ~PVID3ParComKvpCleanupDA() {};

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            // no need to call destructors in this case just dealloc
            gen_alloc->deallocate(ptr);
        }

    private:
        Oscl_DefAlloc* gen_alloc;
};

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVID3ParCom::PVID3ParCom()
        : iInputFile(NULL),
        iTitleFoundFlag(false),
        iArtistFoundFlag(false),
        iAlbumFoundFlag(false),
        iYearFoundFlag(false),
        iCommentFoundFlag(false),
        iTrackNumberFoundFlag(false),
        iGenereFoundFlag(false),
        iFileSizeInBytes(0),
        iByteOffsetToStartOfAudioFrames(0),
        iID3V1Present(false),
        iID3V2Present(false),
        iVersion(PV_ID3_INVALID_VERSION),
        iUseMaxTagSize(false),
        iMaxTagSize(0),
        iUsePadding(false),
        iTagAtBof(false),
        iSeekFrameFound(false)
{
    iLogger = PVLogger::GetLoggerObject("PVID3ParCom");
    iID3TagInfo.iID3V2ExtendedHeaderSize = 0;
    iID3TagInfo.iID3V2FrameSize = 0;
    iID3TagInfo.iID3V2TagFlagsV2 = 0;
    iID3TagInfo.iID3V2TagSize = 0 ;
    iID3TagInfo.iFooterPresent = false;

    oscl_memset(&iID3TagInfo.iID3V2FrameFlag, 0, sizeof(iID3TagInfo.iID3V2FrameFlag));

}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVID3ParCom::~PVID3ParCom()
{
    iFrames.clear();
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 PVID3ParCom::GetID3TagSize(PVFile* aFile)
{
    if (!aFile)
    {
        return 0;
    }

    iInputFile = aFile;

    if (iInputFile->Seek(0, Oscl_File::SEEKSET) == -1)
    {
        iFileSizeInBytes = 0;
    }
    else
    {
        iInputFile->GetRemainingBytes((uint32&)iFileSizeInBytes);
        if (iInputFile->Seek(0, Oscl_File::SEEKSET) == -1)
        {
            return 0;
        }
    }

    uint32 buffsize = 100; //number of bytes to search , from the end of file, to look for footer
    if (CheckForTagID3V2()
            || (LookForV2_4Footer(buffsize, 0) == PVMFSuccess))
    {
        if (iInputFile->Seek(2, Oscl_File::SEEKCUR) == -1)
        {
            return 0;
        }

        // Read and convert tag flags
        uint8 flags = 0;
        if (read8(iInputFile, flags) == false)
        {
            return 0;
        }

        // Read and convert tag size
        uint32 synchIntTagSize;
        if (read32(iInputFile, synchIntTagSize) == false)
        {
            return 0;
        }

        // tagSize will store the file's Id3v2 tag size
        uint32 tagSize = SafeSynchIntToInt32(synchIntTagSize) + ID3V2_TAG_NUM_BYTES_HEADER;

        //check if footer is present
        if ((iVersion == PV_ID3_V2_4) && (flags & FTR_FLAGMASK))
            tagSize += ID3V2_TAG_NUM_BYTES_HEADER; // add 10 bytes for footer

        return tagSize;
    }
    else
    {
        if (CheckForTagID3V1())
        {
            return ID3V1_MAX_NUM_BYTES_TOTAL;
        }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 PVID3ParCom::GetID3TagSize()
{
    uint32 composedTagSize = 0;
    switch (iVersion)
    {
        case PV_ID3_V2_2:
        case PV_ID3_V2_3:
        case PV_ID3_V2_4:
        {
            composedTagSize = ID3V2_TAG_NUM_BYTES_HEADER + iID3TagInfo.iID3V2TagSize;

            if (iID3TagInfo.iFooterPresent)
                composedTagSize += ID3V2_TAG_NUM_BYTES_HEADER; //footer is replica of Tag Header
        }
        break;

        case PV_ID3_V1:
        case PV_ID3_V1_1:
            composedTagSize =  128; // ID3v1 tag has fix size
            break;
        default:
            composedTagSize = 0;

    }

    return composedTagSize;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVID3ParCom::SetMaxID3TagSize(bool aUseMaxSize, uint32 aMaxSize, bool aUsePadding)
{
    switch (iVersion)
    {
        case PV_ID3_V1:
        case PV_ID3_V1_1:
        case PV_ID3_INVALID_VERSION:
        {
            LOG_ERR((0, "PVID3Parcom::SetMaxID3TagSize: Error - Fix size ID3 tag"));
            return PVMFErrNotSupported;
        }
        case PV_ID3_V2_2:
            // Minimum for an ID3v2_2 tag is tag header plus 1 frame.  The minimum data length of
            // a frame is 1 byte
        {
            if (aUseMaxSize && aMaxSize < (ID3V2_TAG_NUM_BYTES_HEADER + ID3V2_2_FRAME_NUM_BYTES_HEADER + 1))
            {
                LOG_ERR((0, "PVID3ParCom::SetMaxID3TagSize: Error - Minimum tag size is 17 bytes"));
                return PVMFErrArgument;
            }
        }
        break;
        case PV_ID3_V2_3:
        case PV_ID3_V2_4:
            // Minimum for an ID3v2 tag is tag header plus 1 frame.  The minimum data length of
            // a frame is 1 byte
            if (aUseMaxSize && aMaxSize < (ID3V2_TAG_NUM_BYTES_HEADER + ID3V2_FRAME_NUM_BYTES_HEADER + 1))
            {
                LOG_ERR((0, "PVID3ParCom::SetMaxID3TagSize: Error - Minimum tag size is 21 bytes"));
                return PVMFErrArgument;
            }
    }

    iUseMaxTagSize = aUseMaxSize;
    if (iUseMaxTagSize)
    {
        iMaxTagSize = aMaxSize;
        iUsePadding = aUsePadding;
    }
    else
    {
        iMaxTagSize = 0;
        iUsePadding = false;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVID3ParCom::ParseID3Tag(PVFile* aFile, uint32 buffsize)
{
    if (!aFile)
    {
        return PVMFFailure;
    }

    iInputFile = aFile;

    int32 currentFilePosn = 0;

    // SEEK TO THE END OF THE FILE AND GET FILE SIZE
    currentFilePosn = iInputFile->Tell();
    if (currentFilePosn == -1)
    {
        return PVMFFailure;
    }

    if (iInputFile->Seek(0, Oscl_File::SEEKSET) == -1)
    {
        iFileSizeInBytes = 0;
    }
    else
    {
        iInputFile->GetRemainingBytes((uint32&)iFileSizeInBytes);
        if (iInputFile->Seek(currentFilePosn, Oscl_File::SEEKSET) == -1)
        {
            iInputFile = NULL;
            return PVMFFailure;
        }
    }

    bool readTags = false;
    //check for ID3 Version 2
    if (CheckForTagID3V2() || (LookForV2_4Footer(buffsize, 0) == PVMFSuccess))
    {
        //Read the header
        readTags = ReadHeaderID3V2();

        //check for ID3 Version 1
        if (CheckForTagID3V1())
        {
            iVersion = PV_ID3_V1;
            ReadID3V1Tag();
        }
        if (iInputFile->Seek(currentFilePosn, Oscl_File::SEEKSET) == -1)
        {
            iInputFile = NULL;
            return PVMFFailure;
        }
    }
    else if (!readTags && CheckForTagID3V1())
    {
        //check for ID3 Version 1
        iVersion = PV_ID3_V1;
        //Read the header
        ReadID3V1Tag();

        iByteOffsetToStartOfAudioFrames = 0;
        if (iInputFile->Seek(currentFilePosn, Oscl_File::SEEKSET) == -1)
        {
            iInputFile = NULL;
            return PVMFFailure;
        }
    }
    else
    {
        return PVMFFailure;
    }

    iInputFile = NULL;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVID3Version PVID3ParCom::GetID3Version() const
{
    return iVersion;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVID3ParCom::IsID3V1Present() const
{
    return iID3V1Present;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVID3ParCom::IsID3V2Present() const
{
    return iID3V2Present;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVID3ParCom::GetID3Frames(PvmiKvpSharedPtrVector& aFrames)
{
    aFrames = iFrames;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVID3ParCom::GetID3Frame(const OSCL_String& aFrameType, PvmiKvpSharedPtrVector& aFrameVector)
{
    uint32 i;
    for (i = 0; i < iFrames.size(); i++)
    {
        if (pv_mime_strcmp(iFrames[i]->key, aFrameType.get_str()) == 0)
        {
            return PushFrameToFrameVector(iFrames[i], aFrameVector);
        }
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVID3ParCom::IsID3FrameAvailable(const OSCL_String& aFrameType)
{
    uint32 i;
    for (i = 0; i < iFrames.size(); i++)
    {
        if (pv_mime_strcmp(iFrames[i]->key, aFrameType.get_str()) == 0)
        {
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVID3ParCom::SetID3Version(PVID3Version aVersion)
{
    iVersion = aVersion;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVID3ParCom::SetID3Frame(const PvmiKvp& aFrame)
{
    uint32 i;
    OSCL_StackString<4> frameID;
    PVID3FrameType newFrameType;
    PVID3FrameType existingFrameType;
    PVMFStatus status = PVMFSuccess;

    // Find an existing entry in iFrames with matching frame type
    status = GetFrameTypeFromKvp(aFrame, frameID, newFrameType);
    for (i = 0; status == PVMFSuccess && i < iFrames.size(); i++)
    {
        status = GetFrameTypeFromKvp(iFrames[i], frameID, existingFrameType);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVID3ParCom::SetID3Frame: Error - GetFrameTypeFromKvp failed"));
            break;
        }
        if (existingFrameType == newFrameType)
        {
            iFrames.erase(iFrames.begin() + i);
            break;
        }

    }

    if (status != PVMFSuccess)
    {
        return status;
    }

    PvmiKvpValueType kvpValueType = GetValTypeFromKeyString(aFrame.key);
    OSCL_StackString<128> key = _STRLIT_CHAR(aFrame.key);
    uint32 valueSize = 0;
    uint32 valueStrLen = 0;
    switch (kvpValueType)
    {
        case PVMI_KVPVALTYPE_CHARPTR:
            valueStrLen = oscl_strlen(aFrame.value.pChar_value);
            valueSize = valueStrLen + 1;
            break;
        case PVMI_KVPVALTYPE_WCHARPTR:
            valueStrLen = oscl_strlen(aFrame.value.pWChar_value);
            valueSize = (valueStrLen + 2) * sizeof(oscl_wchar);
            break;
        case PVMI_KVPVALTYPE_KSV:
            // Comment field
            break;
        case PVMI_KVPVALTYPE_UINT32:
            valueSize = 0;
            break;
        default:
            return PVMFErrNotSupported;
    }

    status = PVMFSuccess;
    PvmiKvpSharedPtr kvp;
    bool truncate = false;
    kvp = HandleErrorForKVPAllocation(key, kvpValueType, valueSize, truncate, status);

    if (truncate || (PVMFSuccess != status) || !kvp)
    {
        return PVMFErrNoMemory;
    }

    switch (kvpValueType)
    {
        case PVMI_KVPVALTYPE_CHARPTR:
            oscl_strncpy(kvp->value.pChar_value, aFrame.value.pChar_value, valueStrLen);
            kvp->value.pChar_value[valueStrLen] = NULL_TERM_CHAR;
            break;
        case PVMI_KVPVALTYPE_WCHARPTR:
            oscl_strncpy(kvp->value.pWChar_value, aFrame.value.pWChar_value, valueStrLen);
            kvp->value.pWChar_value[valueStrLen] = NULL_TERM_CHAR;
            break;
        case PVMI_KVPVALTYPE_UINT32:
            kvp->value.uint32_value = aFrame.value.uint32_value;
            break;
        case PVMI_KVPVALTYPE_KSV:
            // comment field
        default:
            return PVMFErrNotSupported;
    }


    if (PVMFSuccess != PushFrameToFrameVector(kvp, iFrames))
    {
        return PVMFErrNoMemory;
    }
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVID3ParCom::RemoveID3Frame(const OSCL_String& aFrameType)
{
    uint32 i;
    for (i = 0; i < iFrames.size(); i++)
    {
        if (pv_mime_strcmp(iFrames[i]->key, aFrameType.get_str()) == 0)
        {
            iFrames.erase(iFrames.begin() + i);
            return PVMFSuccess;
        }
    }

    // Frame of specified type is not found
    return PVMFErrArgument;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVID3ParCom::ComposeID3Tag(OsclRefCounterMemFrag& aTag)
{
    switch (iVersion)
    {
        case PV_ID3_V2_2:
        case PV_ID3_V2_3:
        case PV_ID3_V2_4:
            return ComposeID3v2Tag(aTag);
        default:
            return PVMFErrNotSupported;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVID3ParCom::Reset()
{
    iFrames.clear();
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
bool PVID3ParCom::CheckForTagID3V1()
{
    uint8 tagHeader[ID3V1_TAG_NUM_BYTES_HEADER+1] = {0};

    // Make sure file is big enough to contain a tag
    if (iFileSizeInBytes >= ID3V1_MAX_NUM_BYTES_TOTAL)
    {
        uint32 nBytes = 0;
        // Read the value at the tag position

        nBytes = iFileSizeInBytes - ID3V1_MAX_NUM_BYTES_TOTAL;
        if (iInputFile->Seek(nBytes, Oscl_File::SEEKSET) == -1)
        {
            return false;
        }


        if (!readByteData(iInputFile, ID3V1_TAG_NUM_BYTES_HEADER, tagHeader))
        {
            return false;
        }

        // Read in ID3 Tags at the front of the file.
        if (oscl_memcmp(ID3_V1_IDENTIFIER, tagHeader, ID3V1_TAG_NUM_BYTES_HEADER) == 0)
        {
            iID3V1Present = true;
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////
bool PVID3ParCom::CheckForTagID3V2()
{
    uint8 id3Header[ID3V2_TAG_NUM_BYTES_HEADER+1] = {0};

    if (!readByteData(iInputFile, ID3V2_TAG_NUM_BYTES_ID, id3Header))
    {
        return false;
    }

    // Read in ID3 Tags at the front of the file.
    if (oscl_memcmp(ID3_V2_IDENTIFIER, id3Header, ID3V2_TAG_NUM_BYTES_ID) == 0)
    {
        iTagAtBof = true;
        iID3V2Present = true;
        return true;
    }
    return false;
}

OSCL_EXPORT_REF bool PVID3ParCom::IsID3V2Present(PVFile* aFile, uint32& aTagSize)
{
    iInputFile = aFile;
    if (iID3V2Present)
    {
        // if id3 tag has already been discovered, just return the tag size
        aTagSize = iByteOffsetToStartOfAudioFrames;
        return true;
    }

    if (CheckForTagID3V2())
    {
        // we dont want to parse the id3 frames, just read the id3 header

        ReadHeaderID3V2(false);
        aTagSize = iByteOffsetToStartOfAudioFrames;
        return true;
    }
    return false;
}

OSCL_EXPORT_REF PVMFStatus PVID3ParCom::LookForV2_4Footer(uint32 aBuffSz, uint32 aFileOffset)
{
    if (iTagAtBof)	//already found tag. no need to search from the end.
        return PVMFFailure;

    uint32 footer_location = SearchTagV2_4(aBuffSz, aFileOffset);

    if (footer_location == 0)
        return PVMFFailure;

    if (iInputFile->Seek(footer_location  + ID3V2_TAG_NUM_BYTES_ID + ID3V2_TAG_NUM_BYTES_VERSION + ID3V2_TAG_NUM_BYTES_FLAG,
                         Oscl_File::SEEKSET) == -1)
    {
        return PVMFFailure;
    }

    uint32 tag_size = 0, size = 0;

    if (read32(iInputFile, size) == false)
    {
        return PVMFFailure;
    }

    tag_size = SafeSynchIntToInt32(size);

    if (iInputFile->Seek(-(int32)(tag_size + ID3V2_TAG_NUM_BYTES_HEADER + (ID3V2_TAG_NUM_BYTES_HEADER - ID3V2_TAG_NUM_BYTES_ID)),
                         Oscl_File::SEEKCUR) == -1)
    {
        return PVMFFailure;
    }

    return PVMFSuccess;
}
////////////////////////////////////////////////////////////////////////////
// (if an error is detected, this returns 0)

uint32 PVID3ParCom::SearchTagV2_4(uint32 aBuffSz, uint32 aFileOffset)
{

    if (iFileSizeInBytes == 0)
        return 0;

    uint8 *buff = (uint8 *)oscl_malloc(aBuffSz);
    if (! buff)		// malloc might fail
        return 0;

    int remaining_bytes = 0;

    int num_bytes_read = aBuffSz + aFileOffset;

    //in case we have almost reached to the begining of file,
    //and remaining bytes are less than the buffer size,
    //look for ID3 tag in the remaining bytes only.

    if (num_bytes_read > iFileSizeInBytes)
        remaining_bytes = iFileSizeInBytes - (num_bytes_read - aBuffSz);

    if (remaining_bytes > 0)
    {

        if (iInputFile->Seek(0 , Oscl_File::SEEKSET) == -1)
        {
            oscl_free(buff);
            return 0;
        }

        if (readByteData(iInputFile, remaining_bytes, buff) == false)
        {
            oscl_free(buff);
            return 0;
        }

        for (int i = 0; i < remaining_bytes;i++)
        {
            if (oscl_memcmp(buff + i, ID3_V2_4_TAG_FOOTER, ID3V2_TAG_NUM_BYTES_ID) == 0)
            {
                oscl_free(buff);
                return i;
            }

        }

        oscl_free(buff);
        return 0;
    }
    else
    {

        if (iInputFile->Seek((iFileSizeInBytes - (aFileOffset + aBuffSz)), Oscl_File::SEEKSET) == -1)
        {
            oscl_free(buff);
            return 0;
        }

        if (readByteData(iInputFile, aBuffSz, buff) == false)
        {
            oscl_free(buff);
            return 0;
        }

        for (uint i = 0; i < aBuffSz;i++)
        {
            if (oscl_memcmp(buff + i, ID3_V2_4_TAG_FOOTER, ID3V2_TAG_NUM_BYTES_ID) == 0)
            {
                oscl_free(buff);
                return ((iFileSizeInBytes - ((aFileOffset + aBuffSz) - i)));
            }

        }

    }

    oscl_free(buff);
    return 0;
}

////////////////////////////////////////////////////////////////////////////
void PVID3ParCom::ReadID3V1Tag(void)
{

    PVMFStatus status = PVMFSuccess;
    bool truncate = false;

    if (!iTitleFoundFlag)
    {
        //Title
        status = ReadStringValueFrame(PV_ID3_FRAME_TITLE, PV_ID3_CHARSET_ISO88591, ID3V1_MAX_NUM_BYTES_TITLE);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error = ReadStringValueFrame failed for title"));
            OSCL_LEAVE(OsclErrGeneral);
        }
        iTitleFoundFlag = true;
    }
    else
    {

        if (iInputFile->Seek(ID3V1_MAX_NUM_BYTES_TITLE, Oscl_File::SEEKCUR) == -1)
        {
            return;
        }

    }
    if (!iArtistFoundFlag)
    {
        //Artist
        status = ReadStringValueFrame(PV_ID3_FRAME_ARTIST, PV_ID3_CHARSET_ISO88591, ID3V1_MAX_NUM_BYTES_ARTIST);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error = ReadStringValueFrame failed for artist"));
            OSCL_LEAVE(OsclErrGeneral);
        }
        iArtistFoundFlag = true;
    }
    else
    {
        if (iInputFile->Seek(ID3V1_MAX_NUM_BYTES_ARTIST, Oscl_File::SEEKCUR) == -1)
        {
            return;
        }
    }

    if (!iAlbumFoundFlag)
    {
        // Album
        status = ReadStringValueFrame(PV_ID3_FRAME_ALBUM, PV_ID3_CHARSET_ISO88591, ID3V1_MAX_NUM_BYTES_ALBUM);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error = ReadStringValueFrame failed for album"));
            OSCL_LEAVE(OsclErrGeneral);
        }
        iAlbumFoundFlag = true;
    }
    else
    {
        if (iInputFile->Seek(ID3V1_MAX_NUM_BYTES_ALBUM, Oscl_File::SEEKCUR) == -1)
        {
            return;
        }
    }

    if (!iYearFoundFlag)
    {
        //Year
        status = ReadStringValueFrame(PV_ID3_FRAME_YEAR, PV_ID3_CHARSET_ISO88591, ID3V1_MAX_NUM_BYTES_YEAR);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error = ReadStringValueFrame failed for year"));
            OSCL_LEAVE(OsclErrGeneral);
        }
        iYearFoundFlag = true;
    }
    else
    {
        if (iInputFile->Seek(ID3V1_MAX_NUM_BYTES_YEAR, Oscl_File::SEEKCUR) == -1)
        {
            return;
        }
    }
    OSCL_StackString<128> keyStr;
    PvmiKvpSharedPtr kvpPtr;
    uint8* frameData = NULL;

    if (!iCommentFoundFlag   || !iTrackNumberFoundFlag)
    {
        frameData = NULL;
        //Comment or Track Number
        // Read and convert comment & track number
        uint32 frameDataSize = ID3V1_MAX_NUM_BYTES_FIELD_SIZE + 1;
        int32 err = OsclErrNone;
        frameData = (uint8*) AllocateValueArray(err, PVMI_KVPVALTYPE_UINT8PTR, frameDataSize, &iAlloc);
        if (OsclErrNone != err || !frameData)
        {
            LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error - Out of memory"));
            OSCL_LEAVE(OsclErrNoMemory);
        }
        oscl_memset(frameData, 0, frameDataSize);

        if (readByteData(iInputFile, ID3V1_MAX_NUM_BYTES_COMMENT, frameData) == false)
        {
            iAlloc.deallocate(frameData);
            LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error - readByteData failed"));
            OSCL_LEAVE(OsclErrGeneral);
        }


        if (frameData[ID3V1_MAX_NUM_BYTES_COMMENT-2] == 0 &&
                frameData[ID3V1_MAX_NUM_BYTES_COMMENT-1] != 0)
        {
            if (!iTrackNumberFoundFlag)
            {
                // This would mean its an ID3v1.1 tag and hence has the
                // the track number also, so extract it
                iVersion = PV_ID3_V1_1;

                if (ConstructKvpKey(keyStr, PV_ID3_FRAME_TRACK_NUMBER, PV_ID3_CHARSET_INVALID) != PVMFSuccess)
                {
                    iAlloc.deallocate(frameData);
                    LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error - ConstructKvpKey failed for tracknumber"));
                    OSCL_LEAVE(OsclErrNotSupported);
                }

                // Allocate key-value pair
                OSCL_TRY(err, kvpPtr = AllocateKvp(keyStr, PVMI_KVPVALTYPE_UINT32, 1, truncate););
                if (OsclErrNone != err || !kvpPtr)
                {
                    LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error - AllocateKvp failed. err=%d", err));
                    iAlloc.deallocate(frameData);
                    OSCL_LEAVE(OsclErrNoMemory);
                    return;
                }

                if (!truncate)
                {
                    kvpPtr->value.uint32_value = (uint32)frameData[ID3V1_MAX_NUM_BYTES_COMMENT - 1];
                }
                OSCL_TRY(err, iFrames.push_back(kvpPtr););
                OSCL_FIRST_CATCH_ANY(err,
                                     LOG_ERR((0, "PVID3ParCom::ReadTrackLengthFrame: Error - iFrame.push_back failed"));
                                     iAlloc.deallocate(frameData);
                                     OSCL_LEAVE(OsclErrNoMemory);
                                     return;
                                    );
                iTrackNumberFoundFlag = true;
            }
        }
        if (!iCommentFoundFlag)
        {

            // Comment frame
            frameData[ID3V1_MAX_NUM_BYTES_COMMENT] = 0;
            if (ConstructKvpKey(keyStr, PV_ID3_FRAME_COMMENT, PV_ID3_CHARSET_ISO88591) != PVMFSuccess)
            {
                LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error - ConstructKvpKey failed for tracknumber"));
                iAlloc.deallocate(frameData);
                OSCL_LEAVE(OsclErrNotSupported);
            }

            uint32 dataSize = ID3V1_MAX_NUM_BYTES_COMMENT + 1;
            // Allocate key-value pair
            OSCL_TRY(err, kvpPtr = AllocateKvp(keyStr, PVMI_KVPVALTYPE_CHARPTR, dataSize, truncate););
            if (OsclErrNone != err || !kvpPtr)
            {
                LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error - AllocateKvp failed. err=%d", err));
                iAlloc.deallocate(frameData);
                OSCL_LEAVE(OsclErrNoMemory);
                return;
            }

            if (!truncate)
            {
                uint32 comment_size = oscl_strlen((char*) frameData);
                oscl_strncpy(kvpPtr->value.pChar_value, (char *)frameData, dataSize);
                kvpPtr->value.pChar_value[comment_size] = 0;
                kvpPtr->length = comment_size + 1;
            }

            if (PVMFSuccess != PushFrameToFrameVector(kvpPtr, iFrames))
            {
                LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error - iFrame.push_back failed"));
                iAlloc.deallocate(frameData);
                return ;
            }
            iCommentFoundFlag = true;

        }
        iAlloc.deallocate(frameData);
    }
    else
    {
        if (iInputFile->Seek(ID3V1_MAX_NUM_BYTES_FIELD_SIZE, Oscl_File::SEEKCUR) == -1)
        {
            return;
        }
    }
    if (!iGenereFoundFlag)
    {
        // Genre frame
        uint32 frameDataSize = ID3V1_MAX_NUM_BYTES_GENRE + 1;
        int32 err = 0;
        frameData = (uint8*) AllocateValueArray(err, PVMI_KVPVALTYPE_UINT8PTR, frameDataSize, &iAlloc);
        if (OsclErrNone != err || !frameData)
        {
            LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error - Out of memory"));
            OSCL_LEAVE(OsclErrNoMemory);
        }
        oscl_memset(frameData, 0, frameDataSize);

        if (readByteData(iInputFile, ID3V1_MAX_NUM_BYTES_GENRE, frameData) == false)
        {
            iAlloc.deallocate(frameData);
            LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error - readByteData failed"));
            OSCL_LEAVE(OsclErrGeneral);
        }

        if (ConstructKvpKey(keyStr, PV_ID3_FRAME_GENRE, PV_ID3_CHARSET_INVALID) != PVMFSuccess)
        {
            iAlloc.deallocate(frameData);
            LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error - ConstructKvpKey failed for genre"));
            OSCL_LEAVE(OsclErrNotSupported);
        }

        OSCL_TRY(err, kvpPtr = AllocateKvp(keyStr, PVMI_KVPVALTYPE_UINT32, 1, truncate););
        if (OsclErrNone != err || !kvpPtr)
        {
            iAlloc.deallocate(frameData);
            LOG_ERR((0, "PVID3ParCom::ReadID3V1Tag: Error - AllocateKvp failed. err=%d", err));
            OSCL_LEAVE(OsclErrNoMemory);
            return;
        }

        if (!truncate)
        {
            kvpPtr->value.uint32_value = (uint32)(frameData[ID3V1_MAX_NUM_BYTES_GENRE - 1]);
        }
        OSCL_TRY(err, iFrames.push_back(kvpPtr););
        OSCL_FIRST_CATCH_ANY(err,
                             iAlloc.deallocate(frameData);
                             LOG_ERR((0, "PVID3ParCom::ReadTrackLengthFrame: Error - iFrame.push_back failed"));
                             OSCL_LEAVE(OsclErrNoMemory);
                             return;
                            );
        iAlloc.deallocate(frameData);
        iGenereFoundFlag = true;
    }
    else
    {
        if (iInputFile->Seek(ID3V1_MAX_NUM_BYTES_GENRE, Oscl_File::SEEKCUR) == -1)
        {
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////
bool PVID3ParCom::ReadHeaderID3V2(bool aReadTags)
{
    bool result = false;

    // Read and convert tag versions, Major and Minor
    uint8 ID3V2MajorVer, ID3V2MinorVer;
    if (read8(iInputFile, ID3V2MajorVer) == false)
    {
        return false;
    }
    if (read8(iInputFile, ID3V2MinorVer) == false)
    {
        return false;
    }

    switch (ID3V2MajorVer)
    {
        case PV_ID3_V2_2:
            iVersion = PV_ID3_V2_2;
            break;
        case PV_ID3_V2_3:
            iVersion = PV_ID3_V2_3;
            break;
        case PV_ID3_V2_4:
            iVersion = PV_ID3_V2_4;
            break;
    }

BEGIN_V2:
    // Read and convert tag flags
    if (read8(iInputFile, iID3TagInfo.iID3V2TagFlagsV2) == false)
    {
        return false;
    }

    uint32 tagsize = 0;

    // Read and convert tag size
    if (read32(iInputFile, tagsize) == false)
    {
        return false;
    }

    // Now check if an extended header exists
    bool extHeaderFlag = false;
    if (iID3TagInfo.iID3V2TagFlagsV2 & EXT_FLAGMASK)
    {
        if (iVersion == PV_ID3_V2_2)
        {
            // Since no compression scheme has been decided yet, just ignore the entire tag
            LOG_ERR((0, "PVID3ParCom::ReadHeaderID3V2(): Error - cannot process a compressed tag"));
            OSCL_LEAVE(OsclErrGeneral);
        }

        extHeaderFlag = true;
        //read extended header data
        if (!ReadExtendedHeader())
            return false;
    }

    //check if footer is present.
    if (iID3TagInfo.iID3V2TagFlagsV2 & FTR_FLAGMASK)
    {
        iID3TagInfo.iFooterPresent = true;
    }

    // tagSize will store the file's Id3v2 tag size
    iID3TagInfo.iID3V2TagSize = SafeSynchIntToInt32(tagsize);

    //calculate start of audio frame.
    if (iTagAtBof)
    {
        // set iByteOffsetToStartOfAudioFrames and it must account for the frame header
        iByteOffsetToStartOfAudioFrames =
            iID3TagInfo.iID3V2TagSize + ID3V2_TAG_NUM_BYTES_HEADER;

        if (iID3TagInfo.iFooterPresent)
            iByteOffsetToStartOfAudioFrames += ID3V2_TAG_NUM_BYTES_HEADER; //header & footer are of same size.
    }
    else
    {
        iByteOffsetToStartOfAudioFrames = 0;
    }

    // Header read is completed, now check whether we need to read the tags or not
    if (!aReadTags)
    {
        return false;
    }
    else
    {
        int count = ReadTagID3V2(iVersion);
        if (count > 0)
        {
            // we found some tags
            result = true;
        }

        if (iSeekFrameFound)
        {
            //jump to the new tag location skipping 5 bytes of tag ID and version.
            uint32 seek_size = iID3TagInfo.iID3V2TagSize + ID3V2_TAG_NUM_BYTES_HEADER + (iID3TagInfo.iFooterPresent ? ID3V2_TAG_NUM_BYTES_HEADER : 0);
            if (iInputFile->Seek(seek_size, Oscl_File::SEEKSET) == -1)
            {
                return false;
            }

            //skip 5 bytes of tag ID and version.
            if (iInputFile->Seek(iID3TagInfo.iID3V2SeekOffset, Oscl_File::SEEKCUR) == -1)
            {
                return false;
            }

            if (!CheckForTagID3V2())
                return false;

            if (read8(iInputFile, ID3V2MajorVer) == false)
            {
                return false;
            }

            if (read8(iInputFile, ID3V2MinorVer) == false)
            {
                return false;
            }

            if (ID3V2MajorVer != PV_ID3_V2_4)
                return false;

            //parse new tag, located at the seek offset.
            iSeekFrameFound = false;
            goto BEGIN_V2;
        }
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////////
bool PVID3ParCom::ReadExtendedHeader()
{

    uint32 extsize = 0;
    if (read32(iInputFile, extsize) == false)
    {
        return false;
    }

    // Calculate the length of the extended header.
    iID3TagInfo.iID3V2ExtendedHeaderSize = SafeSynchIntToInt32(extsize);


    if (iVersion == PV_ID3_V2_4)
    {
        //subtract "ext hdr size" field size from the header size.
        iID3TagInfo.iID3V2ExtendedHeaderSize = iID3TagInfo.iID3V2ExtendedHeaderSize - ID3V2_TAG_EXTENDED_HEADER_SIZE;

        //check if this tag is an update of previous tag.
        uint8 flgsize = 0, exthdrflg = 0;
        // Read and ext hdr flg size tag flags
        if (read8(iInputFile, flgsize) == false)
        {
            return false;
        }
        if (flgsize > 0x01) //flg size should be 1 byte in V2.4
            return false ;
        // Read tag flags
        if (read8(iInputFile, exthdrflg) == false)
        {
            return false;
        }
        //if this tag is not an update flag, ignore any tag found before this one
        if (!(exthdrflg & EXTHDR_UPDMASK) && (iFrames.size() > 0))
        {
            while (iFrames.size() > 0)
                iFrames.pop_back();
        }

        //subtract 2 bytes for flg size and flg from ext hdr size.
        iID3TagInfo.iID3V2ExtendedHeaderSize = iID3TagInfo.iID3V2ExtendedHeaderSize - 2;
    }

    if (iVersion == PV_ID3_V2_3)
    {
        //V2.3 extended hdr size field does not include 6 byte size of the header
        iID3TagInfo.iID3V2ExtendedHeaderSize += ID3V2_TAG_EXTENDED_HEADER_TOTAL_SIZE;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////
int PVID3ParCom::ReadTagID3V2(PVID3Version aVersion)
{
    PVID3FrameType frameType = PV_ID3_FRAME_EEND;
    uint32 i = 0;
    uint32 currFrameLength = 0;
    uint32 current_file_pos = iInputFile->Tell();
    uint32 data_len_indicator_size = 0;
    uint32 count = 0;

    if (iID3TagInfo.iID3V2ExtendedHeaderSize > 0)
        i += iID3TagInfo.iID3V2ExtendedHeaderSize ;

    while (i <= (iID3TagInfo.iID3V2TagSize))
    {
        // Read the frame header
        if (iInputFile->Seek(current_file_pos + i, Oscl_File::SEEKSET) == -1)
        {
            return count;
        }

        ReadFrameHeaderID3V2(aVersion);

        currFrameLength = iID3TagInfo.iID3V2FrameSize;
        frameType = FrameSupportedID3V2(aVersion);

        if ((frameType != PV_ID3_FRAME_SEEK) && (currFrameLength > iID3TagInfo.iID3V2TagSize))
        {
            break;
        }
        // handle the frame header

        uint32 frame_header_size = 0;
        if (aVersion == PV_ID3_V2_2)
            frame_header_size = ID3V2_2_FRAME_NUM_BYTES_HEADER;
        else
            frame_header_size = ID3V2_FRAME_NUM_BYTES_HEADER;


        if ((currFrameLength == 0) && (frameType != PV_ID3_FRAME_EEND))
        {
            i += frame_header_size;
            continue;
        }
        if (aVersion == PV_ID3_V2_3)
        {
            if (iID3TagInfo.iID3V2FrameFlag[1] & ENCR_COMP_3_FLAGMASK)
                frameType = PV_ID3_FRAME_UNRECOGNIZED;
        }
        else if (aVersion == PV_ID3_V2_4)
        {
            if (iID3TagInfo.iID3V2FrameFlag[1] & ENCR_COMP_4_FLAGMASK)
                frameType = PV_ID3_FRAME_UNRECOGNIZED;
        }

        if (frameType == PV_ID3_FRAME_SEEK)
        {
            iSeekFrameFound = true;
            iID3TagInfo.iID3V2SeekOffset = iID3TagInfo.iID3V2FrameSize;
            return count;
        }

        // Check if data length indicator is present
        if (aVersion == PV_ID3_V2_4 && (iID3TagInfo.iID3V2FrameFlag[1] & FRAME_LENGTH_INDICATOR_FLAGMASK))
        {
            uint32 temp = 0;
            // Read data length indicator
            if (read32(iInputFile, temp) == false)
            {
                return count;
            }
            // stored as syncsafe integer
            currFrameLength = SafeSynchIntToInt32(temp);

            data_len_indicator_size = ID3V2_4_DATA_LENGTH_INDICATOR_SIZE;
        }

        if (((currFrameLength > 1) && (frameType != PV_ID3_FRAME_UNRECOGNIZED
                                       && frameType != PV_ID3_FRAME_INVALID
                                       && frameType != PV_ID3_FRAME_EEND
                                       && frameType != PV_ID3_FRAME_CANDIDATE)))
        {
            uint8 unicodeCheck;

            if (read8(iInputFile, unicodeCheck) == false)
            {
                return count;
            }

            if ((frameType == PV_ID3_FRAME_LYRICS) || (frameType == PV_ID3_FRAME_COMMENT))
            {
                ReadLyricsCommFrame(unicodeCheck, currFrameLength - 1, frameType);

            }
            else if ((frameType == PV_ID3_FRAME_APIC) || (frameType == PV_ID3_FRAME_PIC))
            {
                if (ReadAlbumArtFrame(frameType, unicodeCheck, currFrameLength)  != PVMFSuccess)
                {
                    LOG_ERR((0, "PVID3ParCom::ReadTagID3V2: Error - ReadAPICFrame failed"));

                    return count;
                }
            }
            else if (unicodeCheck < PV_ID3_CHARSET_END)
            {

                if (!ReadFrameData(unicodeCheck, frameType,
                                   current_file_pos + i + frame_header_size + data_len_indicator_size + 1,
                                   currFrameLength))
                {
                    return count;
                }
            }
            else
            {
                // This case is when no text type is defined in the frame.
                HandleID3V2FrameDataASCII(frameType, i + frame_header_size + data_len_indicator_size, currFrameLength);
            }
            count++;
        }
        else
        {
            if (frameType == PV_ID3_FRAME_EEND ||
                    frameType == PV_ID3_FRAME_INVALID)
            {
                i = iID3TagInfo.iID3V2TagSize + 1;
            }
            else if (frameType == PV_ID3_FRAME_UNRECOGNIZED ||
                     frameType == PV_ID3_FRAME_CANDIDATE) // handle candidate frames as we do unsupported
            {
                if (i < iID3TagInfo.iID3V2TagSize)
                {
                    HandleID3V2FrameUnsupported(frameType,
                                                current_file_pos + i,
                                                currFrameLength + frame_header_size + data_len_indicator_size);
                }
            }
        }

        i += iID3TagInfo.iID3V2FrameSize + frame_header_size;
    }
    return count;
}

bool PVID3ParCom::ValidateFrameV2_4(PVID3FrameType& frameType, bool bUseSyncSafeFrameSize)
{
    // Initialize OUT param
    frameType = PV_ID3_FRAME_INVALID;

    uint8 frameid[ID3V2_FRAME_NUM_BYTES_ID + 1] = {0};
    // read frame id for next frame
    if (readByteData(iInputFile, ID3V2_FRAME_NUM_BYTES_ID, frameid) == false)
    {
        return false;
    }
    frameid[ID3V2_FRAME_NUM_BYTES_ID] = 0;

    // Get frame type from frame ID
    frameType = FrameSupportedID3V2(PV_ID3_V2_4, frameid);
    if (PV_ID3_FRAME_INVALID == frameType ||
            PV_ID3_FRAME_EEND == frameType)
    {
        return false;
    }
    else
    {
        uint32 frameSize = 0;
        uint8 frameflags[ID3V2_FRAME_NUM_BYTES_FLAG] = {0};

        // Validate frame size and flags

        if (read32(iInputFile, frameSize) == false)
        {
            return false;
        }
        if (bUseSyncSafeFrameSize)
        {
            frameSize = SafeSynchIntToInt32(frameSize);
        }

        if (readByteData(iInputFile, ID3V2_FRAME_NUM_BYTES_FLAG, frameflags) == false)
        {
            return false;
        }

        if ((0 == frameSize ||
                (frameSize + ID3V2_FRAME_NUM_BYTES_HEADER) > iID3TagInfo.iID3V2TagSize) ||
                ((frameflags[0] & ID3V2_4_MASK_FRAME_FLAG_VERIFICATION) ||
                 (frameflags[1] & ID3V2_4_MASK_FRAME_FLAG_VERIFICATION)))
        {
            // validation for frame size or flags failed
            return false;
        }
        else
        {
            return true;
        }
    }
}

uint32 PVID3ParCom::ValidateFrameLengthV2_4(uint32 aFrameSize)
{
    int32 currFilePos = iInputFile->Tell();
    int32 errCode = -1;
    int32 actualFrameLen = 0;

    // we have already read the complete current frame header

    do
    {
        // Assuming syncsafe frame size
        actualFrameLen = SafeSynchIntToInt32(aFrameSize);

        //
        /* validate frame using syncsafe size */
        //

        // Seek to next frame boundary with syncsafe size
        errCode = iInputFile->Seek(actualFrameLen, Oscl_File::SEEKCUR);
        if (-1 == errCode)
        {
            // proceed with default syncsafe handling
            break;
        }

        bool bIsSyncSafeFrameValid = false;
        PVID3FrameType frameTypeUsingSyncSafeSize = PV_ID3_FRAME_INVALID;
        // Get the validation status and frame type
        bIsSyncSafeFrameValid = ValidateFrameV2_4(frameTypeUsingSyncSafeSize);

        //
        /* validate frame using non-syncsafe size */
        //

        // FrameSize is not even stored as non-syncsafe
        if ((aFrameSize + ID3V2_FRAME_NUM_BYTES_HEADER) > iID3TagInfo.iID3V2TagSize)
        {
            // proceed with syncsafe, as non-syncsafe length not valid
            break;
        }

        // Seek back
        errCode = iInputFile->Seek(currFilePos, Oscl_File::SEEKSET);
        if (-1 == errCode)
        {
            LOG_ERR((0, "PVID3ParCom::ValidateFrameLengthV2_4: Error - iInputFile->Seek failed"));
            OSCL_LEAVE(OsclErrGeneral);
        }
        // Seek to next frame boundary with non-syncsafe size
        errCode = iInputFile->Seek(aFrameSize, Oscl_File::SEEKCUR);
        if (-1 == errCode)
        {
            // proceed with default syncsafe handling
            break;
        }

        bool bIsNonSyncSafeFrameValid = false;
        PVID3FrameType frameTypeUsingNonSyncSafeSize = PV_ID3_FRAME_INVALID;
        // Get the validation status and frame type
        bIsNonSyncSafeFrameValid = ValidateFrameV2_4(frameTypeUsingNonSyncSafeSize, false);

        // - Give more priority to non-syncsafe VALID frame ID,
        //		than syncsafe candidate frame ID (frame validation is true for both)
        // - In case, we have frame validation true for both syncsafe and non-syncsafe size,
        //		we will use default syncsafe representation
        // - In case, we have frame validation false for both syncsafe and non-syncsafe size,
        //		we will use default syncsafe representation
        if (bIsSyncSafeFrameValid && frameTypeUsingSyncSafeSize != PV_ID3_FRAME_CANDIDATE)
        {
            // syncsafe representation
        }
        else if (bIsNonSyncSafeFrameValid && frameTypeUsingNonSyncSafeSize != PV_ID3_FRAME_CANDIDATE)
        {
            // non-syncsafe representation
            actualFrameLen = aFrameSize;
        }
        else if (!bIsSyncSafeFrameValid && bIsNonSyncSafeFrameValid)
        {
            // non-syncsafe representation
            actualFrameLen = aFrameSize;
        }
        else
        {
            // consider rest all syncsafe representation
        }

    }
    while (false);

    // Seek back
    errCode = iInputFile->Seek(currFilePos, Oscl_File::SEEKSET);
    if (-1 == errCode)
    {
        LOG_ERR((0, "PVID3ParCom::ValidateFrameLengthV2_4: Error - iInputFile->Seek failed"));
        OSCL_LEAVE(OsclErrGeneral);
    }

    return actualFrameLen;
}

//////////////////////////////////////////////////////////////////////////
bool  PVID3ParCom::ReadFrameData(uint8 unicodeCheck, PVID3FrameType frameType, uint32 pos, uint32 currFrameLength)
{
    if (unicodeCheck == PV_ID3_CHARSET_ISO88591)
    {
        // This frame contains normal ASCII text strings. (ISO-8859-1)
        iID3TagInfo.iTextType = PV_ID3_CHARSET_ISO88591;
        HandleID3V2FrameDataASCII(frameType, pos, currFrameLength - 1);
    }
    else if (unicodeCheck == PV_ID3_CHARSET_UTF16)
    {
        uint16 endianCheck;

        if (read16(iInputFile, endianCheck) == false)
        {
            return false;
        }

        // This frame's text strings are Unicode and the frame
        // does include a BOM value. (UTF-16)
        iID3TagInfo.iTextType = PV_ID3_CHARSET_UTF16;
        uint32 endianType;
        if (endianCheck == UNICODE_LITTLE_ENDIAN_INDICATOR_FULL)
        {
            endianType = UNICODE_LITTLE_ENDIAN;
        }
        else if (endianCheck == UNICODE_BIG_ENDIAN_INDICATOR_FULL)
        {
            endianType = UNICODE_BIG_ENDIAN;
        }
        else
        {
            return false;
        }

        // value of 2 is for BOM Character
        HandleID3V2FrameDataUnicode16(frameType, pos + 2, currFrameLength - 3, endianType);
    }
    else if (unicodeCheck == PV_ID3_CHARSET_UTF16BE)
    {
        // This frame's text strings are Unicode but the frame
        // does not contain a BOM(byte order mark) (UTF-16BE)
        iID3TagInfo.iTextType = PV_ID3_CHARSET_UTF16BE;
        // Default ID3V2 endian type to Big Endian
        uint32 endianType = UNICODE_BIG_ENDIAN;
        // Big Endian is assumed since the frame did not specify the endian type.
        HandleID3V2FrameDataUnicode16(frameType, pos, currFrameLength - 1, endianType);
    }
    else if (unicodeCheck == PV_ID3_CHARSET_UTF8)
    {
        // This frame's text strings are Unicode (UTF-8)
        iID3TagInfo.iTextType = PV_ID3_CHARSET_UTF8;
        HandleID3V2FrameDataUTF8(frameType,	pos, currFrameLength - 1);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
void PVID3ParCom::ReadFrameHeaderID3V2(PVID3Version aVersion)
{
    if (aVersion == PV_ID3_V2_2)
    {
        ReadFrameHeaderID3V2_2();
    }
    else
    {
        // Read frame ID
        if (readByteData(iInputFile, ID3V2_FRAME_NUM_BYTES_ID, iID3TagInfo.iID3V2FrameID) == false)
        {
            return;
        }
        iID3TagInfo.iID3V2FrameID[ID3V2_FRAME_NUM_BYTES_ID] = 0;
        // Read frame size
        if (read32(iInputFile, iID3TagInfo.iID3V2FrameSize) == false)
        {
            return;
        }
        // Read frame flag
        if (readByteData(iInputFile, ID3V2_FRAME_NUM_BYTES_FLAG, iID3TagInfo.iID3V2FrameFlag) == false)
        {
            return;
        }

        if (PV_ID3_V2_4 == aVersion)
        {
            if (iID3TagInfo.iID3V2FrameSize > MAX_SYNCSAFE_LEN)
            {
                // Verify whether frame length is SyncSafe or Non-SyncSafe
                iID3TagInfo.iID3V2FrameSize = ValidateFrameLengthV2_4(iID3TagInfo.iID3V2FrameSize);
            }
        }
    }
    return;
}

////////////////////////////////////////////////////////////////////////////
void PVID3ParCom::ReadFrameHeaderID3V2_2()
{
    if (readByteData(iInputFile, ID3V2_2_FRAME_NUM_BYTES_ID, iID3TagInfo.iID3V2FrameID) == false)
    {
        return;
    }
    iID3TagInfo.iID3V2FrameID[ID3V2_2_FRAME_NUM_BYTES_ID] = 0;
    if (read24(iInputFile, iID3TagInfo.iID3V2FrameSize) == false)
    {
        return;
    }

    return;
}

//////////////////////////////////////////////////////////////////////////////
void PVID3ParCom::HandleID3V2FrameDataASCII(PVID3FrameType aFrameType,
        uint32         aPos,
        uint32         aSize)
{
    OSCL_StackString<128> keyStr;
    PvmiKvpSharedPtr kvpPtr;
    PVMFStatus status = PVMFSuccess;
    if (iInputFile->Seek(aPos, Oscl_File::SEEKSET) == -1)
    {
        LOG_ERR((0, "PVID3ParCom::HandleID3V2FrameDataASCII: Error - iInputFile->Seek failed"));
        OSCL_LEAVE(OsclErrGeneral);
    }

    switch (aFrameType)
    {
        case PV_ID3_FRAME_TRACK_LENGTH:
            status = ReadTrackLengthFrame(aSize, PV_ID3_CHARSET_ISO88591);
            if (status != PVMFSuccess)
            {
                LOG_ERR((0, "PVID3ParCom::HandleID3V2FrameDataASCII: Error - ReadTrackLengthFrame failed. status=%d", status));
                OSCL_LEAVE(OsclErrGeneral);
            }
            break;
        case PV_ID3_FRAME_TITLE:
        case PV_ID3_FRAME_ARTIST:
        case PV_ID3_FRAME_ALBUM:
        case PV_ID3_FRAME_YEAR:
        case PV_ID3_FRAME_TRACK_NUMBER:
        case PV_ID3_FRAME_GENRE:
        case PV_ID3_FRAME_COPYRIGHT:
        case PV_ID3_FRAME_DATE:
        case PV_ID3_FRAME_RECORDING_TIME:
//new frames support
        case PV_ID3_FRAME_AUTHOR:
        case PV_ID3_FRAME_COMPOSER:
        case PV_ID3_FRAME_DESCRIPTION:
        case PV_ID3_FRAME_VERSION:
        case PV_ID3_FRAME_PART_OF_SET:

            status = ReadStringValueFrame(aFrameType, PV_ID3_CHARSET_ISO88591, aSize);
            if (status != PVMFSuccess)
            {
                LOG_ERR((0, "PVID3ParCom::HandleID3V2FrameDataASCII: Error - ReadStringValueFrame failed. status=%d", status));
                OSCL_LEAVE(OsclErrGeneral);
            }
            if (aFrameType == PV_ID3_FRAME_TITLE)
                iTitleFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_ARTIST)
                iArtistFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_ALBUM)
                iAlbumFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_YEAR)
                iYearFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_TRACK_NUMBER)
                iTrackNumberFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_GENRE)
                iGenereFoundFlag = true;

            break;

        default:
            break;
    }
}

void PVID3ParCom::HandleID3V2FrameUnsupported(PVID3FrameType aFrameType,
        uint32         aPos,
        uint32         aSize)
{
    OSCL_StackString<128> keyStr;
    PvmiKvpSharedPtr kvpPtr;
    PVMFStatus status = PVMFSuccess;
    if (iInputFile->Seek(aPos, Oscl_File::SEEKSET) == -1)
    {
        LOG_ERR((0, "PVID3ParCom::HandleID3V2FrameDataASCII: Error - iInputFile->Seek failed"));
        OSCL_LEAVE(OsclErrGeneral);
    }

    status = ReadFrame(aFrameType, aSize);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVID3ParCom::HandleID3V2FrameDataASCII: Error - ReadStringValueFrame failed. status=%d", status));
        OSCL_LEAVE(OsclErrGeneral);
    }

}

////////////////////////////////////////////////////////////////////////////
void PVID3ParCom::HandleID3V2FrameDataUnicode16(PVID3FrameType aFrameType,
        uint32 aPos,
        uint32 aSize,
        uint32 aEndianType)
{
    // seek to the beginning of the current frame data
    if (iInputFile->Seek(aPos, Oscl_File::SEEKSET) == -1)
    {
        LOG_ERR((0, "PVID3ParCom::HandleID3V2FrameDataUnicode16: Error - iInputFile->Seek failed"));
        OSCL_LEAVE(OsclErrGeneral);
        return;
    }

    PVMFStatus status = PVMFSuccess;
    PVID3CharacterSet charSet;
    if (aEndianType == UNICODE_BIG_ENDIAN)
        charSet = PV_ID3_CHARSET_UTF16BE;
    else
        charSet = PV_ID3_CHARSET_UTF16;

    switch (aFrameType)
    {

        case PV_ID3_FRAME_TRACK_LENGTH:
            //is a numeric string and does not depend on text encoding.
            status = ReadTrackLengthFrame(aSize, charSet);
            if (status != PVMFSuccess)
            {
                LOG_ERR((0, "PVID3ParCom::HandleID3V2FrameDataUnicode16: Error - ReadTrackLengthFrame failed. status=%d", status));
                OSCL_LEAVE(OsclErrGeneral);
                return;
            }
            break;
        case PV_ID3_FRAME_TITLE:
        case PV_ID3_FRAME_ARTIST:
        case PV_ID3_FRAME_ALBUM:
        case PV_ID3_FRAME_YEAR:
        case PV_ID3_FRAME_TRACK_NUMBER:
        case PV_ID3_FRAME_GENRE:
        case PV_ID3_FRAME_COPYRIGHT:
        case PV_ID3_FRAME_DATE:
        case PV_ID3_FRAME_RECORDING_TIME:
//new frames support
        case PV_ID3_FRAME_AUTHOR:
        case PV_ID3_FRAME_COMPOSER:
        case PV_ID3_FRAME_DESCRIPTION:
        case PV_ID3_FRAME_VERSION:
        case PV_ID3_FRAME_PART_OF_SET:

            status = ReadStringValueFrame(aFrameType, charSet, aSize);
            if (status != PVMFSuccess)
            {
                LOG_ERR((0, "PVID3ParCom::HandleID3V2FrameDataUnicode16: Error - ReadStringValueFrame failed. status=%d", status));
                OSCL_LEAVE(OsclErrGeneral);
                return;
            }
            if (aFrameType == PV_ID3_FRAME_TITLE)
                iTitleFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_ARTIST)
                iArtistFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_ALBUM)
                iAlbumFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_YEAR)
                iYearFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_TRACK_NUMBER)
                iTrackNumberFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_GENRE)
                iGenereFoundFlag = true;

            break;

        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
void PVID3ParCom::HandleID3V2FrameDataUTF8(PVID3FrameType aFrameType,
        uint32         aPos,
        uint32         aSize)
{
    if (iInputFile->Seek(aPos, Oscl_File::SEEKSET) == -1)
    {
        LOG_ERR((0, "PVID3ParCom::HandleID3V2FrameDataUTF8: Error - iInputFile->Seek failed"));
        OSCL_LEAVE(OsclErrGeneral);
    }

    PVMFStatus status = PVMFSuccess;
    switch (aFrameType)
    {

        case PV_ID3_FRAME_TRACK_LENGTH:
            status = ReadTrackLengthFrame(aSize, PV_ID3_CHARSET_UTF8);
            if (status != PVMFSuccess)
            {
                LOG_ERR((0, "PVID3ParCom::HandleID3V2FrameDataUTF8: Error - ReadTrackLengthFrame failed. status=%d", status));
                OSCL_LEAVE(OsclErrGeneral);
            }
            break;

        case PV_ID3_FRAME_TITLE:
        case PV_ID3_FRAME_ARTIST:
        case PV_ID3_FRAME_ALBUM:
        case PV_ID3_FRAME_YEAR:
        case PV_ID3_FRAME_TRACK_NUMBER:
        case PV_ID3_FRAME_GENRE:
        case PV_ID3_FRAME_COPYRIGHT:
        case PV_ID3_FRAME_DATE:
        case PV_ID3_FRAME_RECORDING_TIME:
//new frames support
        case PV_ID3_FRAME_AUTHOR:
        case PV_ID3_FRAME_COMPOSER:
        case PV_ID3_FRAME_DESCRIPTION:
        case PV_ID3_FRAME_VERSION:
        case PV_ID3_FRAME_PART_OF_SET:

            status = ReadStringValueFrame(aFrameType, PV_ID3_CHARSET_UTF8, aSize);
            if (status != PVMFSuccess)
            {
                LOG_ERR((0, "PVID3ParCom::HandleID3V2FrameDataUTF8: Error - ReadStringValueFrame failed. status=%d", status));
                OSCL_LEAVE(OsclErrGeneral);
            }
            if (aFrameType == PV_ID3_FRAME_TITLE)
                iTitleFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_ARTIST)
                iArtistFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_ALBUM)
                iAlbumFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_YEAR)
                iYearFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_TRACK_NUMBER)
                iTrackNumberFoundFlag = true;
            if (aFrameType == PV_ID3_FRAME_GENRE)
                iGenereFoundFlag = true;

            break;

        default:
            break;
    }
}

//----------------------------------------------------------------------------
// FUNCTION NAME: PVID3ParCom::FrameSupportedID3V2(PVID3Version)
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs: None
//
//  Outputs: None
//
//  Returns:
//    ID3V2FrameTypeReturnValue - The value that describes the current frame.
//                                of type enum TID3V2FrameType
//
//  Global Variables Used:
//    TID3V2FrameType - The enum table containing the supported frame types
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function detects the ID3V2FrameType and returns the enum value that
//  corresponds to the current frame.
//
//------------------------------------------------------------------------------

PVID3ParCom::PVID3FrameType PVID3ParCom::FrameSupportedID3V2(PVID3Version aVersion, uint8* aframeid)
{
    PVID3FrameType ID3V2FrameTypeReturnValue;
    uint8* pFrameID = (aframeid) ? aframeid : iID3TagInfo.iID3V2FrameID;

    if (0xff == pFrameID[0])
    {
        //possibly start of mp3 frame. Stop further parsing.
        return PV_ID3_FRAME_EEND;
    }

    if (aVersion == PV_ID3_V2_2)
    {
        ID3V2FrameTypeReturnValue = FrameSupportedID3V2_2();
    }
    else	//for v2.3 & v2.4
    {
        uint8 endTestBuf[ID3V2_FRAME_NUM_BYTES_ID] = {0};

        if (oscl_memcmp(pFrameID, ID3_FRAME_ID_TITLE, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_TITLE;
            iTitleFoundFlag = true;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_ARTIST, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_ARTIST;
            iArtistFoundFlag = true;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_PART_OF_SET, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_PART_OF_SET;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_ALBUM, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_ALBUM;
            iAlbumFoundFlag = true;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_COPYRIGHT, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_COPYRIGHT;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_GENRE, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_GENRE;
            iGenereFoundFlag = true;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_TRACK_NUMBER, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_TRACK_NUMBER;
            iTrackNumberFoundFlag = true;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_TRACK_LENGTH, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_TRACK_LENGTH;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_COMMENT, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_COMMENT;
            iCommentFoundFlag = true;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_YEAR, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_YEAR;
            iYearFoundFlag = true;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_RECORDING_TIME, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_RECORDING_TIME;
            iYearFoundFlag = true;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_DATE, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_DATE;
        }

        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_ALBUMART, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_APIC;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_LYRICIST, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_LYRICS;
        }

        // For the seek frame.
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_SEEK, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_SEEK;
        }
        //new frames support
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_AUTHOR, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_AUTHOR;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_COMPOSER, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_COMPOSER;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_DESCRIPTION, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_DESCRIPTION;
        }
        else if (oscl_memcmp(pFrameID, ID3_FRAME_ID_VERSION, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_VERSION;
        }
        else if (oscl_memcmp(pFrameID, endTestBuf, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_EEND;
        }
        else if (oscl_memcmp(pFrameID, endTestBuf, ID3V2_FRAME_NUM_BYTES_ID) == 0)
        {
            ID3V2FrameTypeReturnValue = PV_ID3_FRAME_EEND;
        }
        else
        {
            // Find whether frame is invalid or unsupported
            ID3V2FrameTypeReturnValue = FrameValidatedID3V2_4(pFrameID);
        }
    }
    return ID3V2FrameTypeReturnValue;
}

//----------------------------------------------------------------------------
// FUNCTION NAME: PVID3ParCom::FrameSupportedID3V2_2(void)
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs: None
//
//  Outputs: None
//
//  Returns:
//    ID3V2_2FrameTypeReturnValue - The value that describes the current frame.
//                                of type enum TID3V2FrameType
//
//  Global Variables Used:
//    TID3V2_2FrameType - The enum table containing the supported frame types
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function detects the ID3V2_2FrameType and returns the enum value that
//  corresponds to the current frame.
//
//------------------------------------------------------------------------------

PVID3ParCom::PVID3FrameType PVID3ParCom::FrameSupportedID3V2_2(void)
{
    uint8 endTestBuf[ID3V2_2_FRAME_NUM_BYTES_ID] = {0};
    PVID3FrameType ID3V2_2FrameTypeReturnValue;

    if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_TITLE, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_TITLE;
        iTitleFoundFlag = true;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_ARTIST, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_ARTIST;
        iArtistFoundFlag = true;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_PART_OF_SET, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_PART_OF_SET;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_ALBUM, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_ALBUM;
        iAlbumFoundFlag = true;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_COPYRIGHT, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_COPYRIGHT;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_GENRE, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_GENRE;
        iGenereFoundFlag = true;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_TRACK_NUMBER, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_TRACK_NUMBER;
        iTrackNumberFoundFlag = true;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_TRACK_LENGTH, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_TRACK_LENGTH;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_COMMENT, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_COMMENT;
        iCommentFoundFlag = true;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_YEAR, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_YEAR;
        iYearFoundFlag = true;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_DATE, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_DATE;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_ALBUMART, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_PIC;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_LYRICIST, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_LYRICS;
    }
//new frames support
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_AUTHOR, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_AUTHOR;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_COMPOSER, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_COMPOSER;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_DESCRIPTION, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_DESCRIPTION;
    }
    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, ID3V2_2_FRAME_ID_VERSION, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_VERSION;
    }

    else if (oscl_memcmp(iID3TagInfo.iID3V2FrameID, endTestBuf, ID3V2_2_FRAME_NUM_BYTES_ID) == 0)
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_EEND;
    }

    else
    {
        ID3V2_2FrameTypeReturnValue = PV_ID3_FRAME_UNRECOGNIZED;
    }

    return ID3V2_2FrameTypeReturnValue;
}

//----------------------------------------------------------------------------
// FUNCTION NAME: PVID3ParCom::EightBitToWideCharBufferTransfer(
//                                               const TPtr8& aPtrFrameData8,
//                                               uint32         aSize,
//                                               uint32         aEndianType,
//                                               TPtr16&      aPtrFrameData16)
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    aPtrFrameData8  - pointer to intput string format of either big
//                      or little endian.
//    aSize           - number of character elements in aPtrFrameData8
//    aEndianType     - This describes if the encoded Unicode text in the
//                      aPtrFrameData8 buffer is in big or little endian.
//  Outputs:
//    aPtrFrameDataWCBase - pointer to the output string in unicode format.
//
//  Returns: size of data in aPtrFrameDataWCBase
//
//  Global Variables Used: None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function moves unicode data sotred in uint8 buffer to wchar buffer.
//	The endian is taken care of by this function as well.
//
//------------------------------------------------------------------------------
uint32 PVID3ParCom::EightBitToWideCharBufferTransfer(const uint8 * aPtrFrameData8,
        uint32        aSize,
        uint32        aEndianType,
        oscl_wchar  * aPtrFrameDataWCBase)
{
    if (aPtrFrameData8 == NULL || aPtrFrameDataWCBase == NULL)
    {
        return 0;
    }

    oscl_wchar * aPtrFrameDataWC = aPtrFrameDataWCBase;
    uint16 tempFrameData16;
    oscl_wchar tempFrameDataWC = 0;
    uint32 index = 0;
    uint32 z = 0, x = 0;
    uint8 tempByteOne, tempByteTwo;
    if (aEndianType != UNICODE_BIG_ENDIAN)
        // Indication check of big-endian vs. little endian
    {
        z = 0;
        for (x = 0; x < (aSize >> 1); x++)
        {
            z = x << 1;
            tempByteOne = aPtrFrameData8[z];
            tempByteTwo = aPtrFrameData8[z + 1];
            if ((tempByteOne == 0) && (tempByteTwo == 0))
            {
                x++;
                // End of string here and skip to start of next string.
                aPtrFrameDataWC[index++] = ((oscl_wchar)'\0');
            }
            else
            {
                tempFrameData16 = (uint16)((((uint16)(tempByteTwo << 8)) | tempByteOne));
                tempFrameDataWC = tempFrameData16;
                aPtrFrameDataWC[index++] = tempFrameDataWC;
            }
        }
    }
    else
    {
        z = 0;
        for (x = 0; x < (aSize >> 1); x++)
        {
            z = x << 1;
            tempByteOne = aPtrFrameData8[z];
            tempByteTwo = aPtrFrameData8[z + 1];
            if ((tempByteTwo == 0) && (tempByteOne == 0))
            {
                x++;
                // End of string here and skip to start of next string.
                aPtrFrameDataWC[index++] = ((oscl_wchar)'\0');
            }
            else
            {
                tempFrameData16 = (uint16)((((uint16)(tempByteOne << 8)) | tempByteTwo));
                int32 l = sizeof(oscl_wchar);
                if (l == 4)
                    tempFrameDataWC = (tempFrameData16 << 16) && 0xff00;
                else if (l == 2)
                    tempFrameDataWC = tempFrameData16;
                else
                    return 0;
                aPtrFrameDataWC[index++] = tempFrameDataWC;
            }
        }
    }
    return (index);
}

uint32 PVID3ParCom::ConvertToSyncSafeInt(uint32 src)
{
    uint32	dest = (src & 0x0FE00000) << 3;
    dest |= (src & 0x001FC000) << 2;
    dest |= (src & 0x00003f80) << 1;
    dest |= (src & 0x0000007f);

    return dest;
}

////////////////////////////////////////////////////////////////////////////
uint32 PVID3ParCom::SafeSynchIntToInt32(uint32 aSafeSynchInteger)
{
    uint8 * pBuf = (uint8 *) & (aSafeSynchInteger);
    uint8 tmpByte = 0;
    int32 i = 0;
    uint32 integer = 0;

    // This loop will calculate the correct size from the bytes designated for size
    // It is stored as a SynchSafe Integer. This means the 8th bit is reserved in
    // each byte and not used for integer precision.. The number is effectively 28
    // bits in length/precision.
    // Assumes: sizeof(uint32) = 4 Bytes
    for (i = 0; i < 4; i++)
    {
#if (OSCL_BYTE_ORDER_LITTLE_ENDIAN)
        tmpByte = (uint8)(pBuf[i] & MASK127);
#elif (OSCL_BYTE_ORDER_BIG_ENDIAN)
        tmpByte = pBuf[4-i-1] & MASK127;
#else
#error "Must Specify ENDIANNESS"
#endif
        // now shift the data to it's correct place
        integer += tmpByte << VALID_BITS_IN_SYNC_SAFE_BYTE * i;
    }

    return integer;
}

////////////////////////////////////////////////////////////////////////////
bool PVID3ParCom::readByteData(PVFile* aInputFile, uint32 length, uint8 *aData)
{
    if (length > 0)
    {
        uint32 bytesRead;
        bytesRead = aInputFile->Read(aData, 1, length);

        if (bytesRead < (uint32)length) // read byte data failed
        {
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool PVID3ParCom::read32(PVFile* aInputFile, uint32 &aData)
{
    const int32 N = 4;
    uint8 bytes[N];
    aData = 0;

    int32 retVal = (int32)(aInputFile->Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    for (int32 i = 0; i < N; i++)
        aData = (aData << 8) | bytes[i];

    return true;
}
////////////////////////////////////////////////////////////////////////////
bool PVID3ParCom::read16(PVFile* aInputFile, uint16 &aData)
{
    const int32 N = 2;
    uint8 bytes[N];
    aData = 0;

    int32 retVal = (int32)(aInputFile->Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    for (int32 i = 0; i < N; i++)
        aData = (uint16)((aData << 8) | bytes[i]);

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool PVID3ParCom::read24(PVFile* aInputFile, uint32 &aData)
{
    const int32 N = 3;
    uint8 bytes[N];
    aData = 0;

    int32 retVal = (int32)(aInputFile->Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    for (int32 i = 0; i < N; i++)
        aData = (aData << 8) | bytes[i];

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool PVID3ParCom::read8(PVFile* aInputFile, uint8 &aData)
{
    aData = 0;

    int32 retVal = (int32)(aInputFile->Read((void*) & aData, 1, 1));

    if (retVal < 1)
        return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////

PvmiKvpSharedPtr PVID3ParCom::AllocateKvp(OSCL_String& aKey, PvmiKvpValueType aValueType, uint32 aValueSize, bool &truncate)
{
    uint8* myPtr = NULL;
    OsclRefCounter* refCounter;

    uint aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
    uint aligned_cleanup_size = oscl_mem_aligned_size(sizeof(PVID3ParComKvpCleanupDA));
    uint aligned_kvp_size = oscl_mem_aligned_size(sizeof(PvmiKvp));
    uint32 key_size = oscl_mem_aligned_size(aKey.get_size() + 1);
    uint32 total_size = aligned_refcnt_size + aligned_cleanup_size +
                        aligned_kvp_size + key_size + aValueSize;

    int32 error = 0;
    myPtr = (uint8*) AllocateValueArray(error, PVMI_KVPVALTYPE_UINT8PTR, total_size, &iAlloc);
    if ((error != OsclErrNone) || (!myPtr) || (total_size < aValueSize))
    {
        if (myPtr)
        {
            iAlloc.deallocate(myPtr);
        }

        char str_data_len[MAX_RANGE_INT_SIZE + 1] = {0};
        oscl_snprintf(str_data_len, MAX_RANGE_INT_SIZE + 1, "%d", aValueSize);
        aKey += SEMI_COLON;
        aKey += KVP_VALTYPE_REQSIZE;
        aKey += _STRLIT_CHAR((char *)str_data_len);
        uint32 new_key_size = oscl_mem_aligned_size(aKey.get_size() + 1);
        error = OsclErrNone;
        total_size = aligned_refcnt_size + aligned_cleanup_size + aligned_kvp_size + new_key_size;
        myPtr = (uint8*) AllocateValueArray(error, PVMI_KVPVALTYPE_UINT8PTR, total_size, &iAlloc);
        truncate = true;
        if (OsclErrNone != error)
        {
            OSCL_LEAVE(OsclErrNoMemory);
        }
    }
    oscl_memset(myPtr, 0, total_size);

    PVID3ParComKvpCleanupDA* kvpCleanup = OSCL_PLACEMENT_NEW(myPtr + aligned_refcnt_size, PVID3ParComKvpCleanupDA(&iAlloc));
    refCounter = OSCL_PLACEMENT_NEW(myPtr, OsclRefCounterDA(myPtr, kvpCleanup));
    myPtr += aligned_refcnt_size + aligned_cleanup_size;

    PvmiKvp* kvp = OSCL_STATIC_CAST(PvmiKvp *, myPtr) ;
    myPtr += aligned_kvp_size;

    kvp->key = OSCL_STATIC_CAST(char* , myPtr);
    myPtr += key_size;
    oscl_strncpy(kvp->key, aKey.get_cstr(), key_size);
    if (!truncate)
    {
        switch (aValueType)
        {
            case PVMI_KVPVALTYPE_WCHARPTR:
                kvp->value.pWChar_value = OSCL_STATIC_CAST(oscl_wchar*, myPtr);
                break;
            case PVMI_KVPVALTYPE_CHARPTR:
                kvp->value.pChar_value = OSCL_STATIC_CAST(char* , myPtr);
                break;
            case PVMI_KVPVALTYPE_UINT8PTR:
                kvp->value.pUint8_value = OSCL_STATIC_CAST(uint8*, myPtr);
                break;
            case PVMI_KVPVALTYPE_INT32PTR:
                kvp->value.pInt32_value = OSCL_STATIC_CAST(int32*, myPtr);
                break;
            case PVMI_KVPVALTYPE_UINT32PTR:
                kvp->value.pUint32_value = OSCL_STATIC_CAST(uint32*, myPtr);
                break;
            case PVMI_KVPVALTYPE_INT64PTR:
                kvp->value.pInt64_value = OSCL_STATIC_CAST(int64* , myPtr);
                break;
            case PVMI_KVPVALTYPE_UINT64PTR:
                kvp->value.pUint64_value = OSCL_STATIC_CAST(uint64* , myPtr);
                break;
            case PVMI_KVPVALTYPE_FLOATPTR:
                kvp->value.pFloat_value = OSCL_STATIC_CAST(float* , myPtr);
                break;
            case PVMI_KVPVALTYPE_DOUBLEPTR:
                kvp->value.pDouble_value = OSCL_STATIC_CAST(double*, myPtr);
                break;
            case PVMI_KVPVALTYPE_KSV:
                kvp->value.key_specific_value = OSCL_STATIC_CAST(OsclAny*, myPtr);
            default:
                break;
        }
        kvp->length = aValueSize;
        kvp->capacity = aValueSize;
    }
    else
    {
        kvp->length = 0;
        kvp->capacity = 0;

    }
    PvmiKvpSharedPtr sharedPtr(kvp, refCounter);
    return sharedPtr;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVID3ParCom::ConstructKvpKey(OSCL_String& aKey, PVID3FrameType aType, PVID3CharacterSet aCharSet)
{
    aKey = _STRLIT_CHAR("");

    switch (aType)
    {
        case PV_ID3_FRAME_TITLE:
            aKey += _STRLIT_CHAR(KVP_KEY_TITLE);
            break;
        case PV_ID3_FRAME_ARTIST:
            aKey += _STRLIT_CHAR(KVP_KEY_ARTIST);
            break;
        case PV_ID3_FRAME_PART_OF_SET:
            aKey += _STRLIT_CHAR(KVP_KEY_PART_OF_SET);
            break;
        case PV_ID3_FRAME_ALBUM:
            aKey += _STRLIT_CHAR(KVP_KEY_ALBUM);
            break;
        case PV_ID3_FRAME_COPYRIGHT:
            aKey += _STRLIT_CHAR(KVP_KEY_COPYRIGHT);
            break;
        case PV_ID3_FRAME_GENRE:
            aKey += _STRLIT_CHAR(KVP_KEY_GENRE);
            if (aCharSet == PV_ID3_CHARSET_INVALID)
            {
                // ID3 V1 genre is a uint32 value
                aKey += SEMI_COLON;
                aKey += _STRLIT_CHAR(PVMI_KVPVALTYPE_STRING);
                aKey += _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING);
                aKey += SEMI_COLON;
                aKey += _STRLIT_CHAR(KVP_FORMAT_ID3V1);
                return PVMFSuccess;
            }
            break;
        case PV_ID3_FRAME_TRACK_NUMBER:
            aKey += _STRLIT_CHAR(KVP_KEY_TRACK_NUMBER);
            if (aCharSet == PV_ID3_CHARSET_INVALID)
            {
                // ID3 V1 track number is a uint32 value
                aKey += SEMI_COLON;
                aKey += _STRLIT_CHAR(PVMI_KVPVALTYPE_STRING);
                aKey += _STRLIT_CHAR(PVMI_KVPVALTYPE_UINT32_STRING);
                return PVMFSuccess;
            }
            break;
        case PV_ID3_FRAME_YEAR:
            aKey += _STRLIT_CHAR(KVP_KEY_YEAR);
            break;
        case PV_ID3_FRAME_DATE:
            aKey += _STRLIT_CHAR(KVP_KEY_DATE);
            break;
        case PV_ID3_FRAME_RECORDING_TIME:
            aKey += _STRLIT_CHAR(KVP_KEY_RECORDING_TIME);
            break;
//new frames support
        case PV_ID3_FRAME_AUTHOR:
            aKey += _STRLIT_CHAR(KVP_KEY_AUTHOR);
            break;
        case PV_ID3_FRAME_COMPOSER:
            aKey += _STRLIT_CHAR(KVP_KEY_COMPOSER);
            break;
        case PV_ID3_FRAME_DESCRIPTION:
            aKey += _STRLIT_CHAR(KVP_KEY_DESCRIPTION);
            break;
        case PV_ID3_FRAME_VERSION:
            aKey += _STRLIT_CHAR(KVP_KEY_VERSION);
            break;
        case PV_ID3_FRAME_COMMENT:
            aKey += _STRLIT_CHAR(KVP_KEY_COMMENT);
            if ((iVersion != PV_ID3_V1_1) && (iVersion != PV_ID3_V1))
            {
                aKey += SEMI_COLON;
                aKey += KVP_VALTYPE_LYRICS;
            }
            break;
        case PV_ID3_FRAME_TRACK_LENGTH:
            aKey += _STRLIT_CHAR(KVP_KEY_ID3DURATION);
            aKey += SEMI_COLON;
            aKey += _STRLIT_CHAR(KVP_VALTYPE_DURATION);
            return PVMFSuccess; // No character set for track length
        case PV_ID3_FRAME_PIC:
        case PV_ID3_FRAME_APIC:
            aKey += _STRLIT_CHAR(KVP_KEY_ALBUMART);
            aKey += SEMI_COLON;
            return PVMFSuccess;
        case PV_ID3_FRAME_LYRICS:
            aKey += _STRLIT_CHAR(KVP_KEY_LYRICS);
            aKey += SEMI_COLON;
            aKey += KVP_VALTYPE_LYRICS;
            break;
        case PV_ID3_FRAME_UNRECOGNIZED:
        case PV_ID3_FRAME_CANDIDATE:
            aKey += _STRLIT_CHAR(KVP_ID3V2_VALUE);
            aKey += FORWARD_SLASH;
            aKey += _STRLIT_CHAR((char *)iID3TagInfo.iID3V2FrameID);
            aKey += SEMI_COLON;
            aKey += _STRLIT_CHAR(KVP_VALTYPE_UINT8PTR_STRING);
            return PVMFSuccess;
            break;
        default:
            return PVMFErrNotSupported;
    }


    //add language code to lyrics key
    if ((aType == PV_ID3_FRAME_LYRICS) || (aType == PV_ID3_FRAME_COMMENT))
    {
        //not valid for id3 v1
        if ((iVersion != PV_ID3_V1_1) && (iVersion != PV_ID3_V1))
        {
            aKey += SEMI_COLON;
            aKey += _STRLIT_CHAR(KVP_PARAM_LANGUAGE_CODE);
            aKey += _STRLIT_CHAR((char*)iID3TagInfo.iID3V2LanguageID);
            return PVMFSuccess;
        }
    }
    aKey += SEMI_COLON;
    switch (aCharSet)
    {
        case PV_ID3_CHARSET_ISO88591:
            aKey += _STRLIT_CHAR(KVP_VALTYPE_ISO88591_CHAR);
            break;
        case PV_ID3_CHARSET_UTF16:
            aKey += _STRLIT_CHAR(KVP_VALTYPE_UTF16_WCHAR);
            break;
        case PV_ID3_CHARSET_UTF16BE:
            aKey += _STRLIT_CHAR(KVP_VALTYPE_UTF16BE_WCHAR);
            break;
        case PV_ID3_CHARSET_UTF8:
            aKey += _STRLIT_CHAR(KVP_VALTYPE_UTF8_CHAR);
            break;
        default:
            return PVMFErrNotSupported;
    }

    aKey += NULL_CHARACTER;

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVID3ParCom::ReadStringValueFrame(PVID3FrameType aFrameType, PVID3CharacterSet aCharSet, uint32 aValueSize)
{
    // Construct key string
    OSCL_StackString<128> keyStr;
    if (ConstructKvpKey(keyStr, aFrameType, aCharSet) != PVMFSuccess)
    {
        LOG_ERR((0, "PVID3ParCom::ReadStringValueFrame: Error - ConstructKvpKey failed"));
        return PVMFErrNotSupported;
    }

    // Allocate key-value pair
    int32 err = OsclErrNone;
    PvmiKvpSharedPtr kvpPtr;
    bool truncate = false;
    switch (aCharSet)
    {
        case PV_ID3_CHARSET_ISO88591:
        case PV_ID3_CHARSET_UTF8:
            OSCL_TRY(err, kvpPtr = AllocateKvp(keyStr, PVMI_KVPVALTYPE_CHARPTR, aValueSize + 1, truncate););
            if (OsclErrNone != err || !kvpPtr)
            {
                LOG_ERR((0, "PVID3ParCom::ReadStringValueFrame: Error - AllocateKvp failed. err=%d", err));
                return PVMFErrNoMemory;
            }

            if (truncate)
            {
                iInputFile->Seek(aValueSize, Oscl_File::SEEKCUR);
                kvpPtr->length = 0;
            }
            else
            {
                if (readByteData(iInputFile, aValueSize, (uint8*)kvpPtr->value.pChar_value) == false)
                {
                    LOG_ERR((0, "PVID3ParCom::ReadStringValueFrame: Error - readByteData failed"));
                    return PVMFFailure;
                }

                int32 valueLen = aValueSize;
                valueLen = oscl_strlen((char*)kvpPtr->value.pChar_value);

                kvpPtr->value.pChar_value[valueLen] = NULL_TERM_CHAR;
                kvpPtr->length = valueLen + 1;
            }
            // Add to frame vector
            OSCL_TRY(err, iFrames.push_back(kvpPtr););
            OSCL_FIRST_CATCH_ANY(err,
                                 LOG_ERR((0, "PVID3ParCom::ReadStringValueFrame: Error - iFrames.push_back failed. err=%d", err));
                                 return PVMFErrNoMemory;
                                );
            break;

        case PV_ID3_CHARSET_UTF16:
        case PV_ID3_CHARSET_UTF16BE:
        {
            // create buffers to store frame data
            uint8* ptrFrameData = NULL;
            ptrFrameData = (uint8*) AllocateValueArray(err, PVMI_KVPVALTYPE_UINT8PTR, aValueSize + 2, &iAlloc);
            if (OsclErrNone != err || !ptrFrameData)
            {
                LOG_ERR((0, "PVID3ParCom::ReadStringValueFrame: Error - frameData allocation failed"));
                return PVMFErrNoMemory;
            }
            oscl_memset(ptrFrameData, 0, aValueSize + 2);
            uint32 wchar_size = sizeof(oscl_wchar);	//for platforms where wchar is 4 bytes.
            // Allocate key-value pair
            OSCL_TRY(err, kvpPtr = AllocateKvp(keyStr, PVMI_KVPVALTYPE_WCHARPTR, (wchar_size / 2) * (aValueSize + 2), truncate););
            if (OsclErrNone != err || !kvpPtr)
            {
                LOG_ERR((0, "PVID3ParCom::ReadStringValueFrame: Error - AllocateKvp failed. err=%d", err));
                return PVMFErrNoMemory;
            }

            if (truncate)
            {
                iAlloc.deallocate(ptrFrameData);
                iInputFile->Seek(aValueSize, Oscl_File::SEEKCUR);
                kvpPtr->length = 0;
            }
            else
            {
                if (readByteData(iInputFile, aValueSize, ptrFrameData) == false)
                {
                    iAlloc.deallocate(ptrFrameData);
                    LOG_ERR((0, "PVID3ParCom::ReadStringValueFrame: Error - readByteData failed"));
                    return PVMFFailure;
                }
                ptrFrameData[aValueSize] = 0;
                ptrFrameData[aValueSize+1] = 0;

                uint32 endianType = UNICODE_LITTLE_ENDIAN;
                if (aCharSet == PV_ID3_CHARSET_UTF16BE)
                {
                    endianType = UNICODE_BIG_ENDIAN;
                }

                uint32 wcSize = EightBitToWideCharBufferTransfer(ptrFrameData, aValueSize, endianType, kvpPtr->value.pWChar_value);
                kvpPtr->value.pWChar_value[wcSize] = NULL_TERM_CHAR;
                iAlloc.deallocate(ptrFrameData);
                kvpPtr->length = wcSize;
            }
            OSCL_TRY(err, iFrames.push_back(kvpPtr););
            OSCL_FIRST_CATCH_ANY(err,
                                 LOG_ERR((0, "PVID3ParCom::ReadStringValueFrame: Error - iFrame.push_back failed"));
                                 iAlloc.deallocate((OsclAny*)ptrFrameData);
                                 return PVMFErrNoMemory;
                                );
        }
        break;
        default:
            return PVMFFailure;
    }

    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVID3ParCom::ReadFrame(PVID3FrameType aFrameType, uint32 aValueSize)
{

    // Construct key string
    OSCL_StackString<128> keyStr;
    if (ConstructKvpKey(keyStr, aFrameType, PV_ID3_CHARSET_INVALID) != PVMFSuccess)
    {
        LOG_ERR((0, "PVID3ParCom::ReadFrame: Error - ConstructKvpKey failed"));
        return PVMFErrNotSupported;
    }

    // Allocate key-value pair
    int32 err = OsclErrNone;
    PvmiKvpSharedPtr kvpPtr;
    bool truncate = false;
    OSCL_TRY(err, kvpPtr = AllocateKvp(keyStr, PVMI_KVPVALTYPE_CHARPTR, aValueSize + 1 + VERSION_SIZE, truncate););
    if (OsclErrNone != err || !kvpPtr)
    {
        LOG_ERR((0, "PVID3ParCom::ReadFrame: Error - AllocateKvp failed. err=%d", err));
        return PVMFErrNoMemory;
    }

    if (truncate)
    {
        iInputFile->Seek(aValueSize, Oscl_File::SEEKCUR);
        kvpPtr->length = 0;
    }
    else
    {
        kvpPtr->value.pUint8_value[0] = (uint8)iVersion;
        kvpPtr->value.pUint8_value[1] = 0;

        if (readByteData(iInputFile, aValueSize, (uint8*)kvpPtr->value.pUint8_value + VERSION_SIZE) == false)
        {
            LOG_ERR((0, "PVID3ParCom::ReadFrame: Error - readByteData failed"));
            return PVMFFailure;
        }
        kvpPtr->value.pUint8_value[aValueSize+VERSION_SIZE] = 0;
        kvpPtr->length = aValueSize + VERSION_SIZE;
    }
    // Add to frame vector
    OSCL_TRY(err, iFrames.push_back(kvpPtr););
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVID3ParCom::ReadFrame: Error - iFrames.push_back failed. err=%d", err));
                         return PVMFErrNoMemory;
                        );


    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////

// Read in a NULL terminated UNICODE string byte by byte and take most significant byte first
PVMFStatus
PVID3ParCom::readNullTerminatedUnicodeString(PVFile* aInputFile, OSCL_wHeapString<OsclMemAllocator> &aData, uint32 &bomSz)
{
    uint8 buff[ID3_MAX_STRING_FRAME_LEN];
    uint32 index = 0;
    uint16 endianCheck;
    if (read16(aInputFile, endianCheck) == false)
    {
        return PVMFFailure;
    }

    iID3TagInfo.iTextType = PV_ID3_CHARSET_UTF16;
    uint32 endianType = 0;

    //some id3 tools does not add BOM with null strings.
    if (endianCheck != 0)
    {
        bomSz = UNICODE_BOM_SIZE;
        // This frame's text strings are Unicode and the frame
        // does include a BOM value. (UTF-16)
        if (endianCheck == UNICODE_LITTLE_ENDIAN_INDICATOR_FULL)
        {
            endianType = UNICODE_LITTLE_ENDIAN;
        }
        else if (endianCheck == UNICODE_BIG_ENDIAN_INDICATOR_FULL)
        {
            endianType = UNICODE_BIG_ENDIAN;
        }
        //read frame data from file
        bool more = true;
        while (more && (index < ID3_MAX_STRING_FRAME_LEN))
        {
            if (read8(iInputFile, buff[index]) == false)
                return PVMFFailure;

            if ((index > 0) && (buff[index] == 0) && (buff[index -1] == 0))
                more = false;
            index++;
        }

        //read remaining characters if any.
        if (more)
        {
            uint16 temp = 1;
            while (temp != 0)
                if (read16(iInputFile, temp) == false)
                    return PVMFFailure;
        }

    }
    else
    {
        buff[0] = 0;
        buff[1] = 0;
        bomSz = 0;
        index = 2;
    }

    int32 err = OsclErrNone;
    oscl_wchar *tmpData = NULL;
    tmpData = (oscl_wchar*) AllocateValueArray(err, PVMI_KVPVALTYPE_WCHARPTR, index);
    if (OsclErrNone != err || !tmpData)
    {
        LOG_ERR((0, "PVID3ParCom::readNullTerminatedUnicodeString: Error - allocation failed"));
        return PVMFErrNoMemory;
    }

    uint32 wcSize = EightBitToWideCharBufferTransfer(buff, index, endianType, tmpData);
    tmpData[wcSize] = 0;
    aData = tmpData;
    OSCL_ARRAY_DELETE(tmpData);
    return PVMFSuccess;
}

bool
PVID3ParCom::readNullTerminatedAsciiString(PVFile* aInputFile, OSCL_HeapString<OsclMemAllocator> &aData)
{
    uint8 buf[256];
    int32 index = 0;

    if (!read8(aInputFile, buf[index]))
        return PVMFFailure;

    bool nextChar = (buf[index] == 0) ? false : true;

    while (nextChar && (index < 256))
    {
        index++;

        if (!read8(aInputFile, buf[index]))
            return PVMFFailure;

        nextChar = (buf[index] == 0) ? false : true;
    }

    OSCL_HeapString<OsclMemAllocator> temp((const char *)buf);
    aData = temp;
    return true;
}

PVMFStatus PVID3ParCom::ReadAlbumArtFrame(PVID3FrameType aFrameType, uint8 unicode, uint32 aFrameSize)
{
    PvmfApicStruct* aApicStruct = NULL;
    uint8	picType;
    uint32	dataLen;
    OSCL_HeapString<OsclMemAllocator> ImageFormat;
    bool pic_as_url = false;
    bool truncate = false;
    int32 err = OsclErrNone;
    PvmiKvpSharedPtr kvpPtr;
    OSCL_StackString<128> keyStr;

    if (ConstructKvpKey(keyStr, PV_ID3_FRAME_APIC, PV_ID3_CHARSET_INVALID) != PVMFSuccess)
    {
        LOG_ERR((0, "PVID3ParCom::ReadAlbumArtFrame: Error - ConstructKvpKey failed"));
        return PVMFErrNotSupported;
    }

    switch (unicode)
    {
        case PV_ID3_CHARSET_ISO88591:
        case PV_ID3_CHARSET_UTF8:
        {
            OSCL_HeapString<OsclMemAllocator> description;

            if (GetAlbumArtInfo(aFrameType, aFrameSize, ImageFormat, picType, description, dataLen) != PVMFSuccess)
                return PVMFFailure;

            if (!oscl_strcmp(ImageFormat.get_cstr(), "-->"))
            {
                pic_as_url = true;
                break;
            }

            keyStr += KVP_FORMAT_ALBUMART;
            keyStr += SEMI_COLON;
            keyStr += KVP_VALTYPE_ALBUMART;

            //description and image format are stored as wchar.
            uint32 rfs = aFrameSize - (ImageFormat.get_size() + description.get_size());
            uint32 wchar_size = sizeof(oscl_wchar); //for platforms that store wchar as 4 bytes.
            uint32 total_size = sizeof(PvmfApicStruct) + rfs + ((wchar_size) * (ImageFormat.get_size() + description.get_size())) + 2 * sizeof(oscl_wchar) ;

            // Allocate key-value pair
            OSCL_TRY(err, kvpPtr = AllocateKvp(keyStr, PVMI_KVPVALTYPE_KSV, total_size, truncate););
            if (OsclErrNone != err || !kvpPtr)
            {
                LOG_ERR((0, "PVID3ParCom::ReadAlbumArtFrame: Error - AllocateKvp failed. err=%d", err));
                return PVMFFailure;
            }

            if (truncate)
            {
                iInputFile->Seek(dataLen, Oscl_File::SEEKCUR);
                kvpPtr->capacity = 0;
                kvpPtr->length = 0;

                OSCL_TRY(err, iFrames.push_back(kvpPtr););
                OSCL_FIRST_CATCH_ANY(err,
                                     LOG_ERR((0, "PVID3ParCom::ReadLyricsCommFrame: Error - iFrame.push_back failed"));
                                     return PVMFErrNoMemory;);

                return PVMFSuccess;

            }

            kvpPtr->capacity = total_size;
            kvpPtr->length = total_size;

            //convert raw data to struct
            PVMFStatus status;
            status = ConvertAsciiDataToApic(ImageFormat.get_str(), description.get_str(), picType,
                                            dataLen, kvpPtr->value.key_specific_value, total_size);
        }
        break;


        case PV_ID3_CHARSET_UTF16:
        case PV_ID3_CHARSET_UTF16BE:
        {
            OSCL_wHeapString<OsclMemAllocator> description;
            if (GetAlbumArtInfo(aFrameType, aFrameSize, ImageFormat, picType, description, dataLen) != PVMFSuccess)
                return PVMFFailure;

            if (!oscl_strcmp(ImageFormat.get_cstr(), "-->"))
            {
                pic_as_url = true;
                break;
            }

            keyStr += KVP_FORMAT_ALBUMART;
            keyStr += SEMI_COLON;
            keyStr += KVP_VALTYPE_ALBUMART;

            //image format is stored as wchar.
            uint32 rfs = aFrameSize - (ImageFormat.get_size() + (2 * oscl_strlen(description.get_str())));
            uint32 wchar_size = sizeof(oscl_wchar); //for platforms that store wchar as 4 bytes.
            uint32 total_size = sizeof(PvmfApicStruct) + rfs + (wchar_size * (ImageFormat.get_size() + description.get_size())) + (2 * wchar_size);

            // Allocate key-value pair
            OSCL_TRY(err, kvpPtr = AllocateKvp(keyStr, PVMI_KVPVALTYPE_KSV, total_size, truncate););
            if (OsclErrNone != err || !kvpPtr)
            {
                LOG_ERR((0, "PVID3ParCom::ReadAlbumArtFrame: Error - AllocateKvp failed. err=%d", err));
                return PVMFFailure;
            }

            if (truncate)
            {
                iInputFile->Seek(dataLen, Oscl_File::SEEKCUR);
                kvpPtr->capacity = 0;
                kvpPtr->length = 0;

                OSCL_TRY(err, iFrames.push_back(kvpPtr););
                OSCL_FIRST_CATCH_ANY(err,
                                     LOG_ERR((0, "PVID3ParCom::ReadLyricsCommFrame: Error - iFrame.push_back failed"));
                                     return PVMFErrNoMemory;);

                return PVMFSuccess;

            }

            kvpPtr->capacity = total_size;
            kvpPtr->length = total_size;

            //convert raw data to struct
            PVMFStatus status;
            status = ConvertUnicodeDataToApic(ImageFormat.get_str(), description.get_str(), picType,
                                              dataLen , kvpPtr->value.key_specific_value, total_size);
        }
        break;
        default:
            return PVMFFailure;

    }

    if (pic_as_url)
    {

        keyStr += KVP_VALTYPE_ISO88591_CHAR;
        bool truncate = false;
        // Allocate key-value pair
        OSCL_TRY(err, kvpPtr = AllocateKvp(keyStr, PVMI_KVPVALTYPE_CHARPTR, dataLen, truncate););
        if (OsclErrNone != err || !kvpPtr)
        {
            LOG_ERR((0, "PVID3ParCom::ReadAlbumArtFrame: Error - AllocateKvp failed. err=%d", err));
            return PVMFFailure;
        }

        if (truncate)
        {
            iInputFile->Seek(dataLen, Oscl_File::SEEKCUR);
            kvpPtr->length = 0;

            OSCL_TRY(err, iFrames.push_back(kvpPtr););
            OSCL_FIRST_CATCH_ANY(err,
                                 LOG_ERR((0, "PVID3ParCom::ReadLyricsCommFrame: Error - iFrame.push_back failed"));
                                 return PVMFErrNoMemory;);

            return PVMFSuccess;

        }
        else
        {
            if (readByteData(iInputFile, dataLen, (uint8 *)kvpPtr->value.pChar_value) == false)
                return PVMFFailure;
            kvpPtr->value.pChar_value[dataLen] = NULL_TERM_CHAR;
            kvpPtr->length = dataLen;
        }
    }
    else
    {
        aApicStruct = OSCL_STATIC_CAST(PvmfApicStruct *, kvpPtr->value.key_specific_value);
        if (readByteData(iInputFile, dataLen, aApicStruct->iGraphicData) == false)
            return PVMFFailure;
    }


    OSCL_TRY(err, iFrames.push_back(kvpPtr););
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVID3ParCom::ReadAlbumArtFrame: Error - iFrame.push_back failed"));
                         return PVMFErrNoMemory;
                        );

    return PVMFSuccess;
}

PVMFStatus PVID3ParCom::ReadLyricsCommFrame(uint8 unicodeCheck, uint32 aFramesize, PVID3FrameType aFrameType)
{

    uint32 framesize = aFramesize;
    bool truncate = false;
    if (readByteData(iInputFile, ID3V2_LANGUAGE_SIZE, iID3TagInfo.iID3V2LanguageID) == false)
    {
        return PVMFFailure;
    }

    iID3TagInfo.iID3V2LanguageID[ID3V2_LANGUAGE_SIZE] = 0;

    //subtract 3 bytes of language from the frame size.
    framesize = aFramesize - ID3V2_LANGUAGE_SIZE ;
    if ((framesize == 0) || (framesize > aFramesize))
    {
        return PVMFFailure;
    }
    //construct kvp for lyrics frame
    OSCL_StackString<128> keyStr;
    if (ConstructKvpKey(keyStr, aFrameType, (PVID3ParCom::PVID3CharacterSet)unicodeCheck) != PVMFSuccess)
    {
        LOG_ERR((0, "PVID3ParCom::ReadStringValueFrame: Error - ConstructKvpKey failed"));
        return PVMFErrNotSupported;
    }

    int32 err = OsclErrNone;
    PvmiKvpSharedPtr kvpPtr;
    uint32 wchar_size = sizeof(oscl_wchar); //for platforms where wchar is 4 bytes.
    uint32 total_size = (wchar_size * framesize) + sizeof(PvmfLyricsCommStruct) + (2 * wchar_size) ;
    uint32 used_size = 0;
    // Allocate key-value pair
    OSCL_TRY(err, kvpPtr = AllocateKvp(keyStr, PVMI_KVPVALTYPE_KSV, total_size , truncate););
    if (OsclErrNone != err || !kvpPtr)
    {
        LOG_ERR((0, "PVID3ParCom::ReadLyricsCommFrame: Error - AllocateKvp failed. err=%d", err));
        return PVMFFailure;
    }

    if (truncate)
    {
        iInputFile->Seek(framesize, Oscl_File::SEEKCUR);
        kvpPtr->capacity = 0;
        kvpPtr->length = 0;

        OSCL_TRY(err, iFrames.push_back(kvpPtr););
        OSCL_FIRST_CATCH_ANY(err,
                             LOG_ERR((0, "PVID3ParCom::ReadLyricsCommFrame: Error - iFrame.push_back failed"));
                             return PVMFErrNoMemory;);

        return PVMFSuccess;

    }

    PvmfLyricsCommStruct *lcStruct = NULL;
    uint8 *ptr = NULL;
    lcStruct = OSCL_STATIC_CAST(PvmfLyricsCommStruct *, kvpPtr->value.key_specific_value);
    ptr = (uint8 *)kvpPtr->value.key_specific_value;
    if (!ptr)
    {
        return PVMFFailure;
    }

    ptr += sizeof(PvmfLyricsCommStruct);
    used_size += sizeof(PvmfLyricsCommStruct);

    switch (unicodeCheck)
    {
        case PV_ID3_CHARSET_ISO88591:
        case PV_ID3_CHARSET_UTF8:
        {
            OSCL_HeapString<OsclMemAllocator> descriptor;

            if (readNullTerminatedAsciiString(iInputFile, descriptor) == false)
            {
                iAlloc.deallocate(lcStruct);
                return PVMFFailure;
            }

            uint32 desc_sz = descriptor.get_size();
            framesize = framesize - desc_sz - 1 ; //subtract 1 for null character

            if ((framesize > aFramesize) || (framesize == 0))
            {
                lcStruct->iDescription = NULL;
                lcStruct->iData = NULL;
                break;
            }

            lcStruct->iDescription = OSCL_STATIC_CAST(oscl_wchar *, ptr);
            ptr += wchar_size * (desc_sz + 1); //1 for null char

            used_size += wchar_size * (desc_sz + 1);
            if (used_size > total_size)
                return PVMFErrOverflow;
            if (desc_sz > 0)
            {
                uint32 sz = desc_sz + 1;
                oscl_UTF8ToUnicode((const char *)descriptor.get_cstr(), descriptor.get_size(), lcStruct->iDescription,  sz);
                lcStruct->iDescription[sz] = 0;
            }
            else
            {
                lcStruct->iDescription[desc_sz] = 0;
            }

            uint8 *data = NULL;
            uint32 datasz = framesize + 1;
            OSCL_TRY(err, data = (uint8*)iAlloc.allocate(datasz););

            if (OsclErrNone != err || !(data))
            {
                LOG_ERR((0, "PVID3ParCom::ReadLyricsCommFrame: Error - allocation failed"));
                iInputFile->Seek(framesize, Oscl_File::SEEKCUR);

            }

            else
            {

                if ((readByteData(iInputFile, framesize, data) == false))
                {
                    iAlloc.deallocate(data);
                    return PVMFFailure;
                }

                lcStruct->iData = OSCL_STATIC_CAST(oscl_wchar *, ptr);
                used_size += datasz;

                if (used_size > total_size)
                    return PVMFErrOverflow;
                oscl_UTF8ToUnicode((const char *)data, framesize, lcStruct->iData,  datasz);
                lcStruct->iData[datasz] = 0;

                iAlloc.deallocate(data);
            }
        }

        break;

        case PV_ID3_CHARSET_UTF16:
        case PV_ID3_CHARSET_UTF16BE:
        {

            uint32 bomSize = 0;
            OSCL_wHeapString<OsclMemAllocator> description ;

            if (readNullTerminatedUnicodeString(iInputFile, description, bomSize) == false)
            {
                return PVMFFailure;
            }

            uint32 desc_len = oscl_strlen(description.get_str()) ;
            framesize = framesize - (2 * desc_len) - 2 - bomSize; // description(2 byte unicode in id3 tag), null char, bom
            if ((framesize > aFramesize) || (framesize == 0))
            {
                lcStruct->iDescription = NULL;
                lcStruct->iData = NULL;
                break;
            }

            lcStruct->iDescription = OSCL_STATIC_CAST(oscl_wchar *, ptr);
            ptr += wchar_size * (desc_len + 1); //1 for null char

            used_size += wchar_size * (desc_len + 1);
            if (used_size > total_size)
                return PVMFErrOverflow;

            if (desc_len > 0)
            {
                oscl_strncpy(lcStruct->iDescription, description.get_str(), desc_len);
                lcStruct->iDescription[desc_len + 1] = 0;
            }
            else
            {
                lcStruct->iDescription[desc_len] = 0;
            }


            uint16 endianCheck = 0;
            //check endian type
            if (read16(iInputFile, endianCheck) == false)
            {

                return PVMFFailure;
            }

            // This frame's text strings are Unicode and the frame
            // does include a BOM value. (UTF-16)
            iID3TagInfo.iTextType = PV_ID3_CHARSET_UTF16;
            uint32 endianType = 0;

            if (endianCheck == UNICODE_LITTLE_ENDIAN_INDICATOR_FULL)
            {
                endianType = UNICODE_LITTLE_ENDIAN;
            }
            else if (endianCheck == UNICODE_BIG_ENDIAN_INDICATOR_FULL)
            {
                endianType = UNICODE_BIG_ENDIAN;
            }

            framesize -= UNICODE_BOM_SIZE;
            if ((framesize > aFramesize) || (framesize == 0))
            {
                lcStruct->iDescription = NULL;
                lcStruct->iData = NULL;
                break;
            }

            uint8 *data = NULL;
            uint32 datasz = framesize + 2;
            int32 err = OsclErrNone;
            OSCL_TRY(err, data = (uint8*)iAlloc.allocate(datasz););
            if (OsclErrNone != err || !(data))
            {
                LOG_ERR((0, "PVID3ParCom::ReadLyricsCommFrame: Error - allocation failed"));
                truncate = true;
            }
            else
            {

                oscl_memset(data, 0, datasz);
                if ((readByteData(iInputFile, framesize, data) == false))
                {
                    iAlloc.deallocate(data);
                    return PVMFFailure;
                }
                lcStruct->iData = OSCL_STATIC_CAST(oscl_wchar *, ptr);

                used_size += wchar_size * (framesize / 2);  //UCS-2 unicode
                if (used_size > total_size)
                    return PVMFErrOverflow;

                uint32 sz = EightBitToWideCharBufferTransfer(data, framesize, endianType , lcStruct->iData);
                lcStruct->iData[sz] = 0;

                iAlloc.deallocate(data);
            }
        }
        break;

        default:
            return PVMFFailure;

    }

    kvpPtr->capacity = (aFramesize - ID3V2_LANGUAGE_SIZE) + sizeof(PvmfLyricsCommStruct) ;
    kvpPtr->length = (aFramesize - ID3V2_LANGUAGE_SIZE) + sizeof(PvmfLyricsCommStruct) ;

    OSCL_TRY(err, iFrames.push_back(kvpPtr););
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVID3ParCom::ReadLyricsCommFrame: Error - iFrame.push_back failed"));
                         return PVMFErrNoMemory;
                        );


    return PVMFSuccess;
}

PVMFStatus PVID3ParCom::ConvertAsciiDataToApic(char* aMimeTypeID3, char* aDescriptionID3,
        uint8 aGraphicType, uint32 aGraphicDataLen,
        void *key_specific_value, uint32 total_size)
{
    uint32 used_size = 0;
    uint8 *ptr = (uint8 *)key_specific_value;
    PvmfApicStruct* aAPICStruct = OSCL_STATIC_CAST(PvmfApicStruct*, key_specific_value);
    uint32 mime_string_size;
    uint32 description_size;
    mime_string_size = oscl_strlen(aMimeTypeID3) + 1;
    description_size = oscl_strlen(aDescriptionID3) + 1;
    ptr += sizeof(PvmfApicStruct);
    used_size += sizeof(PvmfApicStruct);
    if (used_size > total_size)
        return PVMFErrOverflow;
    aAPICStruct->iGraphicMimeType = OSCL_STATIC_CAST(oscl_wchar *, ptr);
    ptr += sizeof(oscl_wchar) * (mime_string_size);
    used_size += sizeof(oscl_wchar) * (mime_string_size);
    if (used_size > total_size)
        return PVMFErrOverflow;
    aAPICStruct->iGraphicDescription = OSCL_STATIC_CAST(oscl_wchar *, ptr);
    ptr += sizeof(oscl_wchar) * (description_size);
    used_size += sizeof(oscl_wchar) * (description_size);
    if (used_size > total_size)
        return PVMFErrOverflow;

    oscl_memset(aAPICStruct->iGraphicDescription, 0 , description_size);
    oscl_memset(aAPICStruct->iGraphicMimeType , 0 ,  mime_string_size);
    oscl_UTF8ToUnicode((const char *)aDescriptionID3, description_size , aAPICStruct->iGraphicDescription,  description_size);
    oscl_UTF8ToUnicode((const char *)aMimeTypeID3, mime_string_size, aAPICStruct->iGraphicMimeType, mime_string_size);

    //push the data to PvmfApicStruct
    aAPICStruct->iGraphicType = aGraphicType;

    aAPICStruct->iGraphicData = OSCL_STATIC_CAST(uint8 *, ptr);

    aAPICStruct->iGraphicDataLen = aGraphicDataLen;

    used_size += aGraphicDataLen;
    if (used_size > total_size)
        return PVMFErrOverflow;
    return PVMFSuccess;
}

PVMFStatus PVID3ParCom::ConvertUnicodeDataToApic(char* aMimeTypeID3, oscl_wchar* aDescriptionID3,
        uint8 aGraphicType, uint32 aGraphicDataLen,
        void *key_specific_value, uint32 total_size)
{
    uint32 used_size = 0;
    uint8 *ptr = (uint8 *)key_specific_value;
    PvmfApicStruct* aAPICStruct = OSCL_STATIC_CAST(PvmfApicStruct*, key_specific_value);
    uint32 mime_string_size;
    uint32 description_size;

    mime_string_size = oscl_strlen(aMimeTypeID3) + 1;
    description_size = oscl_strlen(aDescriptionID3) + 1;

    ptr += sizeof(PvmfApicStruct);
    used_size += sizeof(PvmfApicStruct);
    if (used_size > total_size)
        return PVMFErrOverflow;

    aAPICStruct->iGraphicMimeType = OSCL_STATIC_CAST(oscl_wchar *, ptr);
    ptr += sizeof(oscl_wchar) * mime_string_size;
    used_size += sizeof(oscl_wchar) * mime_string_size;
    if (used_size > total_size)
        return PVMFErrOverflow;
    aAPICStruct->iGraphicDescription = OSCL_STATIC_CAST(oscl_wchar *, ptr);
    ptr += sizeof(oscl_wchar) * description_size;
    used_size += sizeof(oscl_wchar) * description_size;
    if (used_size > total_size)
        return PVMFErrOverflow;
    //push the data to PvmfApicStruct
    oscl_memset(aAPICStruct->iGraphicDescription, '\0', sizeof(oscl_wchar)*description_size);
    oscl_memset(aAPICStruct->iGraphicMimeType , '\0', sizeof(oscl_wchar)*mime_string_size);
    oscl_UTF8ToUnicode((const char *)aMimeTypeID3, mime_string_size - 1, aAPICStruct->iGraphicMimeType, mime_string_size);
    aAPICStruct->iGraphicMimeType[mime_string_size] = 0;

    oscl_strncpy(aAPICStruct->iGraphicDescription, aDescriptionID3, description_size);
    aAPICStruct->iGraphicDescription[description_size] = 0;
    aAPICStruct->iGraphicType = aGraphicType;
    aAPICStruct->iGraphicDataLen = aGraphicDataLen;
    aAPICStruct->iGraphicData = OSCL_STATIC_CAST(uint8 *, ptr);
    used_size += aGraphicDataLen;
    if (used_size > total_size)
        return PVMFErrOverflow;

    return PVMFSuccess;
}

PVMFStatus PVID3ParCom::GetAlbumArtInfo(PVID3FrameType aFrameType, uint32 aFrameSize, OSCL_HeapString<OsclMemAllocator> &aImageFormat,
                                        uint8 &aPicType, OSCL_HeapString<OsclMemAllocator> &aDescription, 	uint32 &aDataLen)
{
    uint8 image_format[4] = {0};
    PVMFStatus status = 0;
    uint32 size = 0;
    uint32 currentfilepos = 0 ;
    if (aFrameType == PV_ID3_FRAME_PIC)
    {
        if (readByteData(iInputFile, ID3V2_IMAGE_FORMAT_SIZE, image_format) == false)
        {
            return PVMFFailure;
        }
        else
        {
            aImageFormat = (char *)image_format;
        }
        if (read8(iInputFile, aPicType) == false)
        {
            return PVMFFailure;
        }

        currentfilepos = iInputFile->Tell();
        //read description from file
        status = readNullTerminatedAsciiString(iInputFile, aDescription);
        if (status != PVMFSuccess)
        {
            iInputFile->Seek(currentfilepos, Oscl_File::SEEKSET);
            return PVMFFailure;
        }
        //get the graphic data len
        size = aDescription.get_size() + 1 ; // 1 byte for null char
        aDataLen = aFrameSize - (ID3V2_PIC_CONST_SIZE + size);
        if (aDataLen > aFrameSize)
        {
            iInputFile->Seek(currentfilepos, Oscl_File::SEEKSET);
            aDataLen = 0;
            return PVMFFailure;
        }



    }
    else if (aFrameType == PV_ID3_FRAME_APIC)
    {
        uint32 mimeSize;
        uint32 desSize;

        currentfilepos = iInputFile->Tell();
        status = readNullTerminatedAsciiString(iInputFile, aImageFormat);
        if (status != PVMFSuccess)
        {
            iInputFile->Seek(currentfilepos, Oscl_File::SEEKSET);
            return PVMFFailure;
        }
        if (read8(iInputFile, aPicType) == false)
        {
            return PVMFFailure;
        }
        currentfilepos = iInputFile->Tell();
        status = readNullTerminatedAsciiString(iInputFile, aDescription);
        if (status != PVMFSuccess)
        {
            iInputFile->Seek(currentfilepos, Oscl_File::SEEKSET);
            return status;
        }

        //get the graphic data len
        mimeSize = aImageFormat.get_size() + 1; //for null string
        desSize = aDescription.get_size() + 1; //for null string

        aDataLen = aFrameSize - (ID3V2_APIC_CONST_SIZE + mimeSize + desSize);

        if (aDataLen > aFrameSize)
        {
            iInputFile->Seek(currentfilepos, Oscl_File::SEEKSET);
            aDataLen = 0;
            return PVMFFailure;
        }


    }
    return PVMFSuccess;
}

PVMFStatus PVID3ParCom::GetAlbumArtInfo(PVID3FrameType aFrameType, uint32 aFrameSize, OSCL_HeapString<OsclMemAllocator> &aImageFormat,
                                        uint8 &aPicType, OSCL_wHeapString<OsclMemAllocator> &aDescription, uint32 &aDataLen)
{
    uint8 image_format[4] = {0};
    uint32 currentfilepos = 0;
    uint32 size = 0;
    if (aFrameType == PV_ID3_FRAME_PIC)
    {
        if (readByteData(iInputFile, ID3V2_IMAGE_FORMAT_SIZE, image_format) == false)
        {
            return PVMFFailure;
        }
        else
        {
            aImageFormat = (char *)image_format;
        }
        if (read8(iInputFile, aPicType) == false)
        {
            return PVMFFailure;
        }

        uint32 bomSz = 0;
        currentfilepos = iInputFile->Tell();
        //read description from file
        if (readNullTerminatedUnicodeString(iInputFile, aDescription, bomSz) == false)
        {
            iInputFile->Seek(currentfilepos, Oscl_File::SEEKSET);
            return PVMFFailure;
        }

        //get the graphic data len
        size = (2 * oscl_strlen(aDescription.get_str())) + bomSz + sizeof(oscl_wchar) * 1; //2 byte null character
        aDataLen = aFrameSize - (ID3V2_PIC_CONST_SIZE + size);
        if (aDataLen > aFrameSize)
        {
            iInputFile->Seek(currentfilepos, Oscl_File::SEEKSET);
            aDataLen = 0;
            return PVMFFailure;
        }


    }
    else if (aFrameType == PV_ID3_FRAME_APIC)
    {
        uint32 mimeSize;
        uint32 desSize;
        currentfilepos = iInputFile->Tell();
        //read the mime type information
        if (readNullTerminatedAsciiString(iInputFile, aImageFormat) == false)
        {
            iInputFile->Seek(currentfilepos, Oscl_File::SEEKSET);
            return PVMFFailure;
        }
        //read the picture type information
        if (read8(iInputFile, aPicType) == false)
        {
            return PVMFFailure;
        }
        uint32 bomSz = 0;
        currentfilepos = iInputFile->Tell();
        //read the null terminated description information
        if (readNullTerminatedUnicodeString(iInputFile, aDescription, bomSz) == false)
        {
            iInputFile->Seek(currentfilepos, Oscl_File::SEEKSET);
            return PVMFFailure;
        }
        //get the graphic data len
        mimeSize = aImageFormat.get_size() + 1;
        desSize = (2 * oscl_strlen(aDescription.get_str())) + bomSz + sizeof(oscl_wchar) * 1; //2 bytes for null character
        aDataLen = aFrameSize - (ID3V2_APIC_CONST_SIZE + mimeSize + desSize);
        if (aDataLen > aFrameSize)
        {
            iInputFile->Seek(currentfilepos, Oscl_File::SEEKSET);
            aDataLen = 0;
            return PVMFFailure;
        }



    }
    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVID3ParCom::ReadTrackLengthFrame(uint32 aValueSize, PVID3CharacterSet aCharSet)
{
    OSCL_StackString<128> keyStr;
    if (ConstructKvpKey(keyStr, PV_ID3_FRAME_TRACK_LENGTH, PV_ID3_CHARSET_INVALID) != PVMFSuccess)
    {
        LOG_ERR((0, "PVID3ParCom::ReadTrackLengthFrame: Error - ConstructKvpKey failed"));
        return PVMFErrNotSupported;
    }

    int32 err = OsclErrNone;
    uint8* ptrFrameData = NULL;
    ptrFrameData = (uint8*) AllocateValueArray(err, PVMI_KVPVALTYPE_UINT8PTR, aValueSize + 2, &iAlloc);
    if (OsclErrNone != err || !ptrFrameData)
    {
        LOG_ERR((0, "PVID3ParCom::ReadTrackLengthFrame: Error - ptrFrameData allocation failed"));
        return PVMFErrNoMemory;
    }

    if (readByteData(iInputFile, aValueSize, ptrFrameData) == false)
    {
        iAlloc.deallocate((OsclAny*)ptrFrameData);
        LOG_ERR((0, "PVID3ParCom::ReadTrackLengthFrame: Error - readByteData failed"));
        return PVMFFailure;
    }
    ptrFrameData[aValueSize] = 0;
    ptrFrameData[aValueSize+1] = 0;

    switch (aCharSet)
    {
        case PV_ID3_CHARSET_UTF16:
        case PV_ID3_CHARSET_UTF16BE:
        {
            //it uses 16-bit unicode 2.0 (ISO/IEC 10646-1:1993, UCS-2)

            char* tmpData = NULL;
            int len = aValueSize / UNICODE_CHAR_SIZE + 1;
            tmpData = (char*) AllocateValueArray(err, PVMI_KVPVALTYPE_CHARPTR, len, &iAlloc);

            if (sizeof(oscl_wchar) == UNICODE_CHAR_SIZE)
            {
                oscl_UnicodeToUTF8((oscl_wchar*)ptrFrameData, aValueSize / sizeof(oscl_wchar), tmpData, len);
            }
            else
            {
                oscl_wchar* tmpData2 = NULL;

                tmpData2 = (oscl_wchar*) AllocateValueArray(err, PVMI_KVPVALTYPE_WCHARPTR, len);
                // convert 2 byte unicode data  to 4 byte wchar data
                ExpandWChar2BytesTo4Bytes(tmpData2, (uint16*)ptrFrameData, len);

                oscl_UnicodeToUTF8(tmpData2, aValueSize / UNICODE_CHAR_SIZE, tmpData, len);

                OSCL_ARRAY_DELETE(tmpData2);
            }

            // copy the UTF8 string back to ptrFrameData
            oscl_strncpy((char*)ptrFrameData, tmpData, len);

            iAlloc.deallocate((OsclAny*)tmpData);
        }
        break;
        case PV_ID3_CHARSET_ISO88591:
        case PV_ID3_CHARSET_UTF8:
            break;
        default:
            return PVMFFailure;
    }

    // The ID3 Track Length is a numeric string in milliseconds.
    uint32 duration = 0;
    int32 numericStringLen = oscl_strlen((const char*)ptrFrameData);

    if (!PV_atoi((const char *)ptrFrameData, 'd', numericStringLen, duration))
    {
        uint8* ptr = ptrFrameData;
        bool invalid_frame = false;
        while (ptr - ptrFrameData < numericStringLen)
        {
            // if the charater read is a digit or decimal point
            // then truncate it to integer value
            if (!(((*ptr >= ZERO_CHARACTER_ASCII_VALUE) &&
                    (*ptr <= NINE_CHARACTER_ASCII_VALUE)) ||
                    (*ptr == PERIOD_CHARACTER_ASCII_VALUE) ||
                    (*ptr == COMMA_CHARACTER_ASCII_VALUE)))
            {
                invalid_frame = true;
                break;
            }
            else if (*ptr == PERIOD_CHARACTER_ASCII_VALUE ||
                     *ptr == COMMA_CHARACTER_ASCII_VALUE)
            {
                numericStringLen = ptr - ptrFrameData;
                break;
            }
            ptr++;
        }

        if (!invalid_frame)
        {
            numericStringLen = ptr - ptrFrameData;
        }
        else
        {
            iAlloc.deallocate((OsclAny*)ptrFrameData);
            numericStringLen = 0;
            return PVMFSuccess;
        }
    }

    // Allocate key-value pair
    bool truncate = false;
    PvmiKvpSharedPtr kvpPtr;
    PVMFStatus status = PVMFSuccess;
    kvpPtr = HandleErrorForKVPAllocation(keyStr, PVMI_KVPVALTYPE_UINT32, 1, truncate, status);
    if (PVMFSuccess != status || !kvpPtr)
    {
        iAlloc.deallocate((OsclAny*)ptrFrameData);
        return PVMFErrNoMemory;
    }

    kvpPtr->value.uint32_value = duration; // Track length in milliseconds
    kvpPtr->length = numericStringLen;

    iAlloc.deallocate((OsclAny*)ptrFrameData);

    OSCL_TRY(err, iFrames.push_back(kvpPtr););
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVID3ParCom::ReadTrackLengthFrame: Error - iFrame.push_back failed"));
                         return PVMFErrNoMemory;
                        );

    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVID3ParCom::GetKvpValueType(PvmiKvpSharedPtr aKvp,
                                        PvmiKvpValueType& aValueType,
                                        PVID3CharacterSet& aCharSet)
{
    aValueType = GetValTypeFromKeyString(aKvp->key);
    aCharSet = PV_ID3_CHARSET_INVALID;
    char* param = NULL;
    switch (aValueType)
    {
        case PVMI_KVPVALTYPE_CHARPTR:
            aCharSet = PV_ID3_CHARSET_ISO88591;
            if (pv_mime_string_parse_param(aKvp->key, (char*) KVP_PARAM_CHAR_ENCODING_UTF8, param) > 0)
                aCharSet = PV_ID3_CHARSET_UTF8;
            break;
        case PVMI_KVPVALTYPE_WCHARPTR:
            aCharSet = PV_ID3_CHARSET_UTF16;
            if (pv_mime_string_parse_param(aKvp->key, (char*) KVP_PARAM_CHAR_ENCODING_UTF16BE, param) > 0)
                aCharSet = PV_ID3_CHARSET_UTF16BE;
            break;
        case PVMI_KVPVALTYPE_KSV:
            // Comment field
            break;
        case PVMI_KVPVALTYPE_UNKNOWN:
            return PVMFFailure;
        default:
            break;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVID3ParCom::GetFrameTypeFromKvp(PvmiKvpSharedPtr aKvp,
        OSCL_String& aFrameID,
        PVID3FrameType& aFrameType)
{
    return GetFrameTypeFromKvp(*aKvp, aFrameID, aFrameType);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVID3ParCom::GetFrameTypeFromKvp(const PvmiKvp& aKvp,
        OSCL_String& aFrameID,
        PVID3FrameType& aFrameType)
{
    if (iVersion == PV_ID3_V2_2)
    {
        if (pv_mime_strcmp(aKvp.key, KVP_KEY_TITLE) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_TITLE);
            aFrameType = PV_ID3_FRAME_TITLE;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_ARTIST) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_ARTIST);
            aFrameType = PV_ID3_FRAME_ARTIST;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_PART_OF_SET) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_PART_OF_SET);
            aFrameType = PV_ID3_FRAME_PART_OF_SET;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_ALBUM) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_ALBUM);
            aFrameType = PV_ID3_FRAME_ALBUM;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_TRACK_NUMBER) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_TRACK_NUMBER);
            aFrameType = PV_ID3_FRAME_TRACK_NUMBER;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_GENRE) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_GENRE);
            aFrameType = PV_ID3_FRAME_GENRE;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_YEAR) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_YEAR);
            aFrameType = PV_ID3_FRAME_YEAR;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_COPYRIGHT) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_COPYRIGHT);
            aFrameType = PV_ID3_FRAME_COPYRIGHT;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_COMMENT) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_COMMENT);
            aFrameType = PV_ID3_FRAME_COMMENT;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_ID3DURATION) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_TRACK_LENGTH);
            aFrameType = PV_ID3_FRAME_TRACK_LENGTH;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_DATE) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_DATE);
            aFrameType = PV_ID3_FRAME_DATE;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_ALBUMART) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_ALBUMART);
            aFrameType = PV_ID3_FRAME_PIC;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_LYRICS) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_LYRICIST);
            aFrameType = PV_ID3_FRAME_LYRICS;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_AUTHOR) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_AUTHOR);
            aFrameType = PV_ID3_FRAME_AUTHOR;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_COMPOSER) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_COMPOSER);
            aFrameType = PV_ID3_FRAME_COMPOSER;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_DESCRIPTION) >= 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_DESCRIPTION);
            aFrameType = PV_ID3_FRAME_DESCRIPTION;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_VERSION) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3V2_2_FRAME_ID_VERSION);
            aFrameType = PV_ID3_FRAME_VERSION;
        }
        else
        {
            LOG_ERR((0, "PVID3ParCom::GetFrameTypeFromKvp: Error : Frame type not supported for composing"));
            return PVMFErrNotSupported;
        }
    }
    else
    {
        if (pv_mime_strcmp(aKvp.key, KVP_KEY_TITLE) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_TITLE);
            aFrameType = PV_ID3_FRAME_TITLE;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_ARTIST) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_ARTIST);
            aFrameType = PV_ID3_FRAME_ARTIST;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_PART_OF_SET) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_PART_OF_SET);
            aFrameType = PV_ID3_FRAME_PART_OF_SET;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_ALBUM) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_ALBUM);
            aFrameType = PV_ID3_FRAME_ALBUM;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_TRACK_NUMBER) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_TRACK_NUMBER);
            aFrameType = PV_ID3_FRAME_TRACK_NUMBER;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_GENRE) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_GENRE);
            aFrameType = PV_ID3_FRAME_GENRE;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_YEAR) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_YEAR);
            aFrameType = PV_ID3_FRAME_YEAR;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_COPYRIGHT) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_COPYRIGHT);
            aFrameType = PV_ID3_FRAME_COPYRIGHT;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_COMMENT) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_COMMENT);
            aFrameType = PV_ID3_FRAME_COMMENT;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_ID3DURATION) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_TRACK_LENGTH);
            aFrameType = PV_ID3_FRAME_TRACK_LENGTH;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_RECORDING_TIME) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_RECORDING_TIME);
            aFrameType = PV_ID3_FRAME_RECORDING_TIME;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_AUTHOR) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_AUTHOR);
            aFrameType = PV_ID3_FRAME_AUTHOR;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_COMPOSER) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_COMPOSER);
            aFrameType = PV_ID3_FRAME_COMPOSER;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_DESCRIPTION) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_DESCRIPTION);
            aFrameType = PV_ID3_FRAME_DESCRIPTION;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_VERSION) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_VERSION);
            aFrameType = PV_ID3_FRAME_VERSION;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_DATE) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_DATE);
            aFrameType = PV_ID3_FRAME_DATE;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_LYRICS) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_LYRICIST);
            aFrameType = PV_ID3_FRAME_LYRICS;
        }
        else if (pv_mime_strcmp(aKvp.key, KVP_KEY_ALBUMART) == 0)
        {
            aFrameID = _STRLIT_CHAR(ID3_FRAME_ID_ALBUMART);
            aFrameType = PV_ID3_FRAME_APIC;
        }
        else
        {
            LOG_ERR((0, "PVID3ParCom::GetFrameTypeFromKvp: Error : Frame type not supported for composing"));
            return PVMFErrNotSupported;
        }
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVID3ParCom::GetID3v2FrameDataSize(PvmiKvpSharedPtr aKvp,
        PvmiKvpValueType aValueType,
        PVID3CharacterSet aCharSet,
        uint32& aSize)
{
    aSize = 0;
    switch (aValueType)
    {
        case PVMI_KVPVALTYPE_CHARPTR:
            aSize += (BYTE_FOR_CHARACTERSET_FORMAT + oscl_strlen(aKvp->value.pChar_value) + 1);
            break;
        case PVMI_KVPVALTYPE_WCHARPTR:
            aSize += (BYTE_FOR_CHARACTERSET_FORMAT +
                      (oscl_strlen(aKvp->value.pWChar_value) + 1) * sizeof(oscl_wchar));
            break;
        case PVMI_KVPVALTYPE_KSV:
            // Comment field
            break;
        case PVMI_KVPVALTYPE_UINT32:
            uint32 value;
            value = aKvp->value.uint32_value;
            if (value == 0)
                return PVMFSuccess;
            while (value > 0)
            {
                value = value / 10;
                ++aSize;
            }
            break;
        default:
            return PVMFErrNotSupported;
    }

    // Add 1 byte for BOM value to indicate byte order
    if (aCharSet == PV_ID3_CHARSET_UTF16)
        ++aSize;

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVID3ParCom::ComposeID3v2Tag(OsclRefCounterMemFrag& aTag)
{
    uint8* memFragPtr;
    uint32 index = 0;
    uint32 composedTagSize = 0;
    uint32 frameDataSize = 0;
    uint32 i = 0;
    char bufTrackLength[16];
    oscl_wchar wbufTrackLength[16];
    oscl_memset(bufTrackLength, 0, 16);
    oscl_memset(wbufTrackLength, 0, 16);
    PvmiKvpValueType kvpValueType;
    PVID3CharacterSet charSet;
    PVMFStatus status = PVMFSuccess;

    // At least one frame should be present for a valid ID3 tag to be composed
    if (iFrames.empty())
    {
        LOG_ERR((0, "PVID3ParCom::ComposeID3v2Tag: Error - No Frames are available to add"));
        return PVMFFailure;
    }

    // Calculate the size of composed ID3tag
    composedTagSize = GetID3TagSize();
    if (composedTagSize == 0)
    {
        LOG_ERR((0, "PVID3ParCom::ComposeID3v2Tag: Error - GetID3TagSize failed"));
        return PVMFFailure;
    }

    //iUseMaxTagSize on but the max tag size set is smaller than the total tag size.
    if (iUseMaxTagSize && (iMaxTagSize < composedTagSize))
    {
        LOG_ERR((0, "PVID3ParCom::ComposeID3v2Tag: Error - Max tag size is smaller than size of composed tag"));
        return PVMFFailure;
    }

    //if maxsize is greater than tagsize and padding is ON, allocate memory equal to iMaxTagSize
    if (iUseMaxTagSize && iUsePadding)
        composedTagSize = iMaxTagSize;

    //if the fragment buffer size is not sufficient, allocate memory
    OsclMemoryFragment& my_memfrag = aTag.getMemFrag();
    if (composedTagSize > aTag.getMemFragSize())
    {
        OsclRefCounter* my_refcnt;

        // Create mem frag for ID3 tag
        OsclMemAllocDestructDealloc<uint8> my_alloc;
        uint aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterSA< OsclMemAllocDestructDealloc<uint8> >));
        uint8* myPtr = (uint8*) my_alloc.allocate(aligned_refcnt_size + composedTagSize);
        my_refcnt = OSCL_PLACEMENT_NEW(myPtr, OsclRefCounterSA< OsclMemAllocDestructDealloc<uint8> >(myPtr));
        myPtr += aligned_refcnt_size;

        my_memfrag.ptr = myPtr;
        my_memfrag.len = 0;

        OsclRefCounterMemFrag mynewRefCntMemFrag(my_memfrag, my_refcnt, composedTagSize);

        //frees the old memeory and make the RefCounterMemFrag point to newly allocated memory
        aTag = mynewRefCntMemFrag;
    }

    memFragPtr = (uint8*)aTag.getMemFragPtr();
    oscl_memset(memFragPtr, 0, composedTagSize);

    // Form the tag header and place it into the tag buffer
    oscl_memcpy(memFragPtr, ID3_V2_IDENTIFIER, ID3V2_TAG_NUM_BYTES_ID);
    index += ID3V2_TAG_NUM_BYTES_ID;

    // Store ID3 tag version
    memFragPtr[index++] = (uint8)iVersion;
    memFragPtr[index++] = 0;

    // ID3v2 flags
    memFragPtr[index++] = ID3V2_FLAGS;

    // Tag size (4 bytes)
    uint32 syncSafeInt = ConvertToSyncSafeInt(composedTagSize - 10); // Total Size of ID3 tag - 10 bytes for header
#if (OSCL_BYTE_ORDER_LITTLE_ENDIAN)
    memFragPtr[index++] = (uint8)((syncSafeInt & 0xFF000000) >> 24);
    memFragPtr[index++] = (uint8)((syncSafeInt & 0x00FF0000) >> 16);
    memFragPtr[index++] = (uint8)((syncSafeInt & 0x0000FF00) >> 8);
    memFragPtr[index++] = (uint8)(syncSafeInt & 0x000000FF);
#elif (OSCL_BYTE_ORDER_BIG_ENDIAN)
    uint32* sizePtr = (uint32*)(memFragPtr + index);
    *sizePtr = syncSafeInt;
    index += 4;
#else
#error "Byte order is not specified in OSCL"
#endif

    //iterate the frames and write into the tag buffer
    OSCL_StackString<4> frameID;
    PVID3FrameType frameType;
    for (i = 0; i < iFrames.size(); i++)
    {
        // 4 character Frame ID
        status = GetFrameTypeFromKvp(iFrames[i], frameID, frameType);

        //ignore unsupported frames.
        if ((status == PVMFErrNotSupported) || (frameType == PV_ID3_FRAME_LYRICS) ||
                (frameType == PV_ID3_FRAME_APIC) || (frameType == PV_ID3_FRAME_PIC) ||
                (frameType == PV_ID3_FRAME_COMMENT))
        {
            continue;
        }

        if ((status != PVMFSuccess))
        {
            LOG_ERR((0, "PVID3ParCom::ComposeID3v2Tag: Error - GetFrameTypeFromKvp failed. status=%d", status));
            return status;
        }
        if (iVersion == PV_ID3_V2_2)
        {
            oscl_memcpy(memFragPtr + index, frameID.get_cstr(), ID3V2_2_FRAME_NUM_BYTES_ID);
            index += ID3V2_2_FRAME_NUM_BYTES_ID;
        }
        else
        {
            oscl_memcpy(memFragPtr + index, frameID.get_cstr(), ID3V2_FRAME_NUM_BYTES_ID);
            index += ID3V2_FRAME_NUM_BYTES_ID;
        }
        status = GetKvpValueType(iFrames[i], kvpValueType, charSet);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVID3ParCom::ComposeID3v2Tag: Error - GetKvpValueType failed. status=%d", status));
            return status;
        }

        // Size of Frame (4 byte sync safe integer)
        status = GetID3v2FrameDataSize(iFrames[i], kvpValueType, charSet, frameDataSize);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVID3ParCom::ComposeID3v2Tag: Error - GetID3v2FrameDataSize failed. status=%d", status));
            return status;
        }
//		syncSafeInt = ConvertToSyncSafeInt(frameDataSize);
        syncSafeInt = frameDataSize;

        if (iVersion == PV_ID3_V2_2)
        {
#if (OSCL_BYTE_ORDER_LITTLE_ENDIAN)
            memFragPtr[index++] = (uint8)((syncSafeInt & 0x00FF0000) >> 16);
            memFragPtr[index++] = (uint8)((syncSafeInt & 0x0000FF00) >> 8);
            memFragPtr[index++] = (uint8)(syncSafeInt & 0x000000FF);
#elif (OSCL_BYTE_ORDER_BIG_ENDIAN)
            sizePtr = (uint32*)(memFragPtr + index);
            *sizePtr = syncSafeInt;
            index += 3;
#else
#error "Byte order not specified in OSCL"
#endif

        }
        else
        {
#if (OSCL_BYTE_ORDER_LITTLE_ENDIAN)
            memFragPtr[index++] = (uint8)((syncSafeInt & 0xFF000000) >> 24);
            memFragPtr[index++] = (uint8)((syncSafeInt & 0x00FF0000) >> 16);
            memFragPtr[index++] = (uint8)((syncSafeInt & 0x0000FF00) >> 8);
            memFragPtr[index++] = (uint8)(syncSafeInt & 0x000000FF);
#elif (OSCL_BYTE_ORDER_BIG_ENDIAN)
            sizePtr = (uint32*)(memFragPtr + index);
            *sizePtr = syncSafeInt;
            index += 4;
#else
#error "Byte order not specified in OSCL"
#endif

            //frame Flag for status messages
            memFragPtr[index++] = ID3V2_FLAGS;

            //frame flag for format description
            memFragPtr[index++] = ID3V2_FLAGS;
        }

        switch (kvpValueType)
        {
            case PVMI_KVPVALTYPE_CHARPTR:
                memFragPtr[index++] = (uint8) charSet;
                frameDataSize -= 2; // Minus 1 byte for the character set and 1 byte for NULL character
                oscl_memcpy(memFragPtr + index, iFrames[i]->value.pChar_value, frameDataSize);
                index += frameDataSize;
                memFragPtr[index++] = 0; // NULL terminate the string
                break;
            case PVMI_KVPVALTYPE_WCHARPTR:
                memFragPtr[index++] = (uint8)charSet;
                frameDataSize -= 3; // Minus 1 byte for the character set and 2 bytes for NULL character
                if (charSet == PV_ID3_CHARSET_UTF16)
                {
#if (OSCL_BYTE_ORDER_LITTLE_ENDIAN)
                    memFragPtr[index++] = UNICODE_LITTLE_ENDIAN_INDICATOR;
                    memFragPtr[index++] = UNICODE_BIG_ENDIAN_INDICATOR;
#elif (OSCL_BYTE_ORDER_BIG_ENDIAN)
                    memFragPtr[index++] = UNICODE_BIG_ENDIAN_INDICATOR;
                    memFragPtr[index++] = UNICODE_LITTLE_ENDIAN_INDICATOR;

#else
#error "Byte order not specified"
#endif
                    frameDataSize -= 2; // Minus 1 byte for byte order
                }
                oscl_memcpy(memFragPtr + index, iFrames[i]->value.pWChar_value, frameDataSize);
                index += frameDataSize;
                memFragPtr[index++] = 0; // NULL terminate the string
                memFragPtr[index++] = 0; // NULL terminate the string
                break;
            case PVMI_KVPVALTYPE_UINT32:
                // Track length frame
                oscl_snprintf((char*)(memFragPtr + index), frameDataSize, "%d\0", iFrames[i]->value.uint32_value);
                break;
            case PVMI_KVPVALTYPE_KSV:
                // Comment frame
                memFragPtr[index++] = (uint8)charSet;
                frameDataSize -= 1; // Minus 1 byte for the character set
                break;
            default:
                LOG_ERR((0, "PVID3ParCom::ComposeID3v2Tag: Error - Unsupported key-value pair value type"));
                return PVMFErrNotSupported;
        }

    }

    //if padding is on and the tag size is less than the maximum tag specified, do the padding
    if (iUseMaxTagSize && iUsePadding && (index < iMaxTagSize))
    {
        oscl_memset((OsclAny*)(memFragPtr + index), 0, (iMaxTagSize - index));
        my_memfrag.len = iMaxTagSize;
    }
    else
    {
        //save the filesize into the memory fragment
        my_memfrag.len = index;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVID3ParCom::PVID3FrameType PVID3ParCom::FrameValidatedID3V2_4(uint8* aFrameID)
{
    PVID3FrameType ID3V2FrameTypeReturnValue;

    if ((oscl_memcmp(aFrameID, ID3_FRAME_ID_ENCRYPTION, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_AUDIO_SEEK_POINT_INDEX, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_COMMERCIAL_FRAME, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_ENCRYPTION_REGISTRATION, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_EQUALIZATION2, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_EVENT_TIMING_CODES, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_GENERAL_ENCAPSULATED_OBJECT, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_GROUP_IDENTITY_REGISTRATION, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_LINKED_INFORMATION, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_MUSIC_CD_IDENTIFIER, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_MPEG_LOCATION_LOOKUP_TABLE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_OWNERSHIP_FRAME, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_PRIVATE_FRAME, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_PLAY_COUNTER, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_POPULARIMETER, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_POSITION_SYNCH_FRAME, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_RECOMMENDED_BUFFER_SIZE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_RELATIVE_VOLUME_ADJUSTMENT2, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_REVERB, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_SYNCHRONIZED_LYRICS_TEXT, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_SYNCHRONIZED_TEMPO_CODES, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_BPM, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_PLAYLIST_DELAY, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_FILE_TYPE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_CONTENT_GROUP_DESC, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_INITIAL_KEY, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_LANGUAGE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_MEDIA_TYPE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_ORIGINAL_ALBUM, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_ORIGINAL_FILENAME, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_ORIGINAL_LYRICIST, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_ORIGINAL_ARTIST, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_FILE_LICENSEE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_BAND, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_CONDUCTOR, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_MODIFIER, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_PART_OF_SET, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_PUBLISHER, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_INTERNET_RADIO_STATION_NAME, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_INTERNET_RADIO_STATION_OWNER, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_RECORDING_CODE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_SOFTWARE_SETTING_ENCODE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_UNIQUE_FILE_IDENTIFIER, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_TERMS_OF_USE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_COMMERCIAL_INFO, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_LEGAL_INFO, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_OFFICIAL_AUDIO_FILE_WEBPAGE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_OFFICIAL_ARTIST_WEBPAGE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_OFFICIAL_AUDIO__SOURCE_WEBPAGE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_OFFICIAL_RADIO_STATION_WEBPAGE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_PAYMENT, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_PUBLISHER_OFFICIAL_WEBPAGE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_USER_DEFINED_URL_LINK_FRAME, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_SIGNATURE_FRAME, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_ENCODING_TIME, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_ORIGINAL_RELEASE_TIME, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_RELEASE_TIME, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_TAGGING_TIME, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_INVOLVED_PEOPLE_LIST, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_MUSICIAN_CREDITS_LIST, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_MOOD, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_PRODUCED_NOTICE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_ALBUM_SORT_ORDER, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_PERFORMER_SORT_ORDER, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_TITLE_SORT_ORDER, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_SET_SUBTITLE, ID3V2_FRAME_NUM_BYTES_ID) == 0) ||
            (oscl_memcmp(aFrameID, ID3_FRAME_ID_USER_DEFINED_TEXT_INFO, ID3V2_FRAME_NUM_BYTES_ID) == 0)
       )
    {
        ID3V2FrameTypeReturnValue = PV_ID3_FRAME_UNRECOGNIZED;
    }
    // The frame ID made out of the characters capital A-Z and 0-9.
    else if (IS_POTENTIAL_FRAME_ID(aFrameID))
    {
        ID3V2FrameTypeReturnValue = PV_ID3_FRAME_CANDIDATE;
    }
    else
    {
        // Frame ID doesn't match with defined/possible frame Ids
        ID3V2FrameTypeReturnValue = PV_ID3_FRAME_INVALID;
    }
    return ID3V2FrameTypeReturnValue;
}

PVMFStatus PVID3ParCom::PushFrameToFrameVector(PvmiKvpSharedPtr& aFrame, PvmiKvpSharedPtrVector& aFrameVector)
{
    int32 err = OsclErrNone;
    OSCL_TRY(err, aFrameVector.push_back(aFrame););
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVID3ParCom::GetID3Frame: Error - aFrame.push_back failed"));
                         return PVMFErrNoMemory;);
    return PVMFSuccess;
}

OsclAny* PVID3ParCom::AllocateValueArray(int32& aLeaveCode, PvmiKvpValueType aValueType, int32 aNumElements, OsclMemAllocator* aMemAllocator)
{
    int32 leaveCode = OsclErrNone;
    OsclAny* buffer = NULL;
    switch (aValueType)
    {
        case PVMI_KVPVALTYPE_WCHARPTR:
            if (aMemAllocator)
            {
                OSCL_TRY(leaveCode,
                         buffer = (oscl_wchar*) aMemAllocator->ALLOCATE(aNumElements););
            }
            else
            {
                OSCL_TRY(leaveCode,
                         buffer = (oscl_wchar*) OSCL_ARRAY_NEW(oscl_wchar, aNumElements););
            }
            break;

        case PVMI_KVPVALTYPE_CHARPTR:
            if (aMemAllocator)
            {
                OSCL_TRY(leaveCode,
                         buffer = (char*) aMemAllocator->ALLOCATE(aNumElements););
            }
            else
            {
                OSCL_TRY(leaveCode,
                         buffer = (char*) OSCL_ARRAY_NEW(char, aNumElements););
            }
            break;
        case PVMI_KVPVALTYPE_UINT8PTR:
            if (aMemAllocator)
            {
                OSCL_TRY(leaveCode,
                         buffer = (uint8*) aMemAllocator->ALLOCATE(aNumElements););
            }
            else
            {
                OSCL_TRY(leaveCode,
                         buffer = (uint8*) OSCL_ARRAY_NEW(char, aNumElements););
            }
            break;
        default:
            break;
    }
    aLeaveCode = leaveCode;
    return buffer;
}

PvmiKvpSharedPtr PVID3ParCom::HandleErrorForKVPAllocation(OSCL_String& aKey, PvmiKvpValueType aValueType, uint32 aValueSize, bool &truncate, PVMFStatus &aStatus)
{
    PvmiKvpSharedPtr kvp;
    int32 err = OsclErrNone;
    aStatus = PVMFSuccess;
    OSCL_TRY(err, kvp = AllocateKvp(aKey, aValueType, aValueSize, truncate););
    if (OsclErrNone != err)
    {
        LOG_ERR((0, "PVID3ParCom::SetID3Frame: Error - AllocateKvp failed"));
        aStatus = PVMFErrNoMemory;
    }
    return kvp;
}
