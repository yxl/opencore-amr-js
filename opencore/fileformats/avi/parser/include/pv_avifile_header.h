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
#ifndef PV_AVIFILE_HEADER_H_INCLUDED
#define PV_AVIFILE_HEADER_H_INCLUDED


#ifndef PV_AVIFILE_TYPEDEFS_H_INCLUDED
#include "pv_avifile_typedefs.h"
#endif

#ifndef PV_AVIFILE_PARSER_UTILS_H_INCLUDED
#include "pv_avifile_parser_utils.h"
#endif

#ifndef	PV_AVIFILE_STREAMLIST_H_INCLUDED
#include "pv_avifile_streamlist.h"
#endif



// class to parse main header
class PVAviFileHeader: public PVAviFileParserStatus
{
    public:

        //constructor
        PVAviFileHeader(PVFile *aFp, uint32 aHdrSize);

        //destructor
        ~PVAviFileHeader()
        {
            for (uint32 ii = 0; ii < iStreamList.size(); ii++)
            {
                iStreamList.pop_back();
            }
        }

        //methods to retrieve PVAviFileMainHeaderStruct information
        PVAviFileMainHeaderStruct GetMainHeaderStruct()
        {
            return iMainHeader;
        }

        uint32 GetFrameDuration()
        {
            return (iMainHeader.iMicroSecPerFrame);
        };

        uint32 GetNumStreams()
        {
            return (iMainHeader.iStreams);
        }

        uint32 GetFileSuggestedBufferSize()
        {
            return (iMainHeader.iSuggestedBufferSize);
        }

        uint32 GetFileDataRate()
        {
            return (iMainHeader.iMaxBytesPerSec);
        }

        bool IsCopyrighted()
        {
            return (iMainHeader.iIsAVIFileCopyrighted);
        }

        bool HasIndexTable()
        {
            return (iMainHeader.iAVIFileHasIndxTbl);
        }

        bool MustUseIndexTbl()
        {
            return (iMainHeader.iAVIFileMustUseIndex);
        }

        bool IsInterleaved()
        {
            return (iMainHeader.iISAVIFileInterleaved);
        }

        bool MustUseIndex()
        {
            return (iMainHeader.iAVIFileMustUseIndex);
        }

        bool IsCaptureFile()
        {
            return (iMainHeader.iAVIFileWasCaptureFile);
        }

        uint32 GetTotalFrames()
        {
            return (iMainHeader.iTotalFrames);
        }

        uint32 GetInitialFrames()
        {
            return (iMainHeader.iInitialFrames);
        }

        uint32 GetWidth(int32 aStreamNo)
        {
            if (aStreamNo > 0)
            {
                return iStreamList[aStreamNo].GetVideoWidth();
            }
            else
            {
                return (iMainHeader.iWidth);
            }
        }

        uint32 GetHeight(bool& rBottomUp, int32 aStreamNo = -1)
        {
            if (aStreamNo > 0)
            {
                return iStreamList[aStreamNo].GetVideoHeight(rBottomUp);
            }
            else
            {
                rBottomUp = true; //rgb bmp is stored as bottom-up by default.
                return (iMainHeader.iHeight);
            }
        }

        //methods to retrieve PVAviFileStreamlist information
        PVAviFileStreamHeaderStruct GetStreamHeaderStruct(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetStreamHeaderStruct());
        }

        OSCL_HeapString<OsclMemAllocator>  GetStreamMimeType(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetStreamMimeType());
        }

        bool PalletChangeAvailable(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].PalletChangeAvailable());
        }

        bool ISDisabledByDefault(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].ISDisabledByDefault());
        }

        bool GetHandlerType(uint32 aStreamNum, uint8* aHdlr, uint32& aSize)
        {
            return (iStreamList[aStreamNum].GetHandlerType(aHdlr, aSize));
        }

        uint32 GetAudioFormat(uint32 aStreamNo);

        uint32 GetNumAudioChannels(uint32 aStreamNo);

        uint16 GetPriority(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetPriority());
        }

        uint16 GetLanguage(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetLanguage());
        }

        uint32 GetInitalFrames(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetInitalFrames());
        }

        uint32 GetRate(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetRate());
        }

        uint32 GetScale(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetScale());
        }

        OsclFloat GetSamplingRate(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetSamplingRate());
        }

        uint32 GetStartTime(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetStartTime());
        }

        uint32 GetStreamDuration(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetStreamDuration());
        }

        uint32 GetStreamSuggestedBufferSize(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetStreamSuggestedBufferSize());
        }

        int32 GetQuality(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetQuality());
        }

        uint32 GetBitsPerSample(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetBitsPerSample());
        }

        uint32 GetSampleSize(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetSampleSize());
        }

        rcFrameType GetRCFrame(uint32 aStreamNum)
        {
            return (iStreamList[aStreamNum].GetRCFrame());
        }

        bool GetFormatSpecificInfo(uint32 aStreamNum, uint8*& aBuff, uint32& aSize)
        {
            return ((iStreamList[aStreamNum]).GetFormatSpecificInfo(aBuff, aSize));
        }

        bool GetCodecSpecificData(uint32 aStreamNum, uint8*& aBuff, uint32& aSize)
        {
            return ((iStreamList[aStreamNum]).GetCodecSpecificData(aBuff, aSize));
        }


    private:


        PV_AVI_FILE_PARSER_ERROR_TYPE       ParseMainHeader(PVFile *aFp);

        //File Main Header
        PVAviFileMainHeaderStruct		    iMainHeader;
        uint32								iHeaderTotalSize;

        //vector to store stream specific info.
        //Index of the vector corresponds to stream number
        Oscl_Vector < PVAviFileStreamlist,
        OsclMemAllocator >		iStreamList;

};

#endif //#ifndef PV_AVIFILE_HEADER_H_INCLUDED

