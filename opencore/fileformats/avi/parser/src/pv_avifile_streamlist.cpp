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
#include "pv_avifile_streamlist.h"

PVAviFileStreamlist::PVAviFileStreamlist(PVFile *aFp, uint32 aStrListSz)
{
    iCodecSpecificHdrDataSize = 0;
    iStreamListSize = aStrListSz;
    ipCodecSpecificHdrData = NULL;
    iError = PV_AVI_FILE_PARSER_SUCCESS;
    uint32 bytesRead = 0;
    uint32 chunkType = 0;

    while (bytesRead < iStreamListSize)
    {
        if ((iError = PVAviFileParserUtils::ReadNextChunkType(aFp, chunkType)) != PV_AVI_FILE_PARSER_SUCCESS)
        {
            PVAVIFILE_LOGINFO((0, "PVAviFileStreamlist::PVAviFileStreamlist: Unsupported chunk"));

            if (PV_AVI_FILE_PARSER_UNSUPPORTED_CHUNK == iError)
            {
                uint32 chksz = 0;
                if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(aFp, chksz, true))
                {
                    PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Read Error"));
                    iError = PV_AVI_FILE_PARSER_READ_ERROR;
                    break;
                }
                aFp->Seek(chksz, Oscl_File::SEEKCUR);
                bytesRead += chksz + CHUNK_SIZE + CHUNK_SIZE; // data + chunk size + chunk type

                PVAVIFILE_LOGINFO((0, "PVAviFileStreamlist::PVAviFileStreamlist: Unsupported Chunk Skipped"));

                iError = PV_AVI_FILE_PARSER_SUCCESS;
                continue;
            }
            else
            {

                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Read Error"));
                iError = PV_AVI_FILE_PARSER_READ_ERROR;
                break;
            }
        }

        bytesRead += CHUNK_SIZE;

        if (STRH == chunkType)
        {
            PVAVIFILE_LOGINFO((0, "PVAviFileStreamlist::PVAviFileStreamlist: Found Stream Header"));

            uint32 aviStrhSize = 0;
            if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(aFp, aviStrhSize, true))
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Read Error"));
                iError =  PV_AVI_FILE_PARSER_READ_ERROR;
                break;
            }

            bytesRead += CHUNK_SIZE;

            if (bytesRead > iStreamListSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File & Byte Count mismatch"));
                iError =  PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;

            }

            if ((aviStrhSize <= 0) || (aviStrhSize > iStreamListSize))
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: stream header size greater than stream list size"));
                iError =  PV_AVI_FILE_PARSER_WRONG_SIZE;
                break;

            }

            if ((iError = ParseStreamHeader(aFp, aviStrhSize)) != PV_AVI_FILE_PARSER_SUCCESS)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: ParseStreamHeader function error"));
                break;
            }

            bytesRead += aviStrhSize;
            if (bytesRead > iStreamListSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Size & Byte Count Mismatch"));
                iError =  PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;

            }
        }
        else if (STRF == chunkType)
        {
            PVAVIFILE_LOGINFO((0, "PVAviFileStreamlist::PVAviFileStreamlist: Found Stream Format Header"));

            uint32 aviStrfSize = 0;
            if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(aFp, aviStrfSize, true))
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Read Error"));
                iError =  PV_AVI_FILE_PARSER_READ_ERROR;
                break;
            }

            bytesRead += CHUNK_SIZE;

            if (bytesRead > iStreamListSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Size & Byte Count Mismatch"));
                iError =  PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;

            }

            if ((aviStrfSize <= 0) || (aviStrfSize > iStreamListSize))
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: Stream Format Header Size Greater Than Stream List Size"));
                iError =  PV_AVI_FILE_PARSER_WRONG_SIZE;
                break;

            }

            if ((iError = ParseStreamFormat(aFp, aviStrfSize)) != PV_AVI_FILE_PARSER_SUCCESS)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: ParseStreamFormat returned error"));
                break;
            }

            bytesRead += aviStrfSize;
            if (bytesRead > iStreamListSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Size & Byte Count Mismatch"));
                iError =  PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;

            }
        }
        else if (STRD == chunkType)
        {
            if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(aFp, iCodecSpecificHdrDataSize, true))
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Read Error"));
                iError =  PV_AVI_FILE_PARSER_READ_ERROR;
                break;
            }

            bytesRead += CHUNK_SIZE;

            if ((iCodecSpecificHdrDataSize <= 0) || (iCodecSpecificHdrDataSize > iStreamListSize))
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Size & Byte Count Mismatch"));
                iError =  PV_AVI_FILE_PARSER_WRONG_SIZE;
                break;
            }

            ipCodecSpecificHdrData = NULL;
            ipCodecSpecificHdrData = (uint8*)oscl_malloc(iCodecSpecificHdrDataSize);
            if (!ipCodecSpecificHdrData)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: Unable to allocate memory."));
                iError = PV_AVI_FILE_PARSER_INSUFFICIENT_MEMORY;
                break;
            }
            if (0 == PVAviFileParserUtils::read8(aFp, ipCodecSpecificHdrData, iCodecSpecificHdrDataSize))
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Read Error"));
                iError =  PV_AVI_FILE_PARSER_READ_ERROR;
                break;
            }

            bytesRead += iCodecSpecificHdrDataSize;
            if (bytesRead > iStreamListSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Size & Byte Count Mismatch"));
                iError =  PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }
        }
        else if (STRN == chunkType)
        {
            PVAVIFILE_LOGINFO((0, "PVAviFileStreamlist::PVAviFileStreamlist: Found stream name"));

            uint32 strnSz = 0;
            if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(aFp, strnSz, true))
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Read Error"));
                iError =  PV_AVI_FILE_PARSER_READ_ERROR;
                break;
            }
            bytesRead += CHUNK_SIZE;
            if (strnSz >= MAX_STRN_SZ)
            {
                uint8* strn = (uint8*)oscl_malloc(strnSz);
                if (!strn)
                {
                    PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: Unable to allocate memory."));
                    iError = PV_AVI_FILE_PARSER_INSUFFICIENT_MEMORY;
                    break;
                }
                if (PVAviFileParserUtils::read8(aFp, strn, strnSz) == 0)
                {
                    PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Read Error"));
                    iError =  PV_AVI_FILE_PARSER_READ_ERROR;
                    break;
                }

                oscl_strncpy(iStreamName, (char*)strn, (MAX_STRN_SZ - 1));
                iStreamName[MAX_STRN_SZ - 1] = '\0';
                oscl_free(strn);
            }
            else
            {
                if (PVAviFileParserUtils::read8(aFp, (uint8*)iStreamName, strnSz) == 0)
                {
                    PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Read Error"));
                    iError =  PV_AVI_FILE_PARSER_READ_ERROR;
                    break;
                }
            }

            bytesRead += strnSz;
            if (bytesRead > iStreamListSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Size & Byte Count Mismatch"));
                iError =  PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }

        }
        else if (JUNK == chunkType)
        {
            PVAVIFILE_LOGINFO((0, "PVAviFileStreamlist::PVAviFileStreamlist: Skip Junk Data"));

            uint32 junkSize = 0;
            if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(aFp, junkSize, true))
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Read Error"));
                iError =  PV_AVI_FILE_PARSER_READ_ERROR;
                break;
            }

            bytesRead += CHUNK_SIZE;
            if (bytesRead > iStreamListSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Size & Byte Count Mismatch"));
                iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }

            if ((junkSize <= 0) || (junkSize > iStreamListSize))
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: Junk data size more than stream list size"));
                iError = PV_AVI_FILE_PARSER_WRONG_SIZE;
                break;
            }

            aFp->Seek(junkSize, Oscl_File::SEEKCUR);
            bytesRead += junkSize;
            if (bytesRead > iStreamListSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: File Size & Byte Count Mismatch"));
                iError =  PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }

        }
        else
        {
            PVAVIFILE_LOGERROR((0, "PVAviFileStreamlist::PVAviFileStreamlist: Unexpected chunk in stream list"));
            iError = PV_AVI_FILE_PARSER_WRONG_CHUNK;
            break;
        }


    } //while(bytesRead <= iStreamListSize)

}


PV_AVI_FILE_PARSER_ERROR_TYPE
PVAviFileStreamlist::ParseStreamHeader(PVFile *aFp, uint32 aHdrSize)
{

    uint32 bytesRead = 0;

    if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(aFp, iStreamHdr.iStreamTypeFCC))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    bytesRead += CHUNK_SIZE;

    if (0 == (PVAviFileParserUtils::read8(aFp, iStreamHdr.iStreamHandler, CHUNK_SIZE)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }


    bytesRead += CHUNK_SIZE;

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamHdr.iFlags)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    bytesRead += CHUNK_SIZE;

    if (iStreamHdr.iFlags != 0)
    {
        if (iStreamHdr.iFlags & AVISF_VIDEO_PALCHANGES)
            iStreamHdr.iIsVidPalChangeAvailable = true;

        if (iStreamHdr.iFlags & AVISF_DISABLED)
            iStreamHdr.iAVISF_Disabled = true;
    }

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read16(aFp, iStreamHdr.iPriority, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    bytesRead += sizeof(iStreamHdr.iPriority);

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read16(aFp, iStreamHdr.iLanguage, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    bytesRead += sizeof(iStreamHdr.iPriority);

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamHdr.iInitFrames, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    bytesRead += CHUNK_SIZE;

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamHdr.iScale, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    bytesRead += CHUNK_SIZE;

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamHdr.iRate, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    bytesRead += CHUNK_SIZE;

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamHdr.iStart, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    bytesRead += CHUNK_SIZE;

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamHdr.iLength, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    bytesRead += CHUNK_SIZE;

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamHdr.iSugBufSize, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    bytesRead += CHUNK_SIZE;

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamHdr.iQuality, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    bytesRead += CHUNK_SIZE;

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamHdr.iSampleSize, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    bytesRead += CHUNK_SIZE;

    if (bytesRead > aHdrSize)
    {
        return PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
    }
    else if (bytesRead == aHdrSize)
    {
        return PV_AVI_FILE_PARSER_SUCCESS;
    }
    else  // read rcFrame
    {
        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read16(aFp, iStreamHdr.ircFrame.left, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        else
        {
            bytesRead += sizeof(uint16);
        }

        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read16(aFp, iStreamHdr.ircFrame.top, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        else
        {
            bytesRead += sizeof(uint16);
        }

        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read16(aFp, iStreamHdr.ircFrame.right, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        else
        {
            bytesRead += sizeof(uint16);
        }

        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read16(aFp, iStreamHdr.ircFrame.bottom, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        else
        {
            bytesRead += sizeof(uint16);
        }
    }

    if (bytesRead != aHdrSize)
    {
        return PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
    }

    iStreamHdr.iSamplingRate = (OsclFloat)iStreamHdr.iRate / iStreamHdr.iScale;

    return  PV_AVI_FILE_PARSER_SUCCESS;
}


PV_AVI_FILE_PARSER_ERROR_TYPE
PVAviFileStreamlist::ParseStreamFormat(PVFile *aFp, uint32 aHdrSize)
{
    uint32 bytesRead = 0;
    switch (iStreamHdr.iStreamTypeFCC)
    {
        case AUDS:
        {
            iStreamFmt.iType = PV_2_AUDIO;
        }
        break;
        case VIDS:
        {
            iStreamFmt.iType = PV_2_VIDEO;
        }
        break;
        case MIDI:
        {
            iStreamFmt.iType = MIDI;
        }
        break;
        case TXTS:
        {
            iStreamFmt.iType = TEXT;
        }
        break;
        default:
        {
            return PV_AVI_FILE_PARSER_ERROR_STREAM_TYPE_UNKNOWN;
        }

    }
    ; //end switch

    if (iStreamFmt.iType == PV_2_AUDIO)
    {
        iStreamFmt.iAudWaveFormatEx.FormatTag = 0;
        iStreamFmt.iAudWaveFormatEx.Channels = 0;
        iStreamFmt.iAudWaveFormatEx.SamplesPerSec = 0;
        iStreamFmt.iAudWaveFormatEx.AvgBytesPerSec = 0;
        iStreamFmt.iAudWaveFormatEx.BlockAlign = 0;
        iStreamFmt.iAudWaveFormatEx.BitsPerSample = 0;
        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read16(aFp, iStreamFmt.iAudWaveFormatEx.FormatTag, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }

        bytesRead += sizeof(uint16);
        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read16(aFp, iStreamFmt.iAudWaveFormatEx.Channels, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }

        bytesRead += sizeof(uint16);
        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamFmt.iAudWaveFormatEx.SamplesPerSec, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }

        bytesRead += CHUNK_SIZE;
        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamFmt.iAudWaveFormatEx.AvgBytesPerSec, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }

        bytesRead += CHUNK_SIZE;
        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read16(aFp, iStreamFmt.iAudWaveFormatEx.BlockAlign, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }

        bytesRead += sizeof(uint16);
        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read16(aFp, iStreamFmt.iAudWaveFormatEx.BitsPerSample, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }

        bytesRead += sizeof(uint16);

        if (bytesRead < aHdrSize)
        {
            if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read16(aFp, iStreamFmt.iAudWaveFormatEx.Size, true)))
            {
                return PV_AVI_FILE_PARSER_READ_ERROR;
            }
        }
        else
        {
            iStreamFmt.iAudWaveFormatEx.Size = 0;
        }

    }
    else if (iStreamFmt.iType == PV_2_VIDEO)
    {
        iStreamFmt.iVidBitMapInfo.BmiHeader.BiSize = 0;
        iStreamFmt.iVidBitMapInfo.BmiHeader.BiHeight = 0;
        iStreamFmt.iVidBitMapInfo.BmiHeader.BiPlanes = 0;
        iStreamFmt.iVidBitMapInfo.BmiHeader.BiBitCount = 0;
        iStreamFmt.iVidBitMapInfo.BmiHeader.BiCompression = 0;
        iStreamFmt.iVidBitMapInfo.BmiHeader.BiSizeImage = 0;
        iStreamFmt.iVidBitMapInfo.BmiHeader.BiXPelsPerMeter = 0;
        iStreamFmt.iVidBitMapInfo.BmiHeader.BiYPelsPerMeter = 0;
        iStreamFmt.iVidBitMapInfo.BmiHeader.BiClrUsed = 0;
        iStreamFmt.iVidBitMapInfo.BmiHeader.BiClrImportant = 0;
        iStreamFmt.iVidBitMapInfo.BmiColorsCount = 0;
        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamFmt.iVidBitMapInfo.BmiHeader.BiSize, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        bytesRead += CHUNK_SIZE;

        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamFmt.iVidBitMapInfo.BmiHeader.BiWidth, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        bytesRead += CHUNK_SIZE;

        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamFmt.iVidBitMapInfo.BmiHeader.BiHeight, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        bytesRead += CHUNK_SIZE;

        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read16(aFp, iStreamFmt.iVidBitMapInfo.BmiHeader.BiPlanes, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        bytesRead += sizeof(uint16);

        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read16(aFp, iStreamFmt.iVidBitMapInfo.BmiHeader.BiBitCount, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        bytesRead += sizeof(uint16);

        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamFmt.iVidBitMapInfo.BmiHeader.BiCompression, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        bytesRead += CHUNK_SIZE;

        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamFmt.iVidBitMapInfo.BmiHeader.BiSizeImage, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        bytesRead += CHUNK_SIZE;

        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamFmt.iVidBitMapInfo.BmiHeader.BiXPelsPerMeter, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        bytesRead += CHUNK_SIZE;

        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamFmt.iVidBitMapInfo.BmiHeader.BiYPelsPerMeter, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        bytesRead += CHUNK_SIZE;

        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamFmt.iVidBitMapInfo.BmiHeader.BiClrUsed, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        bytesRead += CHUNK_SIZE;

        if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iStreamFmt.iVidBitMapInfo.BmiHeader.BiClrImportant, true)))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
        bytesRead += CHUNK_SIZE;

        //get the color table size
        uint32 colorTblSize = iStreamFmt.iVidBitMapInfo.BmiHeader.BiClrUsed;
        if (colorTblSize > MAX_COLOR_TABLE_SIZE)
        {
            return PV_AVI_FILE_PARSER_WRONG_SIZE;
        }
        if (bytesRead == aHdrSize)
        {
            return PV_AVI_FILE_PARSER_SUCCESS;
        }
        else
        {
            if (colorTblSize == 0)
            {
                switch (iStreamFmt.iVidBitMapInfo.BmiHeader.BiBitCount)
                {
                    case BIT_COUNT1:
                    {
                        colorTblSize = 2;
                    }
                    break;
                    case BIT_COUNT2:
                    {
                        colorTblSize = 4;
                    }
                    break;
                    case BIT_COUNT4:
                    {
                        colorTblSize = 16;
                    }
                    break;
                    case BIT_COUNT8:
                    {
                        colorTblSize = 256;
                    }
                    break;
                    case BIT_COUNT16:
                    case BIT_COUNT32:
                    {
                        if (BI_BITFIELDS == iStreamFmt.iVidBitMapInfo.BmiHeader.BiCompression)
                        {
                            colorTblSize = 3;
                        }
                        else if (BI_ALPHABITFIELDS == iStreamFmt.iVidBitMapInfo.BmiHeader.BiCompression)
                        {
                            colorTblSize = 4;
                        }
                        else if (BI_RGB == iStreamFmt.iVidBitMapInfo.BmiHeader.BiCompression)
                        {
                            colorTblSize = 0;
                        }

                    }
                    break;
                    case BIT_COUNT24:
                    {
                        colorTblSize = 0;
                    }
                    break;
                    default:
                        return PV_AVI_FILE_PARSER_WRONG_BIT_COUNT;
                        break;

                } //switch
            }

            iStreamFmt.iVidBitMapInfo.BmiColorsCount = colorTblSize;

            uint32 length = 1;
            //get color table entries
            for (uint32 ii = 0; ii < colorTblSize; ii++)
            {
                if (length != PVAviFileParserUtils::read8(aFp, &(iStreamFmt.iVidBitMapInfo.BmiColors[ii].Blue), length))
                {
                    return PV_AVI_FILE_PARSER_READ_ERROR;
                }
                if (length != PVAviFileParserUtils::read8(aFp, &(iStreamFmt.iVidBitMapInfo.BmiColors[ii].Green), length))
                {
                    return PV_AVI_FILE_PARSER_READ_ERROR;
                }
                if (length != PVAviFileParserUtils::read8(aFp, &(iStreamFmt.iVidBitMapInfo.BmiColors[ii].Red), length))
                {
                    return PV_AVI_FILE_PARSER_READ_ERROR;
                }
                if (length != PVAviFileParserUtils::read8(aFp, &(iStreamFmt.iVidBitMapInfo.BmiColors[ii].Reserved), length))
                {
                    return PV_AVI_FILE_PARSER_READ_ERROR;
                }

            }
        } //else
        bytesRead += colorTblSize * sizeof(RGBQuad);


    }

    if (bytesRead != aHdrSize)
    {
        return PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
    }

    return PV_AVI_FILE_PARSER_SUCCESS;
}

OSCL_HeapString<OsclMemAllocator> PVAviFileStreamlist::GetStreamMimeType()
{
    OSCL_HeapString<OsclMemAllocator> mime_str;

    if (AUDS == iStreamHdr.iStreamTypeFCC)
    {
        mime_str = "audio";
    }
    else if (VIDS == iStreamHdr.iStreamTypeFCC)
    {
        mime_str = "video";
    }
    else if (MIDS == iStreamHdr.iStreamTypeFCC)
    {
        mime_str = "midi";
    }
    else if (TXTS == iStreamHdr.iStreamTypeFCC)
    {
        mime_str = "text";
    }
    else
    {
        mime_str = "unknown";
    }

    return mime_str;
}

bool PVAviFileStreamlist::GetHandlerType(uint8* aHdlr, uint32& aSize)
{
    if (aSize < CHUNK_SIZE)
    {
        return false;
    }

    oscl_memcpy(aHdlr, iStreamHdr.iStreamHandler, CHUNK_SIZE);
    aSize = CHUNK_SIZE;
    return true;
}

uint32 PVAviFileStreamlist::GetBitsPerSample()
{
    if (PV_2_AUDIO == iStreamFmt.iType)
    {
        return (iStreamFmt.iAudWaveFormatEx).BitsPerSample;
    }
    else if (PV_2_VIDEO == iStreamFmt.iType)

    {
        return (iStreamFmt.iVidBitMapInfo).BmiHeader.BiBitCount;
    }
    else
    {
        return 0;
    }
}

uint32 PVAviFileStreamlist::GetAudioFormat()
{
    if (PV_2_AUDIO == iStreamFmt.iType)
    {
        return (iStreamFmt.iAudWaveFormatEx).FormatTag;
    }
    else
    {
        return WAVE_FORMAT_UNKNOWN;
    }
}

uint32 PVAviFileStreamlist::GetNumAudioChannels()
{
    if (PV_2_AUDIO == iStreamFmt.iType)
    {
        return (iStreamFmt.iAudWaveFormatEx).Channels;
    }
    else
    {
        return 0;
    }

}

uint32 PVAviFileStreamlist::GetVideoWidth()
{
    if (PV_2_VIDEO == iStreamFmt.iType)
    {
        return (iStreamFmt.iVidBitMapInfo).BmiHeader.BiWidth;
    }
    else
    {
        return 0;
    }
}

uint32 PVAviFileStreamlist::GetVideoHeight(bool& rBottomUp)
{
    if (PV_2_VIDEO == iStreamFmt.iType)
    {
        int32 height = (iStreamFmt.iVidBitMapInfo).BmiHeader.BiHeight;
        if (height < 0) //negative height
        {
            rBottomUp = false;
            height = oscl_abs(height);
        }
        else
        {
            rBottomUp = true;
        }

        return ((uint32)height);
    }
    else
    {
        return 0;
    }
}

bool PVAviFileStreamlist::GetFormatSpecificInfo(uint8*& aBuff, uint32& aSize)
{
    if (PV_2_AUDIO == iStreamFmt.iType)
    {
        aSize = sizeof(WaveFormatExStruct);
        aBuff = OSCL_STATIC_CAST(uint8*, &(iStreamFmt.iAudWaveFormatEx));
    }
    else if (PV_2_VIDEO == iStreamFmt.iType)
    {
        aSize = sizeof(BitmapInfoHhr) + (sizeof(RGBQuad) * iStreamFmt.iVidBitMapInfo.BmiColorsCount);
        aBuff = OSCL_STATIC_CAST(uint8*, &(iStreamFmt.iVidBitMapInfo));
    }
    else
    {
        return false;
    }

    return true;
}

bool PVAviFileStreamlist::GetCodecSpecificData(uint8*& aBuff, uint32& aSize)
{
    if (iCodecSpecificHdrDataSize > 0)
    {
        aBuff = ipCodecSpecificHdrData;
        aSize = iCodecSpecificHdrDataSize;
    }
    else
    {
        return false;
    }

    return true;
}


