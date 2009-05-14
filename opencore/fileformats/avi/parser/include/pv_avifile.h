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
 * @file pv_avifile.h
 * @brief avi file parser utility
 */

#ifndef PV_AVIFILE_H_INCLUDED
#define PV_AVIFILE_H_INCLUDED

#ifndef PV_AVIFILE_TYPEDEFS_H_INCLUDED
#include "pv_avifile_typedefs.h"
#endif

/**
 * PVAviFile is an avi file parser utility. This allows user to retrieve media data and media data
 * properties from avi file.
 */

class PVAviFile: public PVAviFileParserStatus
{
    public:

        /**
         * Creates parser Object and parse AVI File.
         *
         * @param aFileName avi file name
         * @return arError error occured while parsing
         * @param aFileServer file server
         * @param aCPM CMP Plugin Interface
         * @param aHandle file handle
         * @return pointer to PVAviFile
         */

        OSCL_IMPORT_REF static PVAviFile*  CreateAviFileParser(OSCL_wHeapString<OsclMemAllocator> aFileName, int32& arError,
                Oscl_FileServer* aFileServer = NULL, PVMFCPMPluginAccessInterfaceFactory*  aCPM = NULL,
                OsclFileHandle*  aHandle = NULL);

        /**
         * Destroys parser Object.
         * @param apAviFileParser pointer to avi parser object
         */

        OSCL_IMPORT_REF static void DeleteAviFileParser(PVAviFile* apAviFileParser);

        /** Destructor */
        virtual ~PVAviFile() {};

        /**
         * function to retrieve media samples from avi file.
         *
         * @param aBuffer buffer for media sample
         * @return aStreamNo stream number
         * @return aSize size of data filled in aBuffer
         * @return aTimeStamp timestamp of the media sample
         */

        OSCL_IMPORT_REF virtual PV_AVI_FILE_PARSER_ERROR_TYPE
        GetNextMediaSample(uint32& aStreamNo, uint8* aBuffer, uint32& arSize,
                           uint32& arTimeStamp) = 0;

        /**
         * function to retrieve media samples for a stream in avi file. this function
         * will use index table if present otherwise it will search the movi chunk to
         * get media sample for the specified stream.
         *
         * @param aBuffer buffer for media sample
         * @param aStreamNo stream number
         * @return aSize size of data filled in aBuffer
         * @return aTimeStamp timestamp of the media sample
         */

        OSCL_IMPORT_REF virtual PV_AVI_FILE_PARSER_ERROR_TYPE
        GetNextStreamMediaSample(uint32 aStreamNo, uint8* aBuffer,
                                 uint32& arSize, uint32& arTimeStamp) = 0;


        /**
         * function to retrieve media samples information for a stream in avi file. this function
         * will use index table if present otherwise it will return failure
         *
         * @param aStreamNo stream number
         * @return arSize sample size
         * @return arOffset sample offset in file
         */

        OSCL_IMPORT_REF virtual PV_AVI_FILE_PARSER_ERROR_TYPE
        GetNextStreamSampleInfo(uint32 aStreamNo, uint32& arSize, uint32& arOffset) = 0;

        /**
         * function to reset stream pointers to retrieve data from the begining. .
         * @param
         * @return
         */

        OSCL_IMPORT_REF virtual void Reset() = 0;

        /**
         * function to reset stream pointer to retrieve stream data from the begining. .
         * @param aStreamNo stream number
         * @return
         */

        OSCL_IMPORT_REF virtual void Reset(uint32 aStreamNo) = 0;

        /**
         * function to retrieve avi file properties.
         * @param
         * @return structure of type PVAviFileMainHeaderStruct
         */

        OSCL_IMPORT_REF virtual PVAviFileMainHeaderStruct GetMainHeaderStruct() = 0;

        /**
         * function to get avi file duration.
         * @param
         * @return file duration in Seconds
         */

        OSCL_IMPORT_REF virtual uint32 GetFileDuration() = 0;

        /**
         * function to retrieve stream numbers.
         * @param
         * @return micro seconds per frame.
         */

        OSCL_IMPORT_REF virtual uint32 GetFrameDuration() = 0;

        /**
         * function to retrieve stream numbers.
         * @param
         * @return vector of stream numbers
         */

        OSCL_IMPORT_REF virtual Oscl_Vector<uint32, OsclMemAllocator>  GetStreamCountList() = 0;

        /**
        * function to retrieve stream numbers.
        * @param
        * @return vector of Audio stream numbers
        */

        OSCL_IMPORT_REF virtual Oscl_Vector<uint32, OsclMemAllocator>  GetAudioStreamCountList() = 0;

        /**
        * function to retrieve stream numbers.
        * @param
        * @return vector of Video stream numbers
        */

        OSCL_IMPORT_REF virtual Oscl_Vector<uint32, OsclMemAllocator>  GetVideoStreamCountList() = 0;

        /**
         * function to retrieve number of streams.
         * @param
         * @return number of streams
         */

        OSCL_IMPORT_REF virtual uint32 GetNumStreams() = 0;

        /**
         * function to retrieve average buffer size of the file.
         * @param
         * @return average buffer size.
         */

        OSCL_IMPORT_REF virtual uint32 GetFileSuggestedBufferSize() = 0;

        /**
         * function to retrieve avi file properties.
         * @param
         * @return file data rate.
         */

        OSCL_IMPORT_REF virtual uint32 GetFileDataRate() = 0;

        /**
         * function to retrieve avi file properties.
         * @param
         * @return total frames in file.
         */

        OSCL_IMPORT_REF virtual uint32 GetTotalFrames() = 0;

        /**
         * function to retrieve avi file properties.
         * @param
         * @return number of frames required to be retrieved for interleaved files.
         */

        OSCL_IMPORT_REF virtual uint32 GetInitialFrames() = 0;

        /**
         * function to retrieve avi file properties.
         * @param
         * @return width of video frame
         */

        OSCL_IMPORT_REF virtual uint32 GetWidth(int32 aStreamNo = -1) = 0;

        /**
         * function to retrieve avi file properties.
         * @param aStreamNo stream number
         * @param rBottomUp returns the display orientation(true if bottom up, flase if top down)
         * @return height of video frame
         */

        OSCL_IMPORT_REF virtual uint32 GetHeight(bool& rBottomUp, int32 aStreamNo = -1) = 0;

        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return Stream duration
         */
        OSCL_IMPORT_REF virtual uint32 GetStreamDuration(uint32 aStreamNum) = 0;
        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return number of bits per sample
         */
        OSCL_IMPORT_REF virtual uint32 GetBitsPerSample(uint32 aStreamNum) = 0;

        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return PVAviFileStreamHeaderStruct structure of stream porperties
         */

        OSCL_IMPORT_REF virtual PVAviFileStreamHeaderStruct GetStreamHeaderStruct(uint32 aStreamNum) = 0;

        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return stream mime type
         */

        OSCL_IMPORT_REF virtual OSCL_HeapString<OsclMemAllocator>  GetStreamMimeType(uint32 aStreamNum) = 0;

        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return suggested biffer size for the stream with stream number aStreamNum
         */

        OSCL_IMPORT_REF virtual uint32 GetStreamSuggestedBufferSize(uint32 aStreamNum) = 0;

        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return aHdlr buffer to store FCC code for the video format
         * @return aSize size of aHdlr.
         */

        OSCL_IMPORT_REF virtual bool GetVideoFormatType(uint8* aHdlr, uint32& arSize, uint32 aStreamNo) = 0;

        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return audio format type number e.g 1 for WAVE_FORMAT_PCM
         */

        OSCL_IMPORT_REF virtual uint32 GetAudioFormatType(uint32 aStreamNo) = 0;

        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return returns rate. Dividing rate by scale gives the no if samples per second.
         */

        OSCL_IMPORT_REF virtual uint32 GetRate(uint32 aStreamNum) = 0;

        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return returns scale. Dividing rate by scale gives no of samples per second.
         */

        OSCL_IMPORT_REF virtual uint32 GetScale(uint32 aStreamNum) = 0;

        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return for aduio:sampling rate,for video:frames per second
         */

        OSCL_IMPORT_REF virtual OsclFloat GetFrameRate(uint32 aStreamNum) = 0;

        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return number of audio channels
         */

        OSCL_IMPORT_REF virtual uint32 GetNumAudioChannels(uint32 aStreamNo) = 0;

        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return structure rcFrameType for rc frames
         */

        OSCL_IMPORT_REF virtual rcFrameType GetRCFrame(uint32 aStreamNum) = 0;

        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return aFormatSpecificDataFrag stream format specific properties
         */

        OSCL_IMPORT_REF virtual bool GetFormatSpecificInfo(uint32 aStreamNum, OsclRefCounterMemFrag& arFormatSpecificDataFrag) = 0;

        /**
         * function to retrieve stream properties.
         * @param aStreamNum stream number
         * @return aFormatSpecificDataFrag codec specific information.
         */

        OSCL_IMPORT_REF virtual bool GetCodecSpecificData(uint32 aStreamNum, OsclRefCounterMemFrag& arFormatSpecificDataFrag) = 0;

};

#endif		//#ifndef PV_AVIFILE_H_INCLUDED

