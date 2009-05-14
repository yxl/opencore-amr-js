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
#include "pv_avifile_header.h"

PVAviFileHeader::PVAviFileHeader(PVFile *aFp, uint32 aHdrSize)
{
    iHeaderTotalSize = aHdrSize;
    uint32 bytesRead = 0;
    uint32 chunkType = 0;
    uint32 oldChkType = 0;
    iError = PV_AVI_FILE_PARSER_SUCCESS;

    for (uint32 ii = 0; ii < iStreamList.size(); ii++)
    {
        iStreamList.pop_back();
    }

    uint32 streamListSz = 0;

    while (bytesRead < iHeaderTotalSize)
    {
        oldChkType = chunkType;
        if ((iError = PVAviFileParserUtils::ReadNextChunkType(aFp, chunkType)) != PV_AVI_FILE_PARSER_SUCCESS)
        {
            if ((PV_AVI_FILE_PARSER_UNSUPPORTED_CHUNK == iError))
            {
                PVAVIFILE_LOGINFO((0, "PVAviFileHeader::PVAviFileHeader: Unsupported chunk"));

                uint32 chksz = 0;
                if (oldChkType != LIST)
                {
                    //get the size of unsupported chunk and skip it.
                    if (PVAviFileParserUtils::read32(aFp, chksz, true) != PV_AVI_FILE_PARSER_SUCCESS)
                    {
                        PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: File Read Error"));
                        iError = PV_AVI_FILE_PARSER_READ_ERROR;
                        break;
                    }

                    aFp->Seek(chksz, Oscl_File::SEEKCUR);
                    bytesRead += chksz + CHUNK_SIZE + CHUNK_SIZE; //data + chunk size + chunk type
                }
                else
                {
                    //skip the entire list if not supported
                    aFp->Seek((streamListSz - CHUNK_SIZE), Oscl_File::SEEKCUR); //subtract list name read above
                    bytesRead += streamListSz;
                }

                PVAVIFILE_LOGINFO((0, "PVAviFileHeader::PVAviFileHeader: Unsupported chunk skipped"));
                iError = PV_AVI_FILE_PARSER_SUCCESS;
                continue;
            }
            else
            {
                break;
            }
        }

        bytesRead += CHUNK_SIZE;

        if (bytesRead > iHeaderTotalSize)
        {
            PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: File Size & Byte Count mismatch"));
            iError =  PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
            break;
        }

        if (AVIH == chunkType)
        {
            uint32 aviStrSize = 0;
            if (PVAviFileParserUtils::read32(aFp, aviStrSize, true) != PV_AVI_FILE_PARSER_SUCCESS)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: File Read Error"));
                iError = PV_AVI_FILE_PARSER_READ_ERROR;
                break;
            }

            bytesRead += CHUNK_SIZE;

            if (bytesRead > iHeaderTotalSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: File Size & Byte Count mismatch"));
                iError =  PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }

            if ((aviStrSize <= 0) || (aviStrSize > iHeaderTotalSize))
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: AVIH size greater than file header size"));
                iError =  PV_AVI_FILE_PARSER_WRONG_SIZE;
                break;
            }

            if ((iError = ParseMainHeader(aFp)) != PV_AVI_FILE_PARSER_SUCCESS)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: Error encountered while parsing File Header"));
                break;
            }

            bytesRead += aviStrSize;
            if (bytesRead > iHeaderTotalSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: File Size & Byte Count mismatch"));
                iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }
        }
        else if (LIST == chunkType)
        {
            if (PVAviFileParserUtils::read32(aFp, streamListSz, true) != PV_AVI_FILE_PARSER_SUCCESS)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: File Read Error"));
                iError =  PV_AVI_FILE_PARSER_READ_ERROR;
                break;
            }

            bytesRead += CHUNK_SIZE;

            if (bytesRead > iHeaderTotalSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: File Size & Byte Count mismatch"));
                iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }

            if ((streamListSz <= 0) || (streamListSz > iHeaderTotalSize))
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: stream list soze greater tha file header size"));
                iError = PV_AVI_FILE_PARSER_WRONG_SIZE;
                break;
            }

        }
        else if (STRL == chunkType)
        {
            PVAVIFILE_LOGINFO((0, "PVAviFileHeader::PVAviFileHeader: Found stream list"));

            PVAviFileStreamlist* strlst = OSCL_NEW(PVAviFileStreamlist, (aFp, (streamListSz - CHUNK_SIZE))); //subtract 4 bytes of List type from list size
            if (strlst != NULL)
            {
                if ((iError = strlst->GetStatus()) != PV_AVI_FILE_PARSER_SUCCESS)
                {
                    OSCL_DELETE(strlst);
                    strlst = NULL;
                    break;
                }
            }

            iStreamList.push_back(*strlst);
            bytesRead += streamListSz - CHUNK_SIZE;
            if (bytesRead > iHeaderTotalSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: File Size & Byte Count mismatch"));
                iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }

            OSCL_DELETE(strlst);
        }
        else if (JUNK == chunkType)
        {
            PVAVIFILE_LOGINFO((0, "PVAviFileParser::ParseFile: Skip Junk data"));

            uint32 junkSize = 0;
            if (PVAviFileParserUtils::read32(aFp, junkSize, true) != PV_AVI_FILE_PARSER_SUCCESS)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: File Read Error"));
                iError =  PV_AVI_FILE_PARSER_READ_ERROR;
                break;
            }

            bytesRead += CHUNK_SIZE;
            if (bytesRead > iHeaderTotalSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: File Size & Byte Count mismatch"));
                iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }

            if ((junkSize <= 0) || (junkSize > iHeaderTotalSize))
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: Junk data size more than file header size"));
                iError = PV_AVI_FILE_PARSER_WRONG_SIZE;
                break;
            }

            aFp->Seek(junkSize, Oscl_File::SEEKCUR);
            bytesRead += junkSize;
            if (bytesRead > iHeaderTotalSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: File Size & Byte Count mismatch"));
                iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }

        }
        else
        {
            PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: Chunk not supported in file main header"));
            iError = PV_AVI_FILE_PARSER_WRONG_CHUNK;
            break;
        }

    }

    if ((PV_AVI_FILE_PARSER_SUCCESS == iError) && (iStreamList.size() != iMainHeader.iStreams))
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileHeader::PVAviFileHeader: "));
        iError = PV_AVI_FILE_PARSER_ERROR_NUM_STREAM;
    }


}

PV_AVI_FILE_PARSER_ERROR_TYPE
PVAviFileHeader::ParseMainHeader(PVFile *aFp)
{
    //Read avi main header

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iMainHeader.iMicroSecPerFrame, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iMainHeader.iMaxBytesPerSec, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iMainHeader.iPadding, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iMainHeader.iFlags)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    if (iMainHeader.iFlags != 0)
    {
        if (iMainHeader.iFlags & AVIF_COPYRIGHTED)
            iMainHeader.iIsAVIFileCopyrighted = true;

        if (iMainHeader.iFlags & AVIF_HASINDEX)
            iMainHeader.iAVIFileHasIndxTbl = true;

        if (iMainHeader.iFlags & AVIF_ISINTERLEAVED)
            iMainHeader.iISAVIFileInterleaved = true;

        if (iMainHeader.iFlags & AVIF_MUSTUSEINDEX)
            iMainHeader.iAVIFileMustUseIndex = true;

        if (iMainHeader.iFlags & AVIF_WASCAPTUREFILE)
            iMainHeader.iAVIFileWasCaptureFile = true;
    }
    else
    {
        iMainHeader.iIsAVIFileCopyrighted = false;
        iMainHeader.iAVIFileHasIndxTbl = false;
        iMainHeader.iISAVIFileInterleaved = false;
        iMainHeader.iAVIFileMustUseIndex = false;
        iMainHeader.iAVIFileWasCaptureFile = false;
    }


    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iMainHeader.iTotalFrames, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;

    }

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iMainHeader.iInitialFrames, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;

    }

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iMainHeader.iStreams, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;

    }

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iMainHeader.iSuggestedBufferSize, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;

    }

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iMainHeader.iWidth, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;

    }

    if (PV_AVI_FILE_PARSER_SUCCESS != (PVAviFileParserUtils::read32(aFp, iMainHeader.iHeight, true)))
    {
        return PV_AVI_FILE_PARSER_READ_ERROR;

    }

    for (uint32 ii = 0; ii < RES_BYTES_SZ; ii++)
    {
        if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(aFp, iMainHeader.iReserved[ii], true))
        {
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }
    }
    return PV_AVI_FILE_PARSER_SUCCESS;
}


uint32 PVAviFileHeader::GetAudioFormat(uint32 aStreamNo)
{
    uint32 strnum = GetNumStreams();
    uint32 retType = WAVE_FORMAT_UNKNOWN;
    uint32 ii = 0;

    for (ii = 0; ii < strnum; ii++)
    {
        if ((iStreamList[ii].GetStreamType() == PV_2_AUDIO) && (aStreamNo == ii))
        {
            retType = iStreamList[ii].GetAudioFormat();
        }
    }

    return retType;
}


uint32 PVAviFileHeader::GetNumAudioChannels(uint32 aStreamNo)
{
    uint32 strnum = GetNumStreams();
    uint32 retType = 0;
    uint32 ii = 0;

    for (ii = 0; ii < strnum; ii++)
    {
        if ((iStreamList[ii].GetStreamType() == PV_2_AUDIO) && (ii == aStreamNo))
        {
            retType = iStreamList[ii].GetNumAudioChannels();
        }
    }
    return retType;

}

