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
#ifndef PV_AVIFILE_PARSER_H_INCLUDED
#define PV_AVIFILE_PARSER_H_INCLUDED

#ifndef PV_AVIFILE_TYPEDEFS_H_INCLUDED
#include "pv_avifile_typedefs.h"
#endif

#ifndef PV_AVIFILE_PARSER_UTILS_H_INCLUDED
#include "pv_avifile_parser_utils.h"
#endif

#ifndef PV_AVIFILE_HEADER_H_INCLUDED
#include "pv_avifile_header.h"
#endif

#ifndef PV_AVIFILE_INDX_H_INCLUDED
#include "pv_avifile_indx.h"
#endif

#ifndef PV_AVIFILE_H_INCLUDED
#include "pv_avifile.h"
#endif


//class to parse avi file
class PVAviFileParser: public PVAviFile
{

    public:

        //constructor
        PVAviFileParser(OSCL_wHeapString<OsclMemAllocator> aFileName, int32& aError, Oscl_FileServer* aFileServer,
                        PVMFCPMPluginAccessInterfaceFactory*  aCPM, OsclFileHandle*  aFileHandle);

        //destructor
        ~PVAviFileParser();

        PV_AVI_FILE_PARSER_ERROR_TYPE ParseFile();

        //Read Media Sample form the file in order they exist in 'movi' chunk
        PV_AVI_FILE_PARSER_ERROR_TYPE
        GetNextMediaSample(uint32& arStreamNo, uint8* aBuffer, uint32& arSize,
                           uint32& arTimeStamp);

        //read next media samples for a stream with stream number = aStreamNo.
        PV_AVI_FILE_PARSER_ERROR_TYPE
        GetNextStreamMediaSample(uint32 aStreamNo, uint8* aBuffer, uint32& arSize,
                                 uint32& arTimeStamp);

        PV_AVI_FILE_PARSER_ERROR_TYPE
        GetNextStreamSampleInfo(uint32 aStreamNo, uint32& arSize, uint32& arOffset);

        //methods to retrieve file properties
        PVAviFileMainHeaderStruct GetMainHeaderStruct()
        {
            return (ipFileHeader->GetMainHeaderStruct());
        }

        OSCL_IMPORT_REF uint32 GetFileDuration();

        OSCL_IMPORT_REF void Reset();
        OSCL_IMPORT_REF void Reset(uint32 aStreamNo);

        OSCL_IMPORT_REF uint32 GetFrameDuration();

        OSCL_IMPORT_REF uint32 GetNumStreams();

        uint32 GetFileSuggestedBufferSize();

        uint32 GetFileDataRate()
        {
            return (ipFileHeader->GetFileDataRate());
        }

        bool IsCopyrighted()
        {
            return (ipFileHeader->IsCopyrighted());
        }
        bool HasIndexTable()
        {
            return (ipFileHeader->HasIndexTable());
        }

        bool IsInterleaved()
        {
            return (ipFileHeader->IsInterleaved());
        }

        bool MustUseIndex()
        {
            return (ipFileHeader->MustUseIndex());
        }

        bool IsCaptureFile()
        {
            return (ipFileHeader->IsCaptureFile());
        }

        uint32 GetTotalFrames()
        {
            return (ipFileHeader->GetTotalFrames());
        }

        uint32 GetInitialFrames()
        {
            return (ipFileHeader->GetInitialFrames());
        }

        OSCL_IMPORT_REF uint32 GetWidth(int32 aStreamNo);

        OSCL_IMPORT_REF uint32 GetHeight(bool& rBottomUp, int32 aStreamNo);

        //methods to retrieve stream properties
        OSCL_IMPORT_REF PVAviFileStreamHeaderStruct GetStreamHeaderStruct(uint32 aStreamNum);

        OSCL_IMPORT_REF OSCL_HeapString<OsclMemAllocator>  GetStreamMimeType(uint32 aStreamNum);
        OSCL_IMPORT_REF uint32 GetStreamDuration(uint32 aStreamNum);

        bool PalletChangeAvailable(uint32 aStreamNum)
        {
            return (ipFileHeader->PalletChangeAvailable(aStreamNum));
        }

        bool ISDisabledByDefault(uint32 aStreamNum)
        {
            return (ipFileHeader->ISDisabledByDefault(aStreamNum));
        }

        bool GetVideoFormatType(uint8* aHdlr, uint32& arSize, uint32 aStreamNo);

        uint32 GetAudioFormatType(uint32 aStreamNo)
        {
            return ipFileHeader->GetAudioFormat(aStreamNo);
        }

        uint32 GetNumAudioChannels(uint32 aStreamNo)
        {
            return ipFileHeader->GetNumAudioChannels(aStreamNo);
        }

        uint16 GetPriority(uint32 aStreamNum)
        {
            return (ipFileHeader->GetPriority(aStreamNum));
        }

        uint16 GetLanguage(uint32 aStreamNum)
        {
            return (ipFileHeader->GetLanguage(aStreamNum));
        }

        uint32 GetInitalFrames(uint32 aStreamNum)
        {
            return (ipFileHeader->GetInitalFrames(aStreamNum));
        }

        uint32 GetRate(uint32 aStreamNum)
        {
            return (ipFileHeader->GetRate(aStreamNum));
        }

        uint32 GetScale(uint32 aStreamNum)
        {
            return (ipFileHeader->GetScale(aStreamNum));
        }

        OSCL_IMPORT_REF OsclFloat GetFrameRate(uint32 aStreamNum);

        uint32 GetStartTime(uint32 aStreamNum)
        {
            return (ipFileHeader->GetStartTime(aStreamNum));
        }


        uint32 GetStreamSuggestedBufferSize(uint32 aStreamNum)
        {
            return (ipFileHeader->GetStreamSuggestedBufferSize(aStreamNum));
        }

        uint32 GetBitsPerSample(uint32 aStreamNum)
        {
            return (ipFileHeader->GetBitsPerSample(aStreamNum));
        }

        int32 GetQuality(uint32 aStreamNum)
        {
            return (ipFileHeader->GetQuality(aStreamNum));
        }

        uint32 GetSampleSize(uint32 aStreamNum)
        {
            return (ipFileHeader->GetSampleSize(aStreamNum));
        }

        rcFrameType GetRCFrame(uint32 aStreamNum)
        {
            return (ipFileHeader->GetRCFrame(aStreamNum));
        }

        //method to get codec specific info
        OSCL_IMPORT_REF bool GetCodecSpecificData(uint32 aStreamNum, OsclRefCounterMemFrag& arFormatSpecificDataFrag);

        //method to retrieve stream format specific info
        OSCL_IMPORT_REF bool GetFormatSpecificInfo(uint32 aStreamNum, OsclRefCounterMemFrag& arFormatSpecificDataFrag);

        Oscl_Vector<uint32, OsclMemAllocator>	GetStreamCountList()
        {
            return iStreamCount;
        }

        OSCL_IMPORT_REF Oscl_Vector<uint32, OsclMemAllocator>	GetAudioStreamCountList();
        OSCL_IMPORT_REF Oscl_Vector<uint32, OsclMemAllocator>	GetVideoStreamCountList();

    private:

        PV_AVI_FILE_PARSER_ERROR_TYPE
        GetStreamOffsetFromIndexTable(uint32 aStreamNo, uint32& arSize, uint32& aOffset);

        PV_AVI_FILE_PARSER_ERROR_TYPE
        GetStreamOffset(uint32 aStreamNo, uint32& aSampleSize, uint32& aOffset);


        PVAviFileHeader*		ipFileHeader;
        PVAviFileIdxChunk*		ipIdxChunk;
        bool					iIdxChunkPresent;
        uint32					iFileSize;
        uint32					iHeaderChunkSize;
        uint32					iMovieChunkSize;

        //store the offset of first sample. used if index table is not present.
        uint32					iMovieChunkStartOffset;
        uint32					iIndxChunkSize;
        Oscl_Vector < uint32,
        OsclMemAllocator >	iStreamCount;

        Oscl_Vector < uint32,
        OsclMemAllocator >		iStreamSampleCount;

        //stores current offset if index table is not present
        uint32					iSampleOffset;

        //store latest sample offset if index table is not present
        Oscl_Vector < uint32,
        OsclMemAllocator >		iStreamSampleOffset;

        PVFile*					ipFilePtr;
        uint32					iTimeStampAudio;
        uint32					iTimeStampVideo;

};

#endif	//#ifndef PV_AVIFILE_PARSER_H_INCLUDED

