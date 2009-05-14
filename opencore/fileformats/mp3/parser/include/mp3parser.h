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

// -*- c++ -*-
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//                       M P 3   P A R S E R

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


/**
 *  @file mp3parser.h
 *  @brief This include file contains the definitions for the actual MP3
 *  file parser.
 */

#ifndef MP3PARSER_H_INCLUDED
#define MP3PARSER_H_INCLUDED


//----------------------------------------------------------------------
// Include Files
//----------------------------------------------------------------------
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCL_MEDIA_DATA_H
#include "oscl_media_data.h"
#endif
#ifndef PVFILE_H_INCLUDED
#include "pvfile.h"
#endif
#ifndef IMP3FF_H_INCLUDED
#include "imp3ff.h"
#endif
#ifndef PV_GAU_H_
#include "pv_gau.h"
#endif
#ifndef PV_ID3_PARCOM_H_INCLUDED
#include "pv_id3_parcom.h"
#endif
#ifndef MP3_UTILS_H_INCLUDED
#include "mp3utils.h"
#endif
#ifndef __MEDIA_CLOCK_CONVERTER_H
#include "media_clock_converter.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

//----------------------------------------------------------------------
// Global Type Declarations
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Global Constant Declarations
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// Global Data Declarations
//----------------------------------------------------------------------


//======================================================================
//  CLASS DEFINITIONS and FUNCTION DECLARATIONS
//======================================================================

#define MIN_RANDOM_FRAMES_TO_SCAN 4
#define MIN_RANDOM_LOCATION_TO_SCAN 30

#define STR_VBRI_HEADER_IDENTIFIER "VBRI"
#define STR_XING_HEADER_IDENTIFIER "Xing"
#define STR_INFO_HEADER_IDENTIFIER "Info"

#define VBR_HEADER_SIZE 0x04
#define VBRI_HEADER_OFFSET 0x0024

#define DEFAULT_NUM_FRAMES_TO_SCAN 20
#define ID3_V1_TAG_SIZE 128

#define MAX_TOC_ENTRY_COUNT 200

typedef struct mp3Header_tag
{
    int32 SamplingRate;
    int32 BitRate;
    int32 FrameLengthInBytes;
    int32 FrameSizeUnComp;
    int32 NumberOfChannels;
} MP3ConfigInfoType;

typedef struct mp3HeaderInfo_tag
{
    int32 frameVer;
    int32 layerID;
    int32 crcFollows;
    int32 brIndex;
    int32 srIndex;
    int32 prvBit;
    int32 padBit;
    int32 chMode;
    int32 modeExtn;
    int32 frameSize;
} MP3HeaderType;

// XING Header for VBR Support
// A XING Header may be present in the ancilary data field of
// the first frame of an mp3 bitsream
// The Xing header (optionally) contains:
//    frames   the total number of frames in the audio bitstream
//    bytes    total number of bytes in the bitstream
//    toc      table of contents
// TOC (Table Of Contents) gives seek points for random access
// the ith entry determines the seek point for i-percent duration
// Seek point in bytes = (toc[i]/256.0)*total_bitstream_bytes
// e.g. half seek point = (toc[50]/256.0)*total_bitstream_bytes
typedef struct xingHeader_tag
{
    int32 hId;            // from MPG Header (0=Mpeg2, 1=MPEG1)
    int32 sampRate;       // determined from header
    int32 flags;          // from Xing Header data
    int32 frames;         // total bitstream frames from xing header
    int32 bytes;          // total bitstream bytes from Xing header
    int32 vbr_scale;      // encoded VBR scale from Xing header data
    int32 TOC[100];		  // pointer to TOC[100], maybe NULL
} XINGHeaderType;
// End of XING VBR Header Support

typedef struct vbriHeader_tag
{
    int32 hId;
    int32 vId;
    int32 delay;
    int32 bytes;
    int32 frames;
    int32 entriesTOC;
    int32 scale;
    int32 sTableEntry;
    int32 fTableEntry;
    int32 *TOC;
    int32 sampleRate;
}VBRIHeaderType;


typedef enum VBRType
{
    EXINGType,
    EVBRIType,
    ECBRType,
    EVBRType,  // to take care of case where there is no xing or VBRI hdr present
    EINVALIDType
}   MP3FileType;


/**
 *  @brief The MP3Parser Class is the class that parses the
 *  MP3 file.
 */

class MP3Parser
{

    public:
        /**
        * @brief Constructor.
        *
        * @param Filehandle
        * @returns None
        */
        MP3Parser(PVFile* aFileHandle = NULL);

        /**
        * @brief Destructor.
        *
        * @param None
        * @returns None
        */
        ~MP3Parser();

        /**
        * @brief Parses the MetaData (beginning or end) and positions
        * the file pointer at the first audio frame.
        *
        * @param fpUsed Pointer to file
        * @param enableCRC, CRC check flag
        * @returns error type.
        */
        MP3ErrorType	ParseMP3File(PVFile * fpUsed, bool enableCRC);

        /**
        * @brief Checks the file is valid mp3 clip or not
        *
        * @param fpUsed Pointer to file
        * @param filename Name of file
        * @returns error type.
        */
        MP3ErrorType IsMp3File(MP3_FF_FILE* aFile, uint32 aInitSearchFileSize);

        /**
        * @brief Sets the file size to the parser
        *
        * @param aFileSize
        * @returns error type.
        */
        MP3ErrorType SetFileSize(const uint32 aFileSize);

        /**
        * @brief Attempts to read in the number of MP3 frames specified by n.
        * It formats and stores the data read in the GAU structure.
        *
        * @param n Pointer to the number of frames to read
        * @param pgau Pointer to data read
        * @returns Number of bytes read
        */
        int32   GetNextBundledAccessUnits(uint32 *n, GAU *pgau, MP3ErrorType &err);

        /**
        * @brief Peeks into the next MP3 frames specified by n.
        * It formats and stores the data read in the MediaMetaInfo structure.
        *
        * @param n Pointer to the number of frames to check
        * @param mInfo Pointer to info read
        * @returns Number of bytes read
        */
        int32   PeekNextBundledAccessUnits(uint32 *n, MediaMetaInfo *mInfo);

        /**
        * @brief Reads the next frame from the file
        *
        * @param buf Buffer to read the frame data into
        * @param size Size of the buffer
        * @param framesize Size of the frame data if the read is successful
        * @param timestamp Timestamp for the frame if the read is successful
        * @returns Result of operation: true=success; false=fail
        */
        MP3ErrorType  GetNextMediaSample(uint8 *buf, uint32 bufsize, uint32& framesize, uint32& timestamp);

        /**
        * @brief Returns the timestamp of the frame
        *
        * @param number of the frame to retrieve the timestamp of
        * @returns Timestamp
        */
        uint32  GetTimestampForSample(int32 frameNumber) const;
        uint32  GetTimestampForCurrentSample() const;

        /**
        * @brief Moves the file pointer to the specified timestamp
        *
        * @param timestamp Time to seek to
        * @returns Timestamp seeked to
        */
        uint32  SeekToTimestamp(uint32 timestamp);

        /**
        * @brief Queries the seek point timestamp corresponding to the specified timestamp
        *
        * @param timestamp Time to seek to
        * @param frameNumber Number of frame associated with the seek
        * @returns Timestamp closest to the specified timestamp. If timestamp is past end of clip, timestamp is 0.
        */
        uint32 SeekPointFromTimestamp(uint32 &timestamp);

        /**
        * @brief Queries the offset corressponding to the given timestamp
        *
        * @param timestamp Time to seek to
        * @param frameNumber Number of frame associated with the seek
        * @returns offset
        */
        uint32 GetFileOffsetForAutoResume(uint32 &timestamp);

        /**
        * @brief Reads the MP3 header. This contains audio settings
        * necessary to configure the device.
        *
        * @param pMP3COnfig Data structure that will contain the header
        * @returns True if successful; False otherwise
        */
        bool GetMP3FileHeader(MP3ConfigInfoType * pMP3COnfig);

        /**
        * @brief Retreives and returns channel mode for the current
        * of mp3 file
        *
        * @param
        * @returns
        */
        uint32 GetChannelMode() const;

        /**
        * @brief Returns the content of decoder specific info.
        *
        * @param None
        * @returns Decoder specific info
        */
        uint8 const * GetDecoderSpecificInfoContent() const;

        /**
        * @brief Returns the size of decoder specific info.
        *
        * @param None
        * @returns Decoder specific info size
        */
        uint32  GetDecoderSpecificInfoSize();

        /**
        * @brief Returns the number of frames in the clip.
        *
        * @param None
        * @returns Number of frames
        */
        uint32  GetSampleCountInFile();

        /**
        * @brief Returns the maximum decode buffer size used.
        *
        * @param None
        * @returns Buffer size
        */
        uint32  GetMaximumDecodeBufferSize();

        /**
        * @brief Returns the size of the file.
        *
        * @param None
        * @returns File size
        */
        uint32  GetFileSize();

        /**
        * @brief Returns the duration of the file.
        *
        * @param None
        * @returns Clip duration
        */
        uint32  GetDuration(bool aMetadataDuration = false);

        /**
        * @brief Returns the approximate duration of downloaded data.
        *
        * @param aFileSize, aNPTInMS
        * @returns aNPTInMS
        */
        int32 ConvertSizeToTime(uint32 aFileSize, uint32& aNPTInMS);

        /**
        * @brief outputs the size of id3v2 tags
        *
        * @param aSize, carries the tag size
        * @returns success if meta data is parsed, failure otherwise
        */
        MP3ErrorType GetMetadataSize(uint32 &aSize);

        /**
        * @brief Retrieves minimum bytes required for getting the config info
        *
        * @param
        * @returns byte size of firstframe and id3 tags.
        */
        uint32 GetMinBytesRequired(bool aNextBytes = false);

        /**
        * @brief Copies the metadata to the structure specified.
        *
        * @param None
        * @returns Always 0.
        */
        uint32  GetMetaData(PvmiKvpSharedPtrVector &pMetaData)
        {
            iId3TagParser.GetID3Frames(iId3Frames);
            pMetaData = iId3Frames;
            return 0;
        }
        uint32 GetMetaData(const OSCL_String& pFrameType, PvmiKvpSharedPtrVector &pMetaData)
        {
            iId3TagParser.GetID3Frame(pFrameType, pMetaData);
            return 0;
        }
        bool IsID3Frame(const OSCL_String& pFrameType)
        {
            return iId3TagParser.IsID3FrameAvailable(pFrameType);
        }

        MP3ErrorType ScanMP3File(PVFile * fpUsed, uint32 aFramesToScan);
    private:
        MP3ErrorType ScanMP3File(PVFile* fpUsed);
        MP3ErrorType GetDurationFromVBRIHeader(uint32 &aDuration);
        MP3ErrorType GetDurationFromRandomScan(uint32 &aDuration);
        MP3ErrorType ComputeDurationFromNRandomFrames(PVFile * fpUsed, int32 aNumFrames = MIN_RANDOM_FRAMES_TO_SCAN, int32 aNumRandomLoc = MIN_RANDOM_LOCATION_TO_SCAN);
        void GetDurationFromCompleteScan(uint32 &aClipDuration);

        MP3ErrorType EstimateDurationFromExternalFileSize(uint32 &aClipDuration);

        uint32 GetDurationFromMetadata();

        void FillTOCTable(uint32 aFilePos, uint32 aTimeStampToFrame);

        //duration related values
        uint32 iClipDurationInMsec;
        uint32 iClipDurationFromEstimation;
        uint32 iClipDurationComputed;
        uint32 iClipDurationFromVBRIHeader;
        uint32 iClipDurationFromRandomScan;
        uint32 iClipDurationFromMetadata;
        bool iDurationScanComplete;
        uint32 iTimestamp;
        int32 iAvgBitrateInbpsFromRandomScan;
        int32 iAvgBitrateInbps;
        int32 iAvgBitrateInbpsFromCompleteScan;


    protected:
        uint32 iLocalFileSize;
        uint32 iFileSizeFromExternalSource;
        uint32 iInitSearchFileSize;
        bool iLocalFileSizeSet;
        PVFile * fp;
        MediaClockConverter iClockConverter;
        bool iFirstScan;
        uint32 iLastScanPosition;
        bool iScanEnable;

    private:
        MP3ErrorType IsValidFrame(uint8 * pBuffer, uint32 offset, uint32 seekPoint, PVFile* aFile = NULL);
        MP3ErrorType IsValidFrameHeader(uint8 * mp3FrameHeader, bool &bCRCPresent, uint32 offset, uint32 seekPoint, PVFile* aFile = NULL);
        bool  IsValidFrameCRC(uint8 *pFrame, int32 size, uint32 crcValue);

        bool GetMP3Header(uint32 &fh, MP3HeaderType &hi);
        bool DecodeMP3Header(MP3HeaderType &aHeaderInfoType, MP3ConfigInfoType &aConfigInfoType, bool aComputeAvgBitrate);
        bool DecodeXINGHeader(uint8 *XingBuffer, XINGHeaderType &mp3XingHI, MP3HeaderType &hi);
        bool DecodeVBRIHeader(uint8 * VbriBuffer, VBRIHeaderType &vbriHDI, MP3HeaderType &hi);
        MP3ErrorType mp3FindSync(uint32 seekPoint, uint32 &syncOffset, PVFile* aFile = NULL);
        uint16 CalcCRC16(uint8* pBuffer, uint32 dwBitSize);
        MP3ErrorType mp3VerifyCRC(MP3HeaderType mp3HdrInfo, MP3ConfigInfoType mp3CI);


        /* scan file related */
        uint32 iScannedFrameCount;
        /* toc related */
        int32* iTOC;
        uint32 iTOCFilledCount;
        uint32 iTimestampPrev;
        uint32 iScanTimestamp;
        uint32 iBinWidth;

        uint32 iSamplingRate;
        uint32 iSamplesPerFrame;

        int32 CalculateBufferSizeForHeader(uint8 *VbriHead);
        int32 iCurrFrameNumber;
        int32 iNumberOfFrames;

        bool bVBRFile;
        uint8 ConfigData[4];
        uint32 ConfigSize;
        uint32 StartOffset;
        uint8 * pSyncBuffer;
        uint32 iMaxSyncBufferSize;

        MP3ConfigInfoType  iMP3ConfigInfo;
        MP3HeaderType      iMP3HeaderInfo;
        XINGHeaderType     iXingHeader;
        PVID3ParCom    iId3TagParser;
        VBRIHeaderType iVbriHeader;
        bool iEnableCrcCalc;
        MP3FileType mp3Type;
        uint32 iTagSize;
        PvmiKvpSharedPtrVector iId3Frames;
};

#endif // #ifdef MP3PARSER_H_INCLUDED
