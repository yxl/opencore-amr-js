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
#include "pv_avifile_parser.h"


PVAviFileParser::PVAviFileParser(OSCL_wHeapString<OsclMemAllocator> aFileName, int32& aError, Oscl_FileServer* aFileServer,
                                 PVMFCPMPluginAccessInterfaceFactory*  aCPM, OsclFileHandle*  aFileHandle)
{
    ipFileHeader = NULL;
    ipIdxChunk = NULL;
    iIdxChunkPresent = false;
    iFileSize = 0;
    iHeaderChunkSize = 0;
    iMovieChunkSize = 0;
    iMovieChunkStartOffset = 0;
    iIndxChunkSize = 0;
    iSampleOffset = 0;
    iTimeStampVideo = 0;
    iTimeStampAudio = 0;

    OSCL_TRY(aError, ipFilePtr = OSCL_NEW(PVFile, ()););

    if (0 == aError)
    {
        ipFilePtr->SetCPM(aCPM);
        ipFilePtr->SetFileHandle(aFileHandle);

        aError = PV_AVI_FILE_PARSER_SUCCESS;

        if (ipFilePtr->Open(aFileName.get_cstr(), (Oscl_File::MODE_READ | Oscl_File::MODE_BINARY), *aFileServer) != 0)
        {
            aError = PV_AVI_FILE_PARSER_FILE_OPEN_ERROR;
        }
    }
}

PV_AVI_FILE_PARSER_ERROR_TYPE
PVAviFileParser::ParseFile()
{
    iError = PV_AVI_FILE_PARSER_SUCCESS;
    ipFilePtr->Seek(0, Oscl_File::SEEKSET);

    // Seek to the end to find the file size
    uint32 filesize = 0;
    if (ipFilePtr->Seek(0, Oscl_File::SEEKEND))
    {
        iError = PV_AVI_FILE_PARSER_SEEK_ERROR;
        return iError;
    }
    filesize = ipFilePtr->Tell();

    // Seek back to the beginning
    ipFilePtr->Seek(0, Oscl_File::SEEKSET);
    uint32 chunkType = 0;

    if ((iError = PVAviFileParserUtils::ReadNextChunkType(ipFilePtr, chunkType)) != PV_AVI_FILE_PARSER_SUCCESS)
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileParser::ParseFile: UnSupported Chunk at begining of the file"));
        return iError;
    }

    //first chunk must be RIFF
    if (chunkType != RIFF)
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileParser::ParseFile: First Chunk Must be RIFF"));
        iError = PV_AVI_FILE_PARSER_WRONG_FILE;
        return iError;
    }

    if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(ipFilePtr, iFileSize, true))
    {
        iError = PV_AVI_FILE_PARSER_READ_ERROR;
        return iError;
    }

    if ((iFileSize <= 0) || (iFileSize > filesize))
    {
        iError = PV_AVI_FILE_PARSER_WRONG_SIZE;
        return iError;
    }

    if ((iError = PVAviFileParserUtils::ReadNextChunkType(ipFilePtr, chunkType)) != PV_AVI_FILE_PARSER_SUCCESS)
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileParser::ParseFile: UnSupported Chunk at begining of the file"));
        return iError;
    }

    //second chunk must be AVI
    if (chunkType != AVI)
    {
        PVAVIFILE_LOGERROR((0, "Second Chunk Must be AVI"));
        iError = PV_AVI_FILE_PARSER_WRONG_FILE;
        return iError;
    }

    uint32 bytesRead = CHUNK_SIZE;
    uint32 listSize = 0;
    uint32 oldChkType = chunkType;

    while (bytesRead < iFileSize)
    {
        oldChkType = chunkType;
        if ((iError = PVAviFileParserUtils::ReadNextChunkType(ipFilePtr, chunkType)) != PV_AVI_FILE_PARSER_SUCCESS)
        {
            if ((PV_AVI_FILE_PARSER_UNSUPPORTED_CHUNK == iError))
            {
                PVAVIFILE_LOGINFO((0, "PVAviFileParser::ParseFile: Unsupported chunk"));

                uint32 chksz = 0;
                if (oldChkType != LIST)
                {
                    //get the size of unsupported chunk and skip it.
                    if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(ipFilePtr, chksz, true))
                    {
                        PVAVIFILE_LOGERROR((0, "PVAviFileParser::ParseFile: File Read Error"));
                        iError = PV_AVI_FILE_PARSER_READ_ERROR;
                        break;
                    }

                    ipFilePtr->Seek(chksz, Oscl_File::SEEKCUR);
                    bytesRead += chksz + CHUNK_SIZE + CHUNK_SIZE; //chunk data + chunk size + chunk type;

                }
                else
                {
                    //skip the entire list if not supported
                    chksz = listSize - CHUNK_SIZE ;  //subtract list name already read
                    ipFilePtr->Seek(chksz, Oscl_File::SEEKCUR);
                    bytesRead += listSize;
                }

                PVAVIFILE_LOGINFO((0, "PVAviFileParser::ParseFile: Unsupported chunk skipped"));
                iError = PV_AVI_FILE_PARSER_SUCCESS;
                continue;
            }
            else
            {
                break;
            }
        }
        bytesRead += CHUNK_SIZE;

        if (LIST == chunkType)
        {
            if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(ipFilePtr, listSize, true))
            {
                iError = PV_AVI_FILE_PARSER_READ_ERROR;
                break;
            }

            bytesRead += CHUNK_SIZE;
            if (bytesRead > iFileSize)
            {
                iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }

            if ((listSize <= 0) || (listSize > iFileSize))
            {
                iError = PV_AVI_FILE_PARSER_WRONG_SIZE;
                break;
            }
        }
        else if (HDRL == chunkType)
        {
            PVAVIFILE_LOGINFO((0, "PVAviFileParser::ParseFile: Found File Header"));

            iHeaderChunkSize = listSize - CHUNK_SIZE; //subtract 4 bytes of chunk type
            ipFileHeader = OSCL_NEW(PVAviFileHeader, (ipFilePtr, iHeaderChunkSize));
            if (ipFileHeader != NULL)
            {
                iError = ipFileHeader->GetStatus();
                if (iError != PV_AVI_FILE_PARSER_SUCCESS)
                {
                    OSCL_DELETE(ipFileHeader);
                    ipFileHeader = NULL;
                    break;
                }
            }
            bytesRead += iHeaderChunkSize;
            if (bytesRead > iFileSize)
            {
                iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }
        }
        else if (MOVI == chunkType)
        {
            PVAVIFILE_LOGINFO((0, "PVAviFileParser::ParseFile: Found MOVI Chunk"));

            iMovieChunkSize = listSize - CHUNK_SIZE;
            iMovieChunkStartOffset = ipFilePtr->Tell();		//get movi chunk start offset.
            iSampleOffset = iMovieChunkStartOffset;
            ipFilePtr->Seek(iMovieChunkSize, Oscl_File::SEEKCUR);

            uint32 numStr = GetNumStreams();
            for (uint32 ii = 0; ii < numStr; ii++)
            {
                iStreamCount.push_back(ii);
                iStreamSampleCount.push_back(0);
            }

            bytesRead += iMovieChunkSize;

        }
        else if (IDX1 == chunkType)
        {
            PVAVIFILE_LOGINFO((0, "PVAviFileParser::ParseFile: Found Index Table"));

            if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(ipFilePtr, iIndxChunkSize, true))
            {
                iError = PV_AVI_FILE_PARSER_READ_ERROR;
                break;
            }

            bytesRead += CHUNK_SIZE;
            if (bytesRead > iFileSize)
            {
                iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }

            if ((iIndxChunkSize <= 0) || (iIndxChunkSize > iFileSize))
            {
                iError = PV_AVI_FILE_PARSER_WRONG_SIZE;
                break;
            }
            uint32 numStreams = GetNumStreams();
            ipIdxChunk = OSCL_NEW(PVAviFileIdxChunk, (ipFilePtr, iIndxChunkSize, numStreams));
            if (ipIdxChunk != NULL)
            {
                iError = ipIdxChunk->GetStatus();
                if (iError != PV_AVI_FILE_PARSER_SUCCESS)
                {
                    OSCL_DELETE(ipIdxChunk);
                    ipIdxChunk = NULL;
                    break;
                }
                iIdxChunkPresent = true;
            }
            bytesRead += iIndxChunkSize;
        }
        else if (JUNK == chunkType)
        {
            PVAVIFILE_LOGINFO((0, "PVAviFileParser::ParseFile: Skip Junk data"));

            uint32 junkSize = 0;
            if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(ipFilePtr, junkSize, true))
            {
                iError = PV_AVI_FILE_PARSER_READ_ERROR;
                break;
            }

            bytesRead += CHUNK_SIZE;
            if (bytesRead > iFileSize)
            {
                iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }

            if ((junkSize <= 0) || (junkSize > iFileSize))
            {
                iError = PV_AVI_FILE_PARSER_WRONG_SIZE;
                break;
            }

            ipFilePtr->Seek(junkSize, Oscl_File::SEEKCUR);
            bytesRead += junkSize;
            if (bytesRead > iFileSize)
            {
                iError = PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
                break;
            }
        }
        else
        {
            iError = PV_AVI_FILE_PARSER_WRONG_CHUNK;
            break;
        }

    }	//while (bytesRead <= iFileSize)

    return iError;
}

PV_AVI_FILE_PARSER_ERROR_TYPE
PVAviFileParser::GetNextMediaSample(uint32& arStreamNo, uint8* aBuffer,
                                    uint32& arSize, uint32& arTimeStamp)
{
    if (iSampleOffset >= iFileSize)
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: Wrong offset"));
        return PV_AVI_FILE_PARSER_WRONG_OFFSET;
    }

    if (iSampleOffset == iMovieChunkStartOffset)
    {
        //first Sample reset sample count.
        for (uint32 ii = 0; ii < GetNumStreams(); ii++)
        {
            iStreamSampleCount[ii] = 0;
        }

        ipFilePtr->Seek(0, Oscl_File::SEEKSET);
    }

    uint32 CurrOff = ipFilePtr->Tell();
    if (CurrOff > iSampleOffset)
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: Wrong offset"));
        return PV_AVI_FILE_PARSER_WRONG_OFFSET;
    }

    ipFilePtr->Seek((iSampleOffset - CurrOff), Oscl_File::SEEKCUR);

    uint32 bytesRead = 0;
    uint32 data = 0;
    if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(ipFilePtr, data))
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: Read Error"));
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    int32 strNo = PVAviFileParserUtils::GetStreamNumber(data);
    if (PV_AVI_FILE_PARSER_ERROR_UNKNOWN == strNo)
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: Wrong Stream No"));
        return PV_AVI_FILE_PARSER_ERROR_WRONG_STREAM_NUM;
    }

    arStreamNo = strNo;

    bytesRead += CHUNK_SIZE;

    uint32 size = 0;
    if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(ipFilePtr, size, true))
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: File Read Error"));
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    bytesRead += CHUNK_SIZE;

    if (size > arSize)
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: Insufficient Buffer Size"));
        return PV_AVI_FILE_PARSER_INSUFFICIENT_MEMORY;
    }

    arSize = size;

    if (PVAviFileParserUtils::read8(ipFilePtr, aBuffer, arSize) != arSize)
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: File Read Error"));
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    iSampleOffset = iSampleOffset + arSize + bytesRead;

    uint32 offsetFrmMoviChk = iSampleOffset - iMovieChunkStartOffset;

    if (offsetFrmMoviChk >= iMovieChunkSize)
    {
        PVAVIFILE_LOGINFO((0, "PVAviFileParser::GetNextMediaSample: EOF Reached"));
        return PV_AVI_FILE_PARSER_EOF_REACHED;
    }

    //calculate time stamp
    if (oscl_strstr(GetStreamMimeType(arStreamNo).get_str(), "audio"))
    {
        arTimeStamp = iTimeStampAudio;
        uint32 sampleSize = GetBitsPerSample(arStreamNo);
        sampleSize = sampleSize / BIT_COUNT8; //in bytes
        OsclFloat  samplingRate = GetFrameRate(arStreamNo);
        if (sampleSize > 0)
        {
            OsclFloat  sampleCount = (OsclFloat)arSize / sampleSize;
            if (samplingRate > 0)
            {
                iTimeStampAudio += (uint32)((sampleCount * 1000) / samplingRate);
            }
        }
    }
    else
    {
        if (iStreamSampleCount[arStreamNo] > 0)
        {
            uint32 frameDurationInms = GetFrameDuration() / 1000;
            arTimeStamp = (iStreamSampleCount[arStreamNo] * (frameDurationInms));
        }
        else
        {
            arTimeStamp = 0;
        }
    }

    iStreamSampleCount[arStreamNo] = iStreamSampleCount[arStreamNo] + 1;
    return PV_AVI_FILE_PARSER_SUCCESS;
}


PV_AVI_FILE_PARSER_ERROR_TYPE
PVAviFileParser::GetNextStreamSampleInfo(uint32 aStreamNo, uint32& arSize, uint32& arOffset)
{
    if (false == iIdxChunkPresent)
    {
        return PV_AVI_FILE_PARSER_NO_INDEX_CHUNK;
    }
    else
    {
        return GetStreamOffsetFromIndexTable(aStreamNo, arSize, arOffset);
    }
}

PV_AVI_FILE_PARSER_ERROR_TYPE
PVAviFileParser::GetNextStreamMediaSample(uint32 aStreamNo, uint8* aBuffer,
        uint32& arSize, uint32& arTimeStamp)
{
    uint32 sampleOffset = 0;
    PV_AVI_FILE_PARSER_ERROR_TYPE error = PV_AVI_FILE_PARSER_SUCCESS;

    if (false == iIdxChunkPresent)
    {
        error = GetStreamOffset(aStreamNo, arSize, sampleOffset);

        if ((error != PV_AVI_FILE_PARSER_SUCCESS))
        {
            if (PV_AVI_FILE_PARSER_NO_OFFSET_FOUND == error)
            {
                arSize = 0;
                arTimeStamp = 0;
                //probably EOS has been reached.
                return PV_AVI_FILE_PARSER_EOS_REACHED;
            }
            else
            {
                return error;
            }
        }

        uint32 offsetFrmMoviChk = sampleOffset - iMovieChunkStartOffset;
        if (offsetFrmMoviChk >= iMovieChunkSize)
        {
            PVAVIFILE_LOGINFO((0, "PVAviFileParser::GetNextStreamMediaSample: EOF Reached"));
            return PV_AVI_FILE_PARSER_EOF_REACHED;
        }
    }
    else
    {
        error = GetStreamOffsetFromIndexTable(aStreamNo, arSize, sampleOffset);
        if (error != PV_AVI_FILE_PARSER_SUCCESS)
        {
            return error;
        }
    }

    if (PVAviFileParserUtils::read8(ipFilePtr, aBuffer, arSize) != arSize)
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: File Read Error"));
        return PV_AVI_FILE_PARSER_READ_ERROR;
    }

    //calculate time stamp
    if (oscl_strstr(GetStreamMimeType(aStreamNo).get_str(), "audio"))
    {
        arTimeStamp = iTimeStampAudio;
        uint32 sampleSize = GetBitsPerSample(aStreamNo);
        sampleSize = sampleSize / BIT_COUNT8; //in bytes
        OsclFloat samplingRate = GetFrameRate(aStreamNo);
        if (sampleSize > 0)
        {
            OsclFloat sampleCount = (OsclFloat)arSize / sampleSize;
            if (samplingRate > 0)
            {
                iTimeStampAudio += (uint32)((sampleCount * 1000) / samplingRate);
            }
        }
    }
    else
    {
        if (iStreamSampleCount[aStreamNo] > 0)
        {
            uint32 frameDurationInms = GetFrameDuration() / 1000;
            arTimeStamp = (iStreamSampleCount[aStreamNo] * (frameDurationInms));
        }
        else
        {
            arTimeStamp = 0;
        }
    }

    iStreamSampleCount[aStreamNo] = iStreamSampleCount[aStreamNo] + 1;

    return PV_AVI_FILE_PARSER_SUCCESS;
}


PV_AVI_FILE_PARSER_ERROR_TYPE
PVAviFileParser::GetStreamOffsetFromIndexTable(uint32 aStreamNo, uint32& arSize, uint32& arOffset)
{
    uint32 sampleOffset = 0;

    if (iStreamSampleCount[aStreamNo] >= ipIdxChunk->GetNumberOfSamplesInStream(aStreamNo))
    {
        PVAVIFILE_LOGINFO((0, "PVAviFileParser::GetNextMediaSample: EOF Reached"));
        return PV_AVI_FILE_PARSER_EOS_REACHED;
    }

    sampleOffset = ipIdxChunk->GetOffset(aStreamNo, iStreamSampleCount[aStreamNo]);
    uint32 size = ipIdxChunk->GetSampleSize(aStreamNo, iStreamSampleCount[aStreamNo]);

    if (size > arSize)
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: Insufficient Buffer Size"));
        return PV_AVI_FILE_PARSER_INSUFFICIENT_MEMORY;
    }

    arSize = size;

    if (ipIdxChunk->IsOffsetFromMoviList())
    {
        arOffset = iMovieChunkStartOffset + sampleOffset - CHUNK_SIZE;
    }
    else
    {
        arOffset = sampleOffset;
    }

    arOffset += (CHUNK_SIZE + CHUNK_SIZE); //add 4 bytes each for sample type and data size param.

    if ((arOffset > iFileSize))
    {
        PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: File Size & Byte Count mismatch"));
        return PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR;
    }

    ipFilePtr->Seek(0, Oscl_File::SEEKSET);
    ipFilePtr->Seek(arOffset, Oscl_File::SEEKCUR);

    return PV_AVI_FILE_PARSER_SUCCESS;
}


PV_AVI_FILE_PARSER_ERROR_TYPE
PVAviFileParser::GetStreamOffset(uint32 aStreamNo, uint32& arSampleSize, uint32& arOffset)
{
    uint32 startOffset = 0;
    arOffset = 0;
    bool foundOffset = false;

    if (iStreamSampleOffset.size() == 0)
    {
        uint32 numstreams = GetNumStreams();
        startOffset = iMovieChunkStartOffset;

        for (uint32 ii = 0; ii < numstreams; ii++)
        {
            iStreamSampleOffset.push_back(startOffset);
        }
    }
    else
    {
        startOffset = iStreamSampleOffset[aStreamNo];
    }

    ipFilePtr->Seek(startOffset, Oscl_File::SEEKSET);
    uint32 currentFileOffset = startOffset;

    uint32 endOfdataPosition = iMovieChunkStartOffset + iMovieChunkSize;

    while (currentFileOffset < endOfdataPosition)
    {
        uint32 data = 0;
        if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(ipFilePtr, data))
        {
            PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: Read Error"));
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }

        int32 strNo = PVAviFileParserUtils::GetStreamNumber(data);
        if (PV_AVI_FILE_PARSER_ERROR_UNKNOWN == strNo)
        {
            PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: Wrong Stream No"));
            return PV_AVI_FILE_PARSER_ERROR_WRONG_STREAM_NUM;
        }

        uint32 size = 0;
        if (PV_AVI_FILE_PARSER_SUCCESS != PVAviFileParserUtils::read32(ipFilePtr, size, true))
        {
            PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: File Read Error"));
            return PV_AVI_FILE_PARSER_READ_ERROR;
        }

        if ((uint32)strNo == aStreamNo)
        {
            arOffset = ipFilePtr->Tell();
            iStreamSampleOffset[aStreamNo] = arOffset + size;
            if (size > arSampleSize)
            {
                PVAVIFILE_LOGERROR((0, "PVAviFileParser::GetNextMediaSample: Insufficient Buffer Size"));
                return PV_AVI_FILE_PARSER_INSUFFICIENT_MEMORY;
            }

            arSampleSize = size;
            foundOffset = true;
            break;
        }
        else
        {
            ipFilePtr->Seek(size, Oscl_File::SEEKCUR);
            foundOffset = false;
        }

        currentFileOffset = ipFilePtr->Tell();
    }

    if (foundOffset)
    {
        return PV_AVI_FILE_PARSER_SUCCESS;
    }
    else
    {
        return PV_AVI_FILE_PARSER_NO_OFFSET_FOUND;
    }
}

OSCL_EXPORT_REF void PVAviFileParser::Reset()
{
    PVAVIFILE_LOGINFO((0, "PVAviFileParser::Reset"));

    uint32 ii = 0;
    for (ii = 0; ii < GetNumStreams(); ii++)
    {
        iStreamSampleCount[ii] = 0;
    }

    if (iStreamSampleOffset.size() > 0)
    {
        for (ii = 0; ii < iStreamSampleOffset.size(); ii++)
        {
            iStreamSampleOffset.pop_back();
        }
    }

    iSampleOffset = iMovieChunkStartOffset;
    iTimeStampAudio = 0;
}

OSCL_EXPORT_REF void PVAviFileParser::Reset(uint32 aStreamNo)
{
    PVAVIFILE_LOGINFO((0, "PVAviFileParser::Reset stream"));

    iStreamSampleCount[aStreamNo] = 0;

    if (iStreamSampleOffset.size() > aStreamNo)
    {
        iStreamSampleOffset[aStreamNo] = iMovieChunkStartOffset;
    }

    if (oscl_strstr(GetStreamMimeType(aStreamNo).get_str(), "audio"))
    {
        iTimeStampAudio = 0;
    }
}

OSCL_EXPORT_REF bool PVAviFileParser::GetFormatSpecificInfo(uint32 aStreamNum, OsclRefCounterMemFrag& arFormatSpecificDataFrag)
{
    uint8* buff = NULL;
    uint32 len = 0;

    ipFileHeader->GetFormatSpecificInfo(aStreamNum, buff, len);

    if ((buff == NULL) || (len == 0))
    {
        return false;
    }

    OsclMemAllocDestructDealloc<uint8> my_alloc;
    OsclRefCounter* my_refcnt;

    uint aligned_refcnt_size =
        oscl_mem_aligned_size(sizeof(OsclRefCounterSA< OsclMemAllocDestructDealloc<uint8> >));

    uint8* my_ptr = NULL;
    int32 errcode = 0;
    OSCL_TRY(errcode, my_ptr = (uint8*) oscl_malloc(aligned_refcnt_size + len));

    if (errcode != OsclErrNone)
    {
        return false;
    }

    my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterSA< OsclMemAllocDestructDealloc<uint8> >(my_ptr));
    my_ptr += aligned_refcnt_size;

    oscl_memcpy(my_ptr, buff, len);

    OsclMemoryFragment memfrag;
    memfrag.len = len;
    memfrag.ptr = my_ptr;

    OsclRefCounterMemFrag refcntMemFrag(memfrag, my_refcnt, memfrag.len);
    arFormatSpecificDataFrag = refcntMemFrag;
    return true;

}

//method to get codec specific info
OSCL_EXPORT_REF bool PVAviFileParser::GetCodecSpecificData(uint32 aStreamNum, OsclRefCounterMemFrag& arFormatSpecificDataFrag)
{
    uint8* buff = NULL;
    uint32 len = 0;
    ipFileHeader->GetCodecSpecificData(aStreamNum, buff, len);

    if ((buff == NULL) || (len == 0))
    {
        return false;
    }

    OsclMemAllocDestructDealloc<uint8> my_alloc;
    OsclRefCounter* my_refcnt;

    uint aligned_refcnt_size =
        oscl_mem_aligned_size(sizeof(OsclRefCounterSA< OsclMemAllocDestructDealloc<uint8> >));

    uint8* my_ptr = NULL;
    int32 errcode = 0;
    OSCL_TRY(errcode, my_ptr = (uint8*) oscl_malloc(aligned_refcnt_size + len));

    if (errcode != OsclErrNone)
    {
        return false;
    }

    my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterSA< OsclMemAllocDestructDealloc<uint8> >(my_ptr));
    my_ptr += aligned_refcnt_size;

    oscl_memcpy(my_ptr, buff, len);

    OsclMemoryFragment memfrag;
    memfrag.len = len;
    memfrag.ptr = my_ptr;

    OsclRefCounterMemFrag refcntMemFrag(memfrag, my_refcnt, memfrag.len);
    arFormatSpecificDataFrag = refcntMemFrag;
    return true;
}

PVAviFileParser::~PVAviFileParser()
{
    if (NULL != ipFileHeader)
    {
        OSCL_DELETE(ipFileHeader);
        ipFileHeader = NULL;
    }

    if (NULL != ipIdxChunk)
    {
        OSCL_DELETE(ipIdxChunk);
        ipIdxChunk = NULL;
    }

    if (ipFilePtr->IsOpen())
    {
        ipFilePtr->Close();
    }

    OSCL_DELETE(ipFilePtr);
}

uint32 PVAviFileParser::GetFileSuggestedBufferSize()
{
    uint32 buffSz = ipFileHeader->GetFileSuggestedBufferSize();
    if (0 == buffSz)
    {
        uint32 numStr = ipFileHeader->GetNumStreams();
        for (uint32 ii = 0; ii < numStr; ii++)
        {
            uint32 strSz = ipFileHeader->GetStreamSuggestedBufferSize(ii);
            if (buffSz < strSz)
            {
                buffSz = strSz;
            }
        }
    }
    return buffSz;
}

bool PVAviFileParser::GetVideoFormatType(uint8* aHdlr, uint32& arSize, uint32 aStreamNo)
{
    uint32 numstr = GetNumStreams();
    uint32 ii = 0;
    bool retTyp = false;
    for (ii = 0; ii < numstr; ii++)
    {
        if ((oscl_strstr((GetStreamMimeType(ii)).get_cstr(), "video")) && (aStreamNo == ii))
        {
            ipFileHeader->GetHandlerType(ii, aHdlr, arSize);
            retTyp = true;
        }
    }
    return retTyp;
}

OSCL_EXPORT_REF Oscl_Vector<uint32, OsclMemAllocator> PVAviFileParser::GetAudioStreamCountList()
{
    uint32 ii = 0;
    Oscl_Vector<uint32, OsclMemAllocator> audioStrNum;
    for (ii = 0; ii < GetNumStreams(); ii++)
    {
        if (oscl_strstr((GetStreamMimeType(ii)).get_cstr(), "audio"))
        {
            audioStrNum.push_back(ii);
        }
    }

    return audioStrNum;
}

OSCL_EXPORT_REF Oscl_Vector<uint32, OsclMemAllocator> PVAviFileParser::GetVideoStreamCountList()
{
    uint32 ii = 0;
    Oscl_Vector<uint32, OsclMemAllocator> vidStrNum;
    for (ii = 0; ii < GetNumStreams(); ii++)
    {
        if (oscl_strstr((GetStreamMimeType(ii)).get_cstr(), "video"))
        {
            vidStrNum.push_back(ii);
        }
    }

    return vidStrNum;
}

OSCL_EXPORT_REF uint32 PVAviFileParser::GetFileDuration()
{
    uint32 frameDuration = ipFileHeader->GetFrameDuration();
    uint32 totalFrames = ipFileHeader->GetTotalFrames();
    uint32 fileDuration = (uint32)((OsclFloat)(frameDuration * totalFrames) / 1000000.0); //in seconds
    return fileDuration;
}

OSCL_EXPORT_REF PVAviFileStreamHeaderStruct
PVAviFileParser::GetStreamHeaderStruct(uint32 aStreamNum)
{
    return (ipFileHeader->GetStreamHeaderStruct(aStreamNum));
}

OSCL_EXPORT_REF uint32 PVAviFileParser::GetNumStreams()
{
    return (ipFileHeader->GetNumStreams());
}

OSCL_EXPORT_REF OSCL_HeapString<OsclMemAllocator>
PVAviFileParser::GetStreamMimeType(uint32 aStreamNum)
{
    return (ipFileHeader->GetStreamMimeType(aStreamNum));
}

OSCL_EXPORT_REF uint32 PVAviFileParser::GetWidth(int32 aStreamNo)
{
    return (ipFileHeader->GetWidth(aStreamNo));
}

OSCL_EXPORT_REF uint32 PVAviFileParser::GetHeight(bool& rBottomUp, int32 aStreamNo)
{
    return (ipFileHeader->GetHeight(rBottomUp, aStreamNo));
}

OSCL_EXPORT_REF uint32 PVAviFileParser::GetFrameDuration()
{
    return (ipFileHeader->GetFrameDuration());
}

OSCL_EXPORT_REF OsclFloat PVAviFileParser::GetFrameRate(uint32 aStreamNum)
{
    return (ipFileHeader->GetSamplingRate(aStreamNum));
}

OSCL_EXPORT_REF uint32 PVAviFileParser::GetStreamDuration(uint32 aStreamNum)
{
    return (ipFileHeader->GetStreamDuration(aStreamNum));
}
