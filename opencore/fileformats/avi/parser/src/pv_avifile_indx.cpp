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
#include "pv_avifile_indx.h"

PVAviFileIdxChunk::PVAviFileIdxChunk(PVFile* aFp, uint32 aIndxSize, uint32 aNumStreams)
{
    iIndexSize = aIndxSize;
    iNumStreams = aNumStreams;
    uint32 bytesRead = 0;
    int32 strNum = 0;
    uint32 data = 0;
    iError = PV_AVI_FILE_PARSER_SUCCESS;
    iOffsetFrmMoviLst = false;
    bool firstSample = true;

    for (uint32 ii = 0; ii < iNumStreams; ii++)
    {
        IndxTblVector  xtbl;
        iIndexTable.push_back(xtbl);
    }

    while (bytesRead < iIndexSize)
    {
        if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(aFp, data))
        {
            iError = PV_AVI_FILE_PARSER_READ_ERROR;
            break;
        }
        bytesRead += CHUNK_SIZE;
        if (bytesRead > iIndexSize)
        {
            iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
            break;
        }

        strNum = PVAviFileParserUtils::GetStreamNumber(data);

        if ((strNum < 0) || (strNum >= (int32)iNumStreams))
        {
            //bogus entry skip it.
            aFp->Seek((CHUNK_SIZE + CHUNK_SIZE + CHUNK_SIZE), Oscl_File::SEEKCUR);

            bytesRead += CHUNK_SIZE + CHUNK_SIZE + CHUNK_SIZE ; //flags+offset+size
            continue;
        }

        IdxTblType tbl;
        tbl.chunkId = data;

        if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(aFp, (tbl.flags)))
        {
            iError = PV_AVI_FILE_PARSER_READ_ERROR;
            break;
        }

        bytesRead += CHUNK_SIZE;
        if (bytesRead > iIndexSize)
        {
            iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
            break;
        }

        if (tbl.flags & AVIIF_KEYFRAME)
            tbl.isKeyFrame = true;
        if (tbl.flags & AVIIF_LIST)
            tbl.ifRecList = true;
        if (tbl.flags & AVIIF_NO_TIME)
            tbl.ifNoTime  = true;

        if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(aFp, tbl.offset, true))
        {
            iError = PV_AVI_FILE_PARSER_READ_ERROR;
            break;
        }

        bytesRead += CHUNK_SIZE;
        if (bytesRead > iIndexSize)
        {
            iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
            break;
        }

        if (firstSample)
        {
            firstSample = false;
            if (OFFSET_FROM_MOVI_LST == tbl.offset)
            {
                iOffsetFrmMoviLst = true;
            }
        }

        if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(aFp, tbl.size, true))
        {
            iError = PV_AVI_FILE_PARSER_READ_ERROR;
            break;
        }

        bytesRead += CHUNK_SIZE;
        if (bytesRead > iIndexSize)
        {
            iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
            break;
        }

        (iIndexTable[strNum]).push_back(tbl);

    }
}

