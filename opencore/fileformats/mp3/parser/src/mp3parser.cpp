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
//
//                       M P 3   P A R S E R
//
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


/**
 *  @file mp3parser.cpp
 *  @brief This file contains the implementation of the actual MP3
 *  file parser.
 */
/***********************************************************************
 * Include Files
 ***********************************************************************/
#include "mp3parser.h"
#include "mp3fileio.h"

#include "oscl_mem.h"
#include "oscl_stdstring.h"
#include "oscl_utf8conv.h"

/***********************************************************************
 * Constant Defines
 ***********************************************************************/
// Maximum debug message length
#define KMAXMSGSIZE		1024
#define KMAX_MP3FRAME_LENGTH_IN_BYTES	2884
// Initial search range, resetted to the file size once valid mp3
// frame is found
#define KMAX_INITIAL_SEARCH_FILE_SIZE_IN_BYTES	500000

/***********************************************************************
 * Global constant definitions
 ***********************************************************************/

/***********************************************************************
 * XING VBR Header Constants
 ***********************************************************************/
static const uint32 FRAMES_FLAG      = 0x0001;
static const uint32 BYTES_FLAG       = 0x0002;
static const uint32 TOC_FLAG         = 0x0004;
static const uint32 VBR_SCALE_FLAG   = 0x0008;
static const uint32 FRAMES_AND_BYTES = (FRAMES_FLAG | BYTES_FLAG);

static const uint8	MPEG_LAYER_I				= 0x03;
static const uint8	MPEG_LAYER_II				= 0x02;
static const uint8	MPEG_LAYER_III				= 0X01;

static const uint8  CHANNEL_MODE_JOINT_STEREO	= 0x01;
static const uint8  CHANNEL_MODE_STEREO			= 0x00;
static const uint8  CHANNEL_MODE_DUAL_CHANNEL	= 0x02;
static const uint8  CHANNEL_MODE_MONO			= 0x03;

static const uint8	FRAME_VESION_MPEG_1			= 0x03;
static const uint8	FRAME_VESION_MPEG_2			= 0x02;
static const uint8	FRAME_VESION_MPEG_2_5		= 0x00;

static const uint8	MP3_FRAME_HEADER_SIZE				= 0x04;
static const uint32	MP3_FIRST_FRAME_SIZE				= 128;
/***********************************************************************
 * End XING VBR Header Constants
 ***********************************************************************/

/***********************************************************************
 * MP3 Frame Header Constants
 ***********************************************************************/
// MP3 Frame Header Format
// AAAAAAAA AAABBCCD EEEEFFGH IIJJKLMM
// Bits 31-21 (A)
static const uint32 FRAME_SYNC_MASK       = 0xFFE00000;
static const uint32 FRAME_SYNC_SHIFT      = 21;

// Bits 20-19 (B)
static const uint32 FRAME_VER_ID_MASK     = 0x00180000;
static const uint32 FRAME_VER_ID_SHIFT    = 19;

// Bits 18-17 (C)
static const uint32 FRAME_LAYER_ID_MASK   = 0x00060000;
static const uint32 FRAME_LAYER_ID_SHIFT  = 17;

// Bits 16    (D)
static const uint32 FRAME_PROT_MASK       = 0x00010000;
static const uint32 FRAME_PROT_SHIFT      = 16;

// Bits 15-12 (E)
static const uint32 FRAME_BR_INDEX_MASK   = 0x0000F000;
static const uint32 FRAME_BR_INDEX_SHIFT  = 12;

// Bits 11-10 (F)
static const uint32 FRAME_SR_FREQ_MASK    = 0x00000C00;
static const uint32 FRAME_SR_FREQ_SHIFT   = 10;

// Bits 9     (G)
static const uint32 FRAME_PADDING_MASK    = 0x00000200;
static const uint32 FRAME_PADDING_SHIFT   = 9;

// Bits 8     (H)
static const uint32 FRAME_PRIVATE_MASK    = 0x00000100;
static const uint32 FRAME_PRIVATE_SHIFT   = 8;

// Bits 7-6   (I)
static const uint32 FRAME_CH_MODE_MASK    = 0x000000C0;
static const uint32 FRAME_CH_MODE_SHIFT   = 6;

// Bits 5-4   (J)
static const uint32 FRAME_MODE_EXTN_MASK  = 0x00000030;
static const uint32 FRAME_MODE_EXTN_SHIFT = 4;

// Bits 3     (K)
static const uint32 FRAME_COPYRIGHT_MASK  = 0x00000008;
static const uint32 FRAME_COPYRIGHT_SHIFT = 3;

// Bits 2     (L)
static const uint32 FRAME_ORIGINAL_MASK   = 0x00000004;
static const uint32 FRAME_ORIGINAL_SHIFT  = 2;

// Bits 1-0   (M)
static const uint32 FRAME_EMPHASIS_MASK   = 0x00000003;
static const uint32 FRAME_EMPHASIS_SHIFT  = 0;
/***********************************************************************
 * End MP3 Frame Header Constants
 ***********************************************************************/

/***********************************************************************
 * BitRate Index Table (Version 1)
 ***********************************************************************/
static const int32 brIndexTableV1[4][16] =
{
    // RESERVED
    {0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0},
    // Version 1, Layer III
    {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0},
    // Version 1, Layer II
    {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0},
    // Version 1, Layer I
    {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0}
};

/***********************************************************************
 * BitRate Index Table (Versions 2 and 2.5)
 ***********************************************************************/
static const int32 brIndexTableV2[4][16] =
{
    // RESERVED
    {0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0},
    // Version 2, Layer III
    {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},
    // Version 2, Layer II
    {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},
    // Version 2, Layer I
    {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0}
};

/***********************************************************************
 * SampleRate Index Table
 ***********************************************************************/
static const int32 srIndexTable[] =
{
    // MPEG 2.5
    11025, 12000, 8000, 0,
    // RESERVED
    0, 0, 0, 0,
    // MPEG 2
    22050, 24000, 16000, 0,
    // MPEG 1
    44100, 48000, 32000, 0
};

/***********************************************************************
 * FrameRate Index Table 10 * sample rate / samples per frame
 ***********************************************************************/
static const int32 frIndexTable[4][3] =
{
    // MPEG Version 2.5
    { 385 / 2, 210, 278 / 2 },
    // RESERVED
    { 0, 0, 0 },
    // MPEG Versions 2
    { 385, 418, 278 },
    // MPEG Version 1
    { 385, 418, 278 }
};

/***********************************************************************
 * Samples Per Frame Index Table
 ***********************************************************************/
static const int32 spfIndexTable[4][4] =
{
    // MPEG 2.5
    {0,  576, 1152, 384},
    // RESERVED
    {0,    0,    0,   0},
    // MPEG 2
    {0,  576, 1152, 384},
    // MPEG 1
    {0, 1152, 1152, 384}
};


/***********************************************************************
 * FUNCTION:    SwapFileToHostByteOrderInt32
 * DESCRIPTION: Swaps the bytes in an integer to the correct host
 *				byte order
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
static uint32 SwapFileToHostByteOrderInt32(uint8 * pBuf2)
{
    uint32  temp;
    uint8 * pBuf1 = (uint8 *) & temp;

#if (OSCL_BYTE_ORDER_LITTLE_ENDIAN)
    pBuf1[3] = pBuf2[0];
    pBuf1[2] = pBuf2[1];
    pBuf1[1] = pBuf2[2];
    pBuf1[0] = pBuf2[3];
#elif (OSCL_BYTE_ORDER_BIG_ENDIAN)
    pBuf1[3] = pBuf2[3];
    pBuf1[2] = pBuf2[2];
    pBuf1[1] = pBuf2[1];
    pBuf1[0] = pBuf2[0];
#endif

    return temp;
}

/***********************************************************************
 * FUNCTION:    ReadBuffer
 * DESCRIPTION: Read data from buffer
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
static uint32 ReadBuffer(uint8 * pBuf2, int32 length, int32 &pos)
{
    int32 i, b, number = 0;

    if (pBuf2)
    {
        for (i = 0;  i < length ; i++)
        {
            b = length - 1 - i  ;
            number = number | (uint32)(pBuf2[pos+i] & 0xff) << (8 * b);
        }
        pos += length ;
        return number;
    }
    else
    {
        return 0;
    }
}

/***********************************************************************
 * FUNCTION:    SwapFileToHostByteOrderInt16
 * DESCRIPTION: Swaps the bytes in an integer to the correct host byte order
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
static uint16 SwapFileToHostByteOrderInt16(uint8 * pBuf2)
{
    uint16  temp;
    uint8 * pBuf1 = (uint8 *) & temp;

    oscl_memcpy(&temp, pBuf2, 2);

#if (OSCL_BYTE_ORDER_LITTLE_ENDIAN)
    pBuf1[1] = pBuf2[0];
    pBuf1[0] = pBuf2[1];
#elif (OSCL_BYTE_ORDER_BIG_ENDIAN)
    pBuf1[1] = pBuf2[1];
    pBuf1[0] = pBuf2[0];
#endif

    return temp;
}

/***********************************************************************
 * FUNCTION:    Constructor
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
MP3Parser::MP3Parser(PVFile* aFileHandle)
{
    fp = aFileHandle;
    // initialize all member variables
    iLocalFileSize = 0;
    iFileSizeFromExternalSource = 0;
    iInitSearchFileSize = KMAX_INITIAL_SEARCH_FILE_SIZE_IN_BYTES;
    iLocalFileSizeSet = false;
    iEnableCrcCalc = false;
    mp3Type = EVBRType;
    iTagSize = 0;
    StartOffset = 0;
    iCurrFrameNumber = 0;
    ConfigSize = 0;
    iNumberOfFrames = 0;
    // average bitrate values
    iAvgBitrateInbps = 0;
    iAvgBitrateInbpsFromRandomScan = 0;
    iAvgBitrateInbpsFromCompleteScan = 0;
    iScannedFrameCount = 0;
    // scan completion flag
    iDurationScanComplete = false;
    // duration values from various sources
    iClipDurationInMsec = 0;
    iClipDurationFromEstimation = 0;
    iClipDurationComputed = 0;
    iClipDurationFromVBRIHeader = 0;
    iClipDurationFromRandomScan = 0;
    iClipDurationFromMetadata = 0;

    iSamplesPerFrame = 0;
    iSamplingRate = 0;

    iTimestamp = 0;
    iFirstScan = true;

    iTOC = NULL;
    iTOCFilledCount = 0;
    iTimestampPrev = 0;
    iScanTimestamp = 0;
    iBinWidth = 0;

    iVbriHeader.TOC = NULL;
    oscl_memset(&iMP3ConfigInfo, 0, sizeof(iMP3ConfigInfo));
    oscl_memset(&iMP3HeaderInfo, 0, sizeof(iMP3HeaderInfo));
    oscl_memset(&iXingHeader, 0, sizeof(iXingHeader));
    oscl_memset(ConfigData, 0, sizeof(ConfigData));
    oscl_memset(&iVbriHeader, 0, sizeof(iVbriHeader));
    pSyncBuffer = NULL;
}


/***********************************************************************
 * FUNCTION:    Destructor
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
MP3Parser::~MP3Parser()
{
    // The File Pointer is only used. FileHandles are opened and closed
    // as required.
    fp = NULL;
    iClipDurationInMsec = 0;
    iClipDurationComputed = 0;
    iLocalFileSize = 0;
    iLocalFileSize = false;
    iInitSearchFileSize = 0;
    iCurrFrameNumber = 0;
    iNumberOfFrames = 0;
    ConfigSize = 0;
    StartOffset = 0;
    if (iVbriHeader.TOC != NULL)
        OSCL_ARRAY_DELETE(iVbriHeader.TOC);
    if (pSyncBuffer)
    {
        OSCL_ARRAY_DELETE(pSyncBuffer);
        pSyncBuffer = NULL;
    }
    iMaxSyncBufferSize = 0;

    if (iTOC)
    {
        OSCL_ARRAY_DELETE(iTOC);
        iTOC = NULL;
    }

    oscl_memset(&iMP3ConfigInfo, 0, sizeof(iMP3ConfigInfo));
    oscl_memset(&iMP3HeaderInfo, 0, sizeof(iMP3HeaderInfo));
    oscl_memset(&iXingHeader, 0, sizeof(iXingHeader));
    oscl_memset(&ConfigData, 0, sizeof(ConfigData));
    oscl_memset(&iVbriHeader, 0, sizeof(iVbriHeader));
}

/***********************************************************************
 * FUNCTION:    ParseMP3File
 * DESCRIPTION: This function MUST be called after the Constructor and before
 *				any other public function is called. Otherwise the object's
 *				member data will be uninitialized.
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
MP3ErrorType MP3Parser::ParseMP3File(PVFile * fpUsed, bool aEnableCRC)
{
    //init members
    iEnableCrcCalc = aEnableCRC;
    fp = fpUsed;

    iLocalFileSize = 0;
    iLocalFileSizeSet = false;
    iCurrFrameNumber = 0;
    iNumberOfFrames = 0;
    ConfigSize = 0;
    StartOffset = 0;

    uint32 firstHeader = 0;
    uint8 pFirstFrame[MP3_FIRST_FRAME_SIZE];
    uint8 pFrameHeader[MP3_FRAME_HEADER_SIZE];

    uint8 * pBuf = pFirstFrame;
    oscl_memset(&iMP3ConfigInfo, 0, sizeof(iMP3ConfigInfo));
    oscl_memset(&iMP3HeaderInfo, 0, sizeof(iMP3HeaderInfo));
    oscl_memset(&iXingHeader, 0, sizeof(iXingHeader));
    oscl_memset(&ConfigData, 0, sizeof(ConfigData));
    oscl_memset(&iVbriHeader, 0, sizeof(iVbriHeader));

    MP3ErrorType errCode = MP3_SUCCESS;
    // SAVE THE CURRENT FILE POSITION
    errCode = MP3Utils::SeektoOffset(fp, 0, Oscl_File::SEEKSET);
    // try to retrieve the file size
    if (fp->GetCPM() == NULL && MP3Utils::getCurrentFileSize(fp, iLocalFileSize))
    {
        iLocalFileSizeSet = true;
        iInitSearchFileSize = OSCL_MIN(iInitSearchFileSize, iLocalFileSize);
        if (iLocalFileSize == 0)
        {
            return MP3_END_OF_FILE;
        }
    }

    if (!iLocalFileSizeSet)
    {
        uint32 remBytes = 0;
        if (fp->GetRemainingBytes(remBytes))
        {
            iInitSearchFileSize = OSCL_MIN(iInitSearchFileSize, remBytes);
        }
    }

    if (fp->GetFileBufferingCapacity() <= 0)
    {
        // Parse the MetaData (Beginning or End)
        // Position the File Pointer at the first Audio Frame
        if (iId3TagParser.ParseID3Tag(fp) == PVMFSuccess)
        {
            // This is the position of the first MP3 Frame in the File
            if (iId3TagParser.IsID3V2Present())
            {
                iTagSize = iId3TagParser.GetByteOffsetToStartOfAudioFrames();
            }
        }
    }
    else
    {
        // get id3 tag size only
        iId3TagParser.IsID3V2Present(fp, iTagSize);
    }

    if (iTagSize > 0)
    {
        StartOffset = iTagSize;
    }

    errCode = MP3Utils::SeektoOffset(fp, StartOffset, Oscl_File::SEEKSET);
    if (MP3_SUCCESS != errCode)
    {
        return errCode;
    }

    if (!MP3FileIO::readByteData(fp, MP3_FRAME_HEADER_SIZE, (uint8 *)pFrameHeader))
    {
        return MP3_INSUFFICIENT_DATA;
    }

    firstHeader = SwapFileToHostByteOrderInt32(pFrameHeader);

    /**
     * If we don't find a valid MP3 Marker point we will attempt recovery.
     * Either the ID3 parsing failed or the ID3 header(s) are incorrect.
     * If this fails we won't be able to parse the rest of the file.
     * Attempt recovery now
     **/
    if (!GetMP3Header(firstHeader, iMP3HeaderInfo))
    {
        uint32 seekOffset = 0;
        MP3Utils::SeektoOffset(fp, 0 - MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);
        MP3ErrorType err = mp3FindSync(StartOffset, seekOffset, fp);
        if (err == MP3_SUCCESS)
        {
            StartOffset += seekOffset;
            MP3ErrorType err = MP3Utils::SeektoOffset(fp, seekOffset, Oscl_File::SEEKCUR);
            if (MP3_SUCCESS != err)
            {
                return err;
            }

            if (!MP3FileIO::readByteData(fp, MP3_FRAME_HEADER_SIZE, pFrameHeader))
            {
                return MP3_INSUFFICIENT_DATA;
            }
            firstHeader = SwapFileToHostByteOrderInt32(pFrameHeader);

            if (!GetMP3Header(firstHeader, iMP3HeaderInfo))
            {
                return MP3_FILE_HDR_READ_ERR;
            }
        }
        else if (err == MP3_INSUFFICIENT_DATA ||
                 err == MP3_END_OF_FILE)
        {
            // in eof scenario parser reports eof error to the user
            // eof will be reported in case when no valid sync
            // word is find in the maximum specified search limit
            return err;
        }
    }

    if (!DecodeMP3Header(iMP3HeaderInfo, iMP3ConfigInfo, false))
    {
        return MP3_FILE_HDR_DECODE_ERR;
    }
    else
    {
        oscl_memcpy(ConfigData, pFrameHeader, MP3_FRAME_HEADER_SIZE);
        ConfigSize = MP3_FRAME_HEADER_SIZE;
    }

    int32 revSeek = 0 - MP3_FRAME_HEADER_SIZE;
    errCode = MP3Utils::SeektoOffset(fp, revSeek, Oscl_File::SEEKCUR);
    if (MP3_SUCCESS != errCode)
    {
        return errCode;
    }

    if (!MP3FileIO::readByteData(fp, MP3_FIRST_FRAME_SIZE, pFirstFrame))
    {
        return MP3_INSUFFICIENT_DATA;
    }

    revSeek = 0 - MP3_FIRST_FRAME_SIZE;
    //VBRI header exist exactly 32 bytes after first frame header
    if ((oscl_memcmp((pBuf + VBRI_HEADER_OFFSET), STR_VBRI_HEADER_IDENTIFIER, VBR_HEADER_SIZE) == 0))
    {
        int32 bufferSize = CalculateBufferSizeForHeader(pBuf + 36);
        int32 actualBufferSize = bufferSize + VBRI_HEADER_OFFSET + VBR_HEADER_SIZE;
        uint8* tempBuf = OSCL_ARRAY_NEW(uint8, actualBufferSize);

        MP3ErrorType err = MP3Utils::SeektoOffset(fp, revSeek, Oscl_File::SEEKCUR);
        if (MP3_SUCCESS != err)
        {
            OSCL_ARRAY_DELETE(tempBuf);
            return err;
        }

        if (!MP3FileIO::readByteData(fp, actualBufferSize, tempBuf))
        {
            OSCL_ARRAY_DELETE(tempBuf);
            return MP3_INSUFFICIENT_DATA;
        }
        else
        {
            if (pBuf)
            {
                pBuf = NULL;
                pBuf = tempBuf;
                pBuf += 36;
            }
        }

        revSeek = 0 - actualBufferSize;
        if (!DecodeVBRIHeader(pBuf, iVbriHeader, iMP3HeaderInfo))
        {
            OSCL_ARRAY_DELETE(tempBuf);
            return MP3_FILE_VBRI_HDR_ERR;
        }
        else
        {
            mp3Type = EVBRIType;
            iLocalFileSize = iVbriHeader.bytes;
            OSCL_ARRAY_DELETE(tempBuf);
        }
    }
    else
    {
        uint32 offset = 0;
        // Determine offset of XING headers
        if ((iMP3HeaderInfo.layerID == MPEG_LAYER_III))
        {
            // MPEG 1
            if (iMP3HeaderInfo.frameVer == 3) //MPEG version 1
            {
                if (iMP3HeaderInfo.chMode != 3)
                {
                    offset = (32 + 4);
                    pBuf += offset;
                }
                else
                {
                    offset = (17 + 4);
                    pBuf += offset;
                }
            }
            else
            {
                // MPEG 2
                if (iMP3HeaderInfo.chMode != 3)
                {
                    offset = (17 + 4);
                    pBuf += offset;
                }
                else
                {
                    offset = (9 + 4);
                    pBuf += offset;
                }
            }
        }

        // Check for MP3 Header Tags, XING or INFO
        if ((oscl_memcmp(pBuf, STR_XING_HEADER_IDENTIFIER, VBR_HEADER_SIZE) == 0) ||
                (oscl_memcmp(pBuf, STR_INFO_HEADER_IDENTIFIER, VBR_HEADER_SIZE) == 0))
        {
            MP3ErrorType err = MP3Utils::SeektoOffset(fp, offset - MP3_FIRST_FRAME_SIZE, Oscl_File::SEEKCUR);
            if (MP3_SUCCESS != err)
            {
                return err;
            }
            revSeek = 0 - offset;

            if (!MP3FileIO::readByteData(fp, MP3_FIRST_FRAME_SIZE, pFirstFrame))
            {
                return MP3_INSUFFICIENT_DATA;
            }

            revSeek -= MP3_FIRST_FRAME_SIZE;

            if (!DecodeXINGHeader(pFirstFrame, iXingHeader, iMP3HeaderInfo))
                return MP3_FILE_XING_HDR_ERR;
            else
                mp3Type = EXINGType;
        }
    }

    // Calculate the sampling rate and samples per frame.
    iSamplesPerFrame = spfIndexTable[iMP3HeaderInfo.frameVer][iMP3HeaderInfo.layerID];
    iSamplingRate = srIndexTable[((iMP3HeaderInfo.frameVer)*4) + iMP3HeaderInfo.srIndex];

    if (mp3Type != EXINGType && mp3Type != EVBRIType)
    {
        // if XING or VBRI Headers are not present then we need to build our own TOC for
        // repositioning.
        iTOC = OSCL_ARRAY_NEW(int32, MAX_TOC_ENTRY_COUNT + 1);
        oscl_memset(iTOC, 0, sizeof(iTOC));
    }

    iAvgBitrateInbps = iMP3ConfigInfo.BitRate;
    // Set the position to the position of the first MP3 frame
    errCode = MP3Utils::SeektoOffset(fp, revSeek, Oscl_File::SEEKCUR);
    if (MP3_SUCCESS != errCode)
    {
        return errCode;
    }
    iCurrFrameNumber = 0;
    return MP3_SUCCESS;
}

/***********************************************************************
 *	Function : ScanMP3File
 *	Purpose  : Fetches duration of the clip playing
 *			   Duration is returned, by different
 *			   means by the pre-defined priorities
 *  Input	 : fpUsed
 *  Output	 : iClipDurationComputed
 *	Return   : error code
 *	Modified :
 ***********************************************************************/
MP3ErrorType MP3Parser::ScanMP3File(PVFile * fpUsed, uint32 aFramesToScan)
{
    uint32 firstHeader = 0;
    uint8 pFrameHeader[MP3_FRAME_HEADER_SIZE];
    int32 audioOffset = 0;
    uint32 seekOffset = 0;
    MP3HeaderType mp3HeaderInfo;
    MP3ConfigInfoType mp3ConfigInfo;

    if (iClipDurationFromMetadata || (iClipDurationFromVBRIHeader &&
                                      ((iVbriHeader.entriesTOC >= 0) ||
                                       (iXingHeader.flags & TOC_FLAG)))
       )
    {
        // if Duration can be obtained from either VBRI/XING Headers or from metadata,
        // we will not scan the file for duration
        return MP3_DURATION_PRESENT;
    }

    if (iTOCFilledCount == MAX_TOC_ENTRY_COUNT)
    {
        FillTOCTable(0, 0);
        return MP3_SUCCESS;
    }

    if (iFirstScan)
    {
        if (iTagSize > 0)
        {
            audioOffset = StartOffset;
            MP3ErrorType err = MP3Utils::SeektoOffset(fpUsed, audioOffset, Oscl_File::SEEKSET);
            if (MP3_SUCCESS != err)
            {
                return err;
            }
        }
        iFirstScan = false;
    }
    else
    {
        audioOffset = iLastScanPosition;
    }

    // Set length of initial search to the min between default and filesize
    iInitSearchFileSize = OSCL_MIN(iInitSearchFileSize, iLocalFileSize);

    uint32 numFrames = 0;
    int32 bitrate = 0;
    uint32 frameDur = 0;

    while (numFrames < aFramesToScan)
    {
        firstHeader = 0;
        oscl_memset(&pFrameHeader, 0, sizeof(pFrameHeader));

        if (fpUsed->Read(pFrameHeader, 1, MP3_FRAME_HEADER_SIZE) != MP3_FRAME_HEADER_SIZE)
        {
            if (fpUsed->GetCPM() == NULL)
            {
                iDurationScanComplete = true;
            }
            FillTOCTable(audioOffset, 0);
            return MP3_INSUFFICIENT_DATA;
        }
        firstHeader = SwapFileToHostByteOrderInt32(pFrameHeader);
        uint32 offset = MP3Utils::getCurrentFilePosition(fpUsed);
        if (!GetMP3Header(firstHeader, mp3HeaderInfo))
        {
            MP3Utils::SeektoOffset(fp, 0 - MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);
            MP3ErrorType err = mp3FindSync(offset, seekOffset, fpUsed);
            if (err == MP3_SUCCESS)
            {
                offset += seekOffset;
                err = MP3Utils::SeektoOffset(fpUsed, seekOffset, Oscl_File::SEEKCUR);
                if (MP3_SUCCESS != err)
                {
                    return err;
                }

                if (!MP3FileIO::readByteData(fpUsed, MP3_FRAME_HEADER_SIZE, pFrameHeader))
                {
                    iDurationScanComplete = true;
                    FillTOCTable(offset, iScanTimestamp);
                    return MP3_INSUFFICIENT_DATA;
                }

                firstHeader = SwapFileToHostByteOrderInt32(pFrameHeader);
                if (! GetMP3Header(firstHeader, mp3HeaderInfo))
                {
                    iDurationScanComplete = true;
                    FillTOCTable(offset, iScanTimestamp);
                    return MP3_FILE_HDR_READ_ERR;
                }
            }
            else
            {
                iDurationScanComplete = true;
                FillTOCTable(offset, iScanTimestamp);
                return err;
            }
        }

        if (!DecodeMP3Header(mp3HeaderInfo, mp3ConfigInfo, false))
        {
            iDurationScanComplete = true;
            FillTOCTable(offset, iScanTimestamp);
            return MP3_FILE_HDR_DECODE_ERR;
        }

        MP3Utils::SeektoOffset(fpUsed, mp3ConfigInfo.FrameLengthInBytes - MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);
        bitrate = mp3ConfigInfo.BitRate;
        frameDur = frameDur + (uint32)((OsclFloat) mp3ConfigInfo.FrameLengthInBytes * 8000.00f / mp3ConfigInfo.BitRate);
        iLastScanPosition = fpUsed->Tell();
        numFrames++;
        iScannedFrameCount++;

        if (iScannedFrameCount > 1)
        {
            if (bitrate != iAvgBitrateInbpsFromCompleteScan)
            {
                iAvgBitrateInbpsFromCompleteScan += (int32)((int32)bitrate - (int32)iAvgBitrateInbpsFromCompleteScan) / (int32)iScannedFrameCount;
            }
        }
        else
        {
            iAvgBitrateInbpsFromCompleteScan = bitrate;
            mp3Type = ECBRType;
        }
    }

    // After scan of frames we need to fill the TOC table
    FillTOCTable(audioOffset, iScanTimestamp);
    iScanTimestamp = iScanTimestamp + frameDur;

    return MP3_SUCCESS;
}



/***********************************************************************
 * FUNCTION:    GetMP3Header
 * DESCRIPTION: Parse Header Bit fields into a structure (Pass in 4 bytes)
 *	            Validate ranges and reserved fields.
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
bool MP3Parser::GetMP3Header(uint32 &aFrameHeader, MP3HeaderType &aMP3HeaderInfo)
{
    oscl_memset(&aMP3HeaderInfo, 0, sizeof(aMP3HeaderInfo));

    if ((aFrameHeader & FRAME_SYNC_MASK) != (FRAME_SYNC_MASK))
    {
        return false;
    }

    aMP3HeaderInfo.frameVer   = ((aFrameHeader & FRAME_VER_ID_MASK)    >> FRAME_VER_ID_SHIFT);
    aMP3HeaderInfo.layerID    = ((aFrameHeader & FRAME_LAYER_ID_MASK)  >> FRAME_LAYER_ID_SHIFT);
    aMP3HeaderInfo.crcFollows = ((aFrameHeader & FRAME_PROT_MASK)      >> FRAME_PROT_SHIFT);
    aMP3HeaderInfo.brIndex    = ((aFrameHeader & FRAME_BR_INDEX_MASK)  >> FRAME_BR_INDEX_SHIFT);
    aMP3HeaderInfo.srIndex    = ((aFrameHeader & FRAME_SR_FREQ_MASK)   >> FRAME_SR_FREQ_SHIFT);
    aMP3HeaderInfo.prvBit     = ((aFrameHeader & FRAME_PRIVATE_MASK)   >> FRAME_PRIVATE_SHIFT);
    aMP3HeaderInfo.padBit     = ((aFrameHeader & FRAME_PADDING_MASK)   >> FRAME_PADDING_SHIFT);
    aMP3HeaderInfo.chMode     = ((aFrameHeader & FRAME_CH_MODE_MASK)   >> FRAME_CH_MODE_SHIFT);
    aMP3HeaderInfo.modeExtn   = ((aFrameHeader & FRAME_MODE_EXTN_MASK) >> FRAME_MODE_EXTN_SHIFT);

    // Validate the header
    // Skip Frames with Invalid/Reserved Fields set
    if ((aMP3HeaderInfo.srIndex == 3)  || (aMP3HeaderInfo.brIndex == 15) ||
            (aMP3HeaderInfo.frameVer == 1) || (aMP3HeaderInfo.layerID != 1))    /* layerID == 1 <> layer III or mp3 */
    {
        return false;
    }

    return true;
}



/***********************************************************************
 * FUNCTION:    DecodeMP3Header
 * DESCRIPTION: Decode the MP3 Header struct and place the derived values
 *              into the supplied MP3 Config data structure.
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
bool MP3Parser::DecodeMP3Header(MP3HeaderType &aMP3HeaderInfo, MP3ConfigInfoType &aMP3ConfigInfo, bool aComputeAvgBitrate)
{
    uint32 bitRate, samplingRate;
    uint32 FrameLengthInBytes;
    uint32 FrameSizeUnComp;

    aMP3ConfigInfo.SamplingRate = 0;
    aMP3ConfigInfo.BitRate = 0;
    aMP3ConfigInfo.FrameLengthInBytes = 0;
    aMP3ConfigInfo.FrameSizeUnComp = 0;
    aMP3ConfigInfo.NumberOfChannels = 0;

    if (aMP3HeaderInfo.frameVer == 3)   // MPEG Ver 1
    {
        bitRate = 1000 * brIndexTableV1[aMP3HeaderInfo.layerID][aMP3HeaderInfo.brIndex];
    }
    else if ((aMP3HeaderInfo.frameVer == 2) || (aMP3HeaderInfo.frameVer == 0))
    {  // MPEG Ver 2.0, 2.5
        bitRate = 1000 * brIndexTableV2[aMP3HeaderInfo.layerID][aMP3HeaderInfo.brIndex];
    }
    else
    {
        return false;
    }

    if (bitRate == 0)
    {
        return false;
    }

    samplingRate = srIndexTable[((aMP3HeaderInfo.frameVer)*4) + aMP3HeaderInfo.srIndex];
    if (samplingRate == 0)
    {
        return false;
    }

    // Compressed Frame Size
    if (aMP3HeaderInfo.layerID == 3)  // Layer I
    {
        if (aMP3HeaderInfo.frameVer == 3)   // MPEG Ver 1
        {
            FrameLengthInBytes = (12 * bitRate / samplingRate + aMP3HeaderInfo.padBit) * 4;
            FrameSizeUnComp = 384;
        }
        else // MPEG Ver 2, 2.5
        {
            FrameLengthInBytes = (6 * bitRate / samplingRate + aMP3HeaderInfo.padBit) * 4;
            FrameSizeUnComp = 192;
        }
    }
    else // Layer II & III
    {
        if (aMP3HeaderInfo.frameVer == 3)   // MPEG Ver 1
        {
            FrameLengthInBytes = (144 * bitRate / samplingRate + aMP3HeaderInfo.padBit);
            FrameSizeUnComp = 1152;
        }
        else // MPEG Ver 2,2.5
        {
            FrameLengthInBytes = (72 * bitRate / samplingRate + aMP3HeaderInfo.padBit);
            FrameSizeUnComp = 576;
        }
    }

    switch (aMP3HeaderInfo.chMode)
    {
        case 0:
            aMP3ConfigInfo.NumberOfChannels = 2; // Stereo
            break;
        case 1: // Joint Ch. Stereo
        case 2: // Dual  Ch. Stereo
            aMP3ConfigInfo.NumberOfChannels = 2; // Stereo
            break;
        case 3:
            aMP3ConfigInfo.NumberOfChannels = 1; // Mono
            break;
        default:
            break;
    }

    aMP3ConfigInfo.SamplingRate = samplingRate;
    aMP3ConfigInfo.BitRate = bitRate;
    aMP3ConfigInfo.FrameLengthInBytes = FrameLengthInBytes;
    aMP3ConfigInfo.FrameSizeUnComp = FrameSizeUnComp;

    if (mp3Type != EVBRIType)
    {
        if ((mp3Type == EXINGType && !(iXingHeader.flags & TOC_FLAG) && !(iXingHeader.flags & FRAMES_FLAG)) || mp3Type == EVBRType)
        {
            if (aComputeAvgBitrate)
            {
                int32 filesize = OSCL_MAX(iFileSizeFromExternalSource, iLocalFileSize);
                uint32 audioDataSize = (filesize - StartOffset);
                if (iId3TagParser.IsID3V1Present())
                {
                    // The TAG in an ID3V1.x MP3 File is 128 bytes long
                    audioDataSize -= ID3_V1_TAG_SIZE;
                }
                iNumberOfFrames = audioDataSize / (aMP3ConfigInfo.FrameLengthInBytes);

                if (aMP3ConfigInfo.BitRate <= 0)
                {
                    return true;
                }
                if (iCurrFrameNumber == 1)
                {
                    iAvgBitrateInbps = aMP3ConfigInfo.BitRate;
                }
                if (iCurrFrameNumber > 1)
                {
                    if (aMP3ConfigInfo.BitRate != iAvgBitrateInbps)
                    {
                        iAvgBitrateInbps += (aMP3ConfigInfo.BitRate - (int32)iAvgBitrateInbps) / iCurrFrameNumber;
                    }
                }
            }
        }
    }
    return true;
}

/***********************************************************************
 * FUNCTION:    DecodeVBRIHeader
 * DESCRIPTION:	Decode VBRI Header and store TOC entries used for
				repositioning
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
bool MP3Parser::DecodeVBRIHeader(uint8 *VbriBuffer, VBRIHeaderType &vbriHDType,
                                 MP3HeaderType &aMP3HeaderInfo)
{
    uint8 * pBuf = VbriBuffer;
    int32 pos = 0;
    int32 i, tableLength;
    pBuf += 4;
    vbriHDType.hId = aMP3HeaderInfo.layerID;
    vbriHDType.sampleRate = srIndexTable[((aMP3HeaderInfo.frameVer)*4) + aMP3HeaderInfo.srIndex];
    pBuf += 6;
    vbriHDType.bytes = SwapFileToHostByteOrderInt32(pBuf);
    pBuf += 4;
    vbriHDType.frames = SwapFileToHostByteOrderInt32(pBuf);
    iNumberOfFrames = vbriHDType.frames;
    pBuf += 4;
    vbriHDType.entriesTOC = SwapFileToHostByteOrderInt16(pBuf);
    pBuf += 2;
    vbriHDType.scale = SwapFileToHostByteOrderInt16(pBuf);
    pBuf += 2;
    vbriHDType.sTableEntry = SwapFileToHostByteOrderInt16(pBuf);
    pBuf += 2;
    vbriHDType.fTableEntry = SwapFileToHostByteOrderInt16(pBuf);
    pBuf += 2;

    tableLength = vbriHDType.entriesTOC * vbriHDType.sTableEntry;

    vbriHDType.TOC = OSCL_ARRAY_NEW(int32, vbriHDType.entriesTOC + 1);

    for (i = 0;i <= (vbriHDType.entriesTOC);i++)
    {
        vbriHDType.TOC[i] = ReadBuffer(pBuf, vbriHDType.sTableEntry, pos) * vbriHDType.scale;
    }
    return true;
}


/***********************************************************************
 * FUNCTION:    DecodeXINGHeader
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
bool  MP3Parser::DecodeXINGHeader(uint8 *XingBuffer,
                                  XINGHeaderType &mp3XingHD,
                                  MP3HeaderType &hdrInfo)
{
    /*  4 XING - 4 flags - 4 frames - 4 bytes - 100 toc */

    uint8 * pBuf = XingBuffer;
    int32 i, head_flags;
    mp3XingHD.flags = 0;
    pBuf += 4;
    mp3XingHD.hId = hdrInfo.layerID;
    head_flags = mp3XingHD.flags = SwapFileToHostByteOrderInt32(pBuf);
    pBuf += 4;

    if (head_flags & FRAMES_FLAG)
    {
        mp3XingHD.frames = SwapFileToHostByteOrderInt32(pBuf);
        pBuf += 4;
    }

    if (head_flags & BYTES_FLAG)
    {
        mp3XingHD.bytes = SwapFileToHostByteOrderInt32(pBuf);
        pBuf += 4;
    }

    if (head_flags & TOC_FLAG)
    {
        for (i = 0; i < 100; i++)
        {
            mp3XingHD.TOC[i] = pBuf[i];
        }
        pBuf += 100;
    }

    mp3XingHD.vbr_scale = 0;
    if (head_flags & VBR_SCALE_FLAG)
    {
        mp3XingHD.vbr_scale = SwapFileToHostByteOrderInt32(pBuf);
        pBuf += 4;;
    }
    iNumberOfFrames = mp3XingHD.frames;
    return true;
}


/***********************************************************************
 * FUNCTION:    GetMP3FileHeader
 * DESCRIPTION: Returns information necessary to configure the audio device
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
bool MP3Parser::GetMP3FileHeader(MP3ConfigInfoType * pMP3Config)
{
    if (pMP3Config != NULL)
    {
        // For CBR (Constatnt Bit Rate) files, the frame sizes do not
        // vary. So it is sufficient to simply use the first frame's
        // sizes and lengths. For VBR, we'll want to use the maximum
        // sizes and lengths possible.
        oscl_memcpy(pMP3Config, &iMP3ConfigInfo, sizeof(iMP3ConfigInfo));
        if (mp3Type == EXINGType || mp3Type == EVBRIType)
        {
            // Ensure the Application reserves enough space for the largest
            // data rate for this file.
            // This should be the Maximum Data Rate in the file, for a
            // Variable Bitrate File

            if (iMP3HeaderInfo.frameVer == 3)   // MPEG Ver 1
            {
                pMP3Config->BitRate = 1000 * brIndexTableV1[iMP3HeaderInfo.layerID][14];
            }
            else // MPEG Ver 2, 2.5
            {
                pMP3Config->BitRate = 1000 * brIndexTableV2[iMP3HeaderInfo.layerID][14];
            }

            // Compressed Frame Size
            uint32 bitRate = pMP3Config->BitRate;
            uint32 samplingRate = pMP3Config->SamplingRate;
            uint32 FrameLengthInBytes;
            uint32 FrameSizeUnComp;

            if (iMP3HeaderInfo.layerID == 3)  // Layer I
            {
                if (iMP3HeaderInfo.frameVer == 3)   // MPEG Ver 1
                {
                    FrameLengthInBytes = (12 * bitRate / samplingRate + 1) * 4;
                    FrameSizeUnComp = 384;
                }
                else // MPEG Ver 2, 2.5
                {
                    FrameLengthInBytes = (6 * bitRate / samplingRate + 1) * 4;
                    FrameSizeUnComp = 192;
                }
            }
            else // Layer II & III
            {
                if (iMP3HeaderInfo.frameVer == 3)   // MPEG Ver 1
                {
                    FrameLengthInBytes = (144 * bitRate / samplingRate + 1);
                    FrameSizeUnComp = 1152;
                }
                else // MPEG Ver 2,2.5
                {
                    FrameLengthInBytes = (72 * bitRate / samplingRate + 1);
                    FrameSizeUnComp = 576;
                }
            }
            // This should be the largest Frame Size in the File.
            // The Application uses this information for allocating Audio
            // Device Buffers.
            // Maximum possible at any sample rate is 2880 with a pad byte; use 2884 to word align
            // however we have just calculated the maximum at this particular sample rate
            pMP3Config->FrameLengthInBytes = FrameLengthInBytes;
            pMP3Config->FrameSizeUnComp = FrameSizeUnComp;
        }
        return true;
    }
    return false;
}

/***********************************************************************
 *	Function : GetChannelMode
 *	Purpose  : Fetch Channel mode for the clip
 *  Input	 : None
 *  Output	 :
 *	Return   : ChannelMode
 *	Modified :
 ***********************************************************************/
uint32 MP3Parser::GetChannelMode() const
{
    return iMP3HeaderInfo.chMode;
}

/***********************************************************************
 *	Function : GetDurationFromMetadata
 *	Purpose  : Fetch duration value from id3 frame (TLEN - track length)
 *  Input	 : None
 *  Output	 : iClipDurationFromMetadata
 *	Return   : clip duration
 *	Modified :
 ***********************************************************************/
uint32 MP3Parser::GetDurationFromMetadata()
{
    if (iClipDurationFromMetadata <= 0)
    {
        PvmiKvpSharedPtrVector frame;
        PVMFMetadataList keyList;
        keyList.push_back("duration-from-metadata");
        iId3TagParser.GetID3Frame(keyList[0], frame);
        if (frame.size() > 0)
        {
            iClipDurationFromMetadata = frame[0]->value.uint32_value;
        }
    }
    return iClipDurationFromMetadata;
}

/***********************************************************************
 *	Function : ConvertSizeToTime
 *	Purpose  : Fetches duration of the clip playing
 *			   Duration is returned, by different
 *			   means by the pre-defined priorities
 *  Input	 : aMetadataDuration, true if duration from metadata is needed
 *  Output	 : None
 *	Return   : Clip duration
 *	Modified :
 **********************************************************************/
int32 MP3Parser::ConvertSizeToTime(uint32 aFileSize, uint32& aNPTInMS)
{
    uint32 duration = 0;
    uint32 fileSize = aFileSize;

    if (iId3TagParser.IsID3V2Present())
    {
        if (iTagSize > fileSize)
        {
            return -1;
        }
        fileSize -= iTagSize;
    }
    if (iId3TagParser.IsID3V1Present())
    {
        // id3v1.x tags are 128 bytes long
        fileSize -= ID3_V1_TAG_SIZE;
    }

    if (iAvgBitrateInbps > 0)
    {
        duration = (uint32)((OsclFloat)(fileSize * 8000.00f / iAvgBitrateInbps));
        aNPTInMS = duration;
        return 0;
    }

    return -1;
}

/***********************************************************************
 *	Function : GetDuration
 *	Purpose  : Fetches duration of the clip playing
 *			   Duration is returned, by different
 *			   means by the pre-defined priorities
 *  Input	 : aMetadataDuration, true if duration from metadata is needed
 *  Output	 : None
 *	Return   : Clip duration
 *	Modified :
 **********************************************************************/
uint32 MP3Parser::GetDuration(bool aMetadataDuration)
{
    if (aMetadataDuration)
    {
        return GetDurationFromMetadata();
    }

    uint32 clipDuration = 0;

    // local clip playback
    if (!fp->GetCPM())
    {
        // if scanning is complete, send the clip duration from scan
        // else if vbri/xing headers exist send duration from that
        // else scan "N" random frames to estimate duration from avg bitrate
        if (!iDurationScanComplete)
        {
            if (mp3Type == EXINGType || mp3Type == EVBRIType)
            {
                if (MP3_SUCCESS != GetDurationFromVBRIHeader(clipDuration))
                {
                    clipDuration = 0;
                }
            }

            if (clipDuration == 0 && GetDurationFromMetadata() > 0)
            {
                clipDuration = iClipDurationFromMetadata;
                // random scan will not be performed,
                // estimate bitrate from filesize and duration
            }
            else if (clipDuration == 0)
            {
                if (MP3_SUCCESS == GetDurationFromRandomScan(clipDuration))
                {
                    iClipDurationInMsec = clipDuration;
                    return clipDuration;
                }
            }
            // if control gets here, that means avg bit rate from random scan is not calculated.
            if (iAvgBitrateInbpsFromRandomScan <= 0)
            {
                uint32 fileSize = iLocalFileSize;
                if (iId3TagParser.IsID3V2Present())
                {
                    fileSize -= StartOffset;
                }
                if (iId3TagParser.IsID3V1Present())
                {
                    // id3v1.x tags are 128 bytes long
                    fileSize -= ID3_V1_TAG_SIZE;
                }
                iAvgBitrateInbpsFromRandomScan = (int32)((OsclFloat)(fileSize * 8000.00f) / clipDuration);
            }
        }
        else
        {
            GetDurationFromCompleteScan(clipDuration);
        }
    }
    else
    {
        // PD/PS playback
        // duration can only be estimated using content length provided by user
        // If content length has not been recieved, then duration cant be estimated
        // in that case duration value "unknown" is notified to the user
        if (mp3Type == EXINGType || mp3Type == EVBRIType)
        {
            if (MP3_SUCCESS != GetDurationFromVBRIHeader(clipDuration))
            {
                clipDuration = 0;
            }
        }
        if (clipDuration == 0 && MP3_SUCCESS == EstimateDurationFromExternalFileSize(clipDuration))
        {
            clipDuration = iClipDurationFromEstimation;
        }
    }
    iClipDurationInMsec = clipDuration;
    return clipDuration;
}

/***********************************************************************
 *	Function : GetMetadataSize
 *	Purpose  : Fetches size of id3 data
 *  Input	 : None
 *  Output	 : aSize, size of metadata
 *	Return   : error code
 *	Modified :
 ***********************************************************************/
MP3ErrorType MP3Parser::GetMetadataSize(uint32 &aMetadataSize)
{
    if (fp)
    {
        if (iId3TagParser.IsID3V2Present(fp, iTagSize) && iTagSize > 0)
        {
            aMetadataSize = iTagSize;
            return MP3_SUCCESS;
        }
    }
    aMetadataSize = 0;
    return MP3_METADATA_NOTPARSED;
}

/***********************************************************************
 *	Function : GetMinBytesRequired
 *	Purpose  : Fetches size of maximum Mp3 frame
 *  Input	 : None
 *  Output	 : None
 *	Return   : aSize, size of max mp3 frame
 *	Modified :
 ***********************************************************************/
uint32 MP3Parser::GetMinBytesRequired(bool aNextBytes)
{
    uint32 minBytes = KMAX_MP3FRAME_LENGTH_IN_BYTES;
    if (aNextBytes && fp)
    {
        // case where parse file has failed due to lack of data
        // in that case request next n bytes.
        minBytes += fp->Tell();
    }
    return minBytes;
}

/***********************************************************************
 * FUNCTION: GetNextBundledAccessUnits
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
int32 MP3Parser::GetNextBundledAccessUnits(uint32 *n, GAU *pgau, MP3ErrorType &error)
{
    uint32 nBytesRead = 0;
    uint32 framets = 0;
    uint32 nBytesReadTotal = 0;
    int32  i;
    error = MP3_ERROR_UNKNOWN;
    if ((pgau == NULL) || (pgau->buf.num_fragments > 1)
            || (n == NULL))
    {
        return 0;
    }

    uint8 * pOutputBuffer = (uint8 *)pgau->buf.fragments[0].ptr;
    int32 iLength = pgau->buf.fragments[0].len;
    for (i = 0; (i < (int32)*n) && (iLength > 0); i++)
    {
        pgau->numMediaSamples = i;

        error = GetNextMediaSample(pOutputBuffer, iLength, nBytesRead, framets);
        if ((error == MP3_SUCCESS))
        {
            if (nBytesRead > 0)
            {
                // Frame was read successfully
                pgau->info[i].len = nBytesRead;
                pgau->info[i].ts  = framets;
            }
        }
        else
        {
            // Read failure
            break;
        }

        iLength -= nBytesRead;
        pOutputBuffer += nBytesRead;
        nBytesReadTotal += nBytesRead;
    }
    *n = i;
    return nBytesReadTotal;
}
/***********************************************************************
 * FUNCTION: PeekNextBundledAccessUnits
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
int32  MP3Parser::PeekNextBundledAccessUnits(uint32 *n, MediaMetaInfo *mInfo)
{
    uint32 nBytesToRead = 0;
    if ((mInfo == NULL) || (n == NULL))
    {
        return 0;
    }

    for (uint32 i = 0; i < *n; i++)
    {
        if ((iCurrFrameNumber + (int32)i) >= iNumberOfFrames)
        {
            break;
        }

        mInfo->ts = GetTimestampForSample(iCurrFrameNumber + i);
        // Don't care
        mInfo->layer = 0;
        // Maximum Frame Length
        mInfo->len   = (iMP3ConfigInfo.FrameLengthInBytes + MP3_FRAME_HEADER_SIZE);
        mInfo->sample_info = 0;
        mInfo->dropFlag = 0;

        nBytesToRead += mInfo->len;
    }
    return nBytesToRead;
}


/***********************************************************************
 * FUNCTION:    GetNextMediaSample
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
MP3ErrorType MP3Parser::GetNextMediaSample(uint8 *buffer, uint32 size, uint32& framesize, uint32& timestamp)
{
    MP3ErrorType mp3Err = MP3_SUCCESS;

BEGIN:
    uint32 currentFilePosn = 0;
    uint32 mp3Header = 0;
    uint32 mp3FrameSizeInBytes = 0;
    MP3HeaderType     mp3HeaderInfo = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    MP3ConfigInfoType mp3CDInfo = {0, 0, 0, 0, 0};
    uint8 *buf = buffer;

    framesize = 0;
    timestamp = 0;
    // Read and Decode the MP3 Frame Header to obtain the
    // correct number of bytes in this frame.
    currentFilePosn = MP3Utils::getCurrentFilePosition(fp);

    // If content length is known (non-0), avoid reading beyond EOF
    uint32 contentLength = MP3FileIO::getContentLength(fp);
    if (0 != contentLength)
    {
        // check for reading beyond EOF
        if ((currentFilePosn + MP3_FRAME_HEADER_SIZE) >= contentLength)
        {
            return MP3_END_OF_FILE;
        }
    }

    if (!MP3FileIO::readByteData(fp, MP3_FRAME_HEADER_SIZE, buf))
    {
        return MP3_INSUFFICIENT_DATA;
    }

    // Convert the File Byte Order to Host Memory Byte Order
    // for 32 bit integers
    mp3Header = SwapFileToHostByteOrderInt32(buf);

    // Adjust the buffer write location in preparation for
    // the next read
    if (! GetMP3Header(mp3Header, mp3HeaderInfo))
    {
        // ////////////////////////////////////////////////////////////////////////////
        // If we don't find a valid MP3 Marker point we will attempt recovery.
        uint32 seekOffset = 0;
        MP3Utils::SeektoOffset(fp, 0 - MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);
        MP3ErrorType err = mp3FindSync(currentFilePosn, seekOffset, fp);

        if (err == MP3_SUCCESS)
        {
            err = MP3Utils::SeektoOffset(fp, seekOffset, Oscl_File::SEEKCUR);
            if (MP3_SUCCESS != err)
            {
                return err;
            }
            currentFilePosn += seekOffset;

            if (0 != contentLength)
            {
                // if content length is known, check for reading beyond EOF
                if ((currentFilePosn + MP3_FRAME_HEADER_SIZE) >= contentLength)
                {
                    return MP3_END_OF_FILE;
                }
            }

            if (!MP3FileIO::readByteData(fp, MP3_FRAME_HEADER_SIZE, buf))
            {
                return MP3_INSUFFICIENT_DATA;
            }

            mp3Header = SwapFileToHostByteOrderInt32(buf);
            if (! GetMP3Header(mp3Header, mp3HeaderInfo))
            {
                return MP3_FILE_HDR_READ_ERR;
            }
        }
        else
        {
            return err;
        }
    }

    buf += MP3_FRAME_HEADER_SIZE;
    iCurrFrameNumber++;
    if (! DecodeMP3Header(mp3HeaderInfo, mp3CDInfo, true))
    {
        iCurrFrameNumber--;
        return MP3_FILE_HDR_DECODE_ERR;
    }

    int32 revSeek = 0 - MP3_FRAME_HEADER_SIZE;
    mp3FrameSizeInBytes = mp3CDInfo.FrameLengthInBytes;

    MP3ErrorType err = MP3Utils::SeektoOffset(fp, revSeek, Oscl_File::SEEKCUR);
    if (MP3_SUCCESS != err)
    {
        iCurrFrameNumber--;
        return err;
    }

    mp3Err = mp3VerifyCRC(mp3HeaderInfo, mp3CDInfo);
    if (mp3Err == MP3_CRC_ERR)
    {
        //wrong CRC skip frame. since crc was wrong there
        //could be error in calculating frame size. So try to find sync again.
        iCurrFrameNumber--;
        currentFilePosn += MP3_FRAME_HEADER_SIZE;
        if (MP3_SUCCESS != MP3Utils::SeektoOffset(fp, MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR))
        {
            return err;
        }

        uint32 seekOffset = 0;
        MP3Utils::SeektoOffset(fp, 0 - MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);

        MP3ErrorType err = mp3FindSync(currentFilePosn, seekOffset, fp);
        if (err == MP3_SUCCESS)
        {
            err = MP3Utils::SeektoOffset(fp, seekOffset, Oscl_File::SEEKCUR);
            if (MP3_SUCCESS != err)
            {
                return err;
            }
            currentFilePosn += seekOffset;
            goto BEGIN;
        }
        else
        {
            //no sync found return error
            return err;
        }
    }
    else if (mp3Err == MP3_INSUFFICIENT_DATA)
    {
        iCurrFrameNumber--;
        return mp3Err;
    }

    err = MP3Utils::SeektoOffset(fp, MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);
    if (MP3_SUCCESS != err)
    {
        iCurrFrameNumber--;
        return err;
    }

    currentFilePosn = MP3Utils::getCurrentFilePosition(fp);

    uint32 fileSz = 0;
    MP3Utils::getCurrentFileSize(fp, fileSz);
    if ((fileSz != 0) && (currentFilePosn + (mp3FrameSizeInBytes - MP3_FRAME_HEADER_SIZE) > (uint32)fileSz))
    {
        // At EOF
        iCurrFrameNumber--;
        framesize = 0;
        timestamp = GetTimestampForCurrentSample();
        if (mp3CDInfo.BitRate > 0)
        {
            iTimestamp = uint32(timestamp + (OsclFloat) mp3CDInfo.FrameLengthInBytes * 8000.00f / mp3CDInfo.BitRate);
        }
        if (0 != contentLength)
        {
            // if content length is known, check for reading beyond EOF
            if ((currentFilePosn + mp3FrameSizeInBytes - MP3_FRAME_HEADER_SIZE) >= contentLength)
            {
                return MP3_END_OF_FILE;
            }
        }
        err = MP3Utils::SeektoOffset(fp, 0 - MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);
        if (MP3_SUCCESS != err)
        {
            return err;
        }
        return MP3_INSUFFICIENT_DATA;
    }

    if (size < mp3FrameSizeInBytes)
    {
        framesize = mp3FrameSizeInBytes;
        iCurrFrameNumber--;
        return MP3_FILE_READ_ERR;
    }

    framesize = mp3FrameSizeInBytes;
    timestamp = GetTimestampForCurrentSample();

    // update timestamp for next sample
    // calculate frameDuration
    if (mp3CDInfo.BitRate > 0)
    {
        iTimestamp = uint32(timestamp + (OsclFloat) mp3CDInfo.FrameLengthInBytes * 8000.00f / mp3CDInfo.BitRate);
    }

    // Take into account the header (4 Bytes) already read up front
    // to obtain the correct Frame Size in Bytes
    if (0 != contentLength)
    {
        // if content length is known, check for reading beyond EOF
        if ((currentFilePosn + mp3FrameSizeInBytes - MP3_FRAME_HEADER_SIZE) >= contentLength)
        {
            return MP3_END_OF_FILE;
        }
    }

    // Take into account the header (4 Bytes) already read up front
    // to obtain the correct Frame Size in Bytes
    bool res = MP3FileIO::readByteData(fp, mp3FrameSizeInBytes - MP3_FRAME_HEADER_SIZE, buf);
    if ((iLocalFileSize == 0) && res == false)
    {
        iCurrFrameNumber--;
        return MP3_INSUFFICIENT_DATA;
    }

    return MP3_SUCCESS;
}

/***********************************************************************
 * FUNCTION:    SeekToTimestamp
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
uint32  MP3Parser::SeekToTimestamp(uint32 timestampInMsec)
{
    uint32 SeekPosition = 0;
    SeekPosition = SeekPointFromTimestamp(timestampInMsec);
    if (!((!fp->GetCPM()) && (SeekPosition == iLocalFileSize) && (timestampInMsec == iClipDurationInMsec)))
    {
        SeekPosition += StartOffset;
    }
    MP3Utils::SeektoOffset(fp, SeekPosition, Oscl_File::SEEKSET);
    return timestampInMsec;
}

/***********************************************************************
 * FUNCTION:    SeekPointFromTimestamp
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
uint32 MP3Parser::SeekPointFromTimestamp(uint32 &timestamp)
{
    uint32 seekPoint = 0;
    uint32 seekOffset = 0;
    OsclFloat percent = 0;
    uint32 binNo = 0;

    bool bUseTOCForRepos = false;

    uint32 maxTSInTOC = iTOCFilledCount * iBinWidth;
    if (iTOCFilledCount > 1 && (timestamp < maxTSInTOC || iDurationScanComplete))
    {
        bUseTOCForRepos = true;
    }

    // XING - Use VBR TOC Header
    if ((mp3Type == EXINGType) && (iXingHeader.flags & TOC_FLAG))
    {
        // Interpolate in TOC to get file seek point in bytes
        OsclFloat fpc = (OsclFloat)timestamp / (OsclFloat)iClipDurationFromVBRIHeader;
        OsclFloat fa, fb, fx;
        uint32 pc;

        if (fpc < 0.00f)
            fpc = 0.00f;

        if (fpc > 1.00f)
            fpc = 1.00f;

        pc = (uint32)(fpc * 100.00f);
        fa = (OsclFloat)iXingHeader.TOC[pc];

        if (pc > 99)
            pc = 99;

        if (pc < 99)
            fb = (OsclFloat)iXingHeader.TOC[pc+1];
        else
            fb = 256.00f;

        // //////////////////////////////////////////////////////////
        // Linearly interpolate between fa and fb
        // TOC's appear to max out at TOC[80] = 0cff (255)
        // //////////////////////////////////////////////////////////
        fx = fa + (fb - fa) * ((100.00f * fpc) - (OsclFloat)pc);
        percent = fx;
        if (iXingHeader.flags == 15 || iXingHeader.flags == 7 || iXingHeader.flags == 3)
        {
            seekPoint = (int32)((fx / 256.00f) * (OsclFloat)iXingHeader.bytes);
            if ((seekPoint > (uint32)iXingHeader.bytes))
            {
                seekPoint = 0;
                percent = 0;
            }
        }
        else
        {
            seekPoint = 0;
            percent = 0;
        }
    }
    else if (mp3Type == EVBRIType)
        // //////////////////////////////////////////////////////////
        // CBR or VBRI
    {
        uint32 i = 0, fraction = 0, SamplesPerFrame;
        OsclFloat fLengthMS;
        OsclFloat fLengthMSPerTOCEntry;
        OsclFloat fAccumulatedTimeMS = 0.0f;
        (iVbriHeader.sampleRate >= 32000) ? (SamplesPerFrame = 1152) : (SamplesPerFrame = 576);


        fLengthMS = ((OsclFloat)iVbriHeader.frames * (OsclFloat)SamplesPerFrame)
                    / (OsclFloat)iVbriHeader.sampleRate * 1000.0f;

        fLengthMSPerTOCEntry = (OsclFloat)fLengthMS / (OsclFloat)(iVbriHeader.entriesTOC + 1);

        if (timestamp > fLengthMS)
            timestamp = (uint32)fLengthMS;

        while (fAccumulatedTimeMS <= timestamp)
        {
            seekPoint += iVbriHeader.TOC[i];
            fAccumulatedTimeMS += fLengthMSPerTOCEntry;
            i++;
        }

        fraction = ((int)((((fAccumulatedTimeMS - timestamp) / fLengthMSPerTOCEntry)
                           + (1.0f / (2.0f * (OsclFloat)iVbriHeader.fTableEntry))) * (OsclFloat)iVbriHeader.fTableEntry));

        seekPoint -= (int)((OsclFloat)iVbriHeader.TOC[i-1] * (OsclFloat)(fraction)
                           / (OsclFloat)iVbriHeader.fTableEntry);
    }
    else if (bUseTOCForRepos)
    {
        // Use TOC for calculating seek point.
        OsclFloat fpc = (OsclFloat)timestamp / (OsclFloat)iClipDurationInMsec;
        binNo = (uint32)(fpc * iTOCFilledCount);
        uint32 TScurr = binNo * iBinWidth;
        while (TScurr > timestamp)
        {
            binNo--;
            TScurr = binNo * iBinWidth;
        }

        uint32 offsetDiff = iTOC[binNo+1] - iTOC[binNo];
        uint32 tsDiff = timestamp - TScurr;
        seekPoint = iTOC[binNo] + tsDiff * (offsetDiff / iBinWidth);
    }
    else
    {
        /**
         * vbri and xing headers are not present. seek offset will be
         * calculated on the basis of average bit rate
         **/
        int32 avgBR = 0;
        if (fp->GetCPM())
        {
            avgBR = iAvgBitrateInbps;
        }
        else
        {
            if (iDurationScanComplete && (iAvgBitrateInbpsFromCompleteScan > 0))
            {
                avgBR = iAvgBitrateInbpsFromCompleteScan;
            }
            else
            {
                avgBR = iAvgBitrateInbpsFromRandomScan;
            }
        }
        seekPoint = (uint32)((OsclFloat)(avgBR * (OsclFloat)timestamp) / 8000.0f);
    }

    /**
    * If we don't find a sync point we will start playing from the beginning again
    * try finding seek points only for local playback
    * Since in PD/PS scenarios we might not be having enough data to find the seek point
    * We can find the seek point when we are resuming the playback
    **/
    if (seekPoint > 0 && !fp->GetCPM())
    {
        // seek to the reposition point location
        MP3Utils::SeektoOffset(fp, seekPoint + StartOffset, Oscl_File::SEEKSET);
        uint32 retVal = mp3FindSync(seekPoint + StartOffset, seekOffset, fp);

        if (retVal == MP3_SUCCESS)
        {
            seekPoint += seekOffset;
            MP3Utils::SeektoOffset(fp, seekOffset, Oscl_File::SEEKCUR);
            if (iDurationScanComplete)
            {
                uint32 offsetDiff = iTOC[binNo+1] - iTOC[binNo];
                timestamp = (binNo * iBinWidth) + (iBinWidth * (seekPoint - iTOC[binNo]) / offsetDiff);
            }
        }
        else if (retVal == MP3_INSUFFICIENT_DATA || retVal == MP3_END_OF_FILE)
        {
            // if parser hits Insufficent data during local playback or end of file,
            // we need to set seekpoint and timestamp to clip duration and node will report as end of track.
            seekPoint = iLocalFileSize;
            timestamp = iClipDurationInMsec;
            iTimestamp = timestamp;
            // return from here as we need not compute Current Frame Number
            return seekPoint;
        }
        else
        {
            seekPoint = 0;
            timestamp = 0;
        }
    }


    if (seekPoint > 0)
    {
        if (iMP3ConfigInfo.FrameSizeUnComp > 0  && iMP3ConfigInfo.SamplingRate > 0)
        {
            iCurrFrameNumber = (int32)(timestamp * (iMP3ConfigInfo.SamplingRate / iMP3ConfigInfo.FrameSizeUnComp) / 1000.00f);
        }
    }
    else
    {
        iCurrFrameNumber = 0;
        timestamp = 0;
    }
    iTimestamp = timestamp;
    return seekPoint;
}


/***********************************************************************
 * FUNCTION:    mp3FindSync
 * DESCRIPTION: This function reads the whole file searching for a sync
 *				word. Once it finds one, it check for 4 continuous sync
 *				words to avoid a false synchronization
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
MP3ErrorType MP3Parser::mp3FindSync(uint32 seekPoint, uint32 &syncOffset, PVFile* aFile)
{
    syncOffset = 0;
    iMaxSyncBufferSize = 627;   /* default for 192 kbps, 44.1 kHz */

    if (!aFile)
    {
        if (fp->GetCPM() != NULL)
        {
            iLocalFileSizeSet = (int32)MP3Utils::getCurrentFileSize(fp, iLocalFileSize);
        }

        uint32 contentLength = MP3FileIO::getContentLength(fp);
        if (0 != contentLength)
        {
            // if content length is known, check for reading beyond EOF
            if ((seekPoint + iMaxSyncBufferSize) >= contentLength)
            {
                return MP3_END_OF_FILE;
            }
        }

        if ((iLocalFileSize != 0) && (seekPoint + iMaxSyncBufferSize > (uint32)iLocalFileSize))
        {
            return MP3_INSUFFICIENT_DATA;
        }

        if (pSyncBuffer)
        {
            OSCL_ARRAY_DELETE(pSyncBuffer);
            pSyncBuffer = NULL;
        }
    }

    int32 leavecode = 0;
    OSCL_TRY(leavecode, pSyncBuffer = OSCL_ARRAY_NEW(uint8, iMaxSyncBufferSize + 1));
    if (leavecode || pSyncBuffer == NULL)
    {
        return MP3_ERROR_UNKNOWN;  /* buffer couldn't be allocated */
    }

    PVFile* fpused = (aFile) ? aFile : fp;
    seekPoint = fpused->Tell();
    uint32 i = 0;
    uint32 j = 0;
    uint32 BufferSize = 0;
    pSyncBuffer[0] = 0;
    bool syncFound = false;
    MP3ErrorType mp3Err = MP3_SUCCESS;
    uint32 maxSearchOffset = 0;
    int32 revSeek = 0;
    if (iLocalFileSizeSet)
    {
        maxSearchOffset = OSCL_MIN(iInitSearchFileSize, iLocalFileSize - seekPoint);
    }
    else
    {
        uint32 remBytes = 0;
        if (fp->GetRemainingBytes(remBytes))
        {
            maxSearchOffset = OSCL_MIN(iInitSearchFileSize, fpused->Tell() + remBytes - seekPoint);
        }
    }

    for (j = 0; j < maxSearchOffset; j += iMaxSyncBufferSize)
    {
        revSeek = 0;
        // Grab a new buffer for a byte by byte search
        if (!MP3FileIO::readByteData(fpused, iMaxSyncBufferSize, &pSyncBuffer[1], &BufferSize))
        {
            if (pSyncBuffer)
            {
                OSCL_ARRAY_DELETE(pSyncBuffer);
                pSyncBuffer = NULL;
            }
            return MP3_ERROR_UNKNOWN_OBJECT;
        }
        revSeek -= j;
        // Find the first Sync Marker by doing a byte by byte search.
        // Once we have found the sync words, the frame is validated.
        // For frame header validation, we read four bytes.
        // So the search for sync words should be stopped when we have less than
        //   4 bytes in the buffer.
        if (BufferSize > 3)
        {
            for (i = 0; i < (BufferSize - 3); i++)
            {
                if (pSyncBuffer[i] == 0xFF)
                {
                    // MPEG 1, 2
                    if ((pSyncBuffer[i+1] & 0xF0) == 0xF0)
                    {
                        // if partial match is found verify that 4 consecutives sync word are valid
                        MP3Utils::SeektoOffset(fpused, 0 - (int32) iMaxSyncBufferSize + (int32) i - 1, Oscl_File::SEEKCUR);
                        mp3Err = IsValidFrame(&(pSyncBuffer[i]), j + i - 1, seekPoint, fpused);
                        if (mp3Err == MP3_SUCCESS)
                        {
                            MP3Utils::SeektoOffset(fpused, 0 - (int32)j - i + 1, Oscl_File::SEEKCUR);
                            break;
                        }
                        else if (mp3Err == MP3_INSUFFICIENT_DATA)
                        {
                            if (pSyncBuffer)
                            {
                                OSCL_ARRAY_DELETE(pSyncBuffer);
                                pSyncBuffer = NULL;
                            }
                            return mp3Err;
                        }
                        else
                        {
                            // Drop the frame
                            MP3Utils::SeektoOffset(fpused, iMaxSyncBufferSize - i + 1, Oscl_File::SEEKCUR);
                        }
                    }
                    // MPEG 2.5
                    else if ((pSyncBuffer[i+1] & 0xF0) == 0xE0)
                    {
                        // if partial match is found verify that 4 consecutives sync word are valid
                        MP3Utils::SeektoOffset(fpused, 0 - (int32) iMaxSyncBufferSize + (int32) i - 1, Oscl_File::SEEKCUR);
                        mp3Err = IsValidFrame(&(pSyncBuffer[i]), j + i - 1, seekPoint, fpused);

                        if (mp3Err == MP3_SUCCESS)
                        {
                            MP3Utils::SeektoOffset(fpused, 0 - (int32)j - i + 1, Oscl_File::SEEKCUR);
                            break;
                        }
                        else if (mp3Err == MP3_INSUFFICIENT_DATA)
                        {
                            if (pSyncBuffer)
                            {
                                OSCL_ARRAY_DELETE(pSyncBuffer);
                                pSyncBuffer = NULL;
                            }
                            return mp3Err;
                        }
                        else
                        {
                            // Drop the frame
                            MP3Utils::SeektoOffset(fpused, iMaxSyncBufferSize - i + 1, Oscl_File::SEEKCUR);
                        }
                    }
                }
            }
            if (i < (BufferSize - 3))
            {
                syncFound = true;
                break;      /*  sync was found */
            }               /*  else grab new buffer and keep searching */
            pSyncBuffer[0] = pSyncBuffer[iMaxSyncBufferSize];
        }
        else
        {
            break;
        }
    }

    if (pSyncBuffer)
    {
        OSCL_ARRAY_DELETE(pSyncBuffer);
        pSyncBuffer = NULL;
    }

    if (!syncFound)
    {
        return MP3_END_OF_FILE;  /* File does not have any valid sync word */
    }

    if (iLocalFileSizeSet)
    {
        /* One valid frame was found -> reset initial size to file size */
        iInitSearchFileSize = iLocalFileSize;
    }
    else
    {
        uint32 remBytes = 0;
        if (fp->GetRemainingBytes(remBytes))
        {
            /* One valid frame was found -> reset initial size remaining file size*/
            iInitSearchFileSize = OSCL_MIN(iInitSearchFileSize, remBytes);
        }
    }

    syncOffset = j + i - 1;  /*  set offset */
    return MP3_SUCCESS;
}


/***********************************************************************
 * FUNCTION:   IsValidFrame
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
MP3ErrorType MP3Parser::IsValidFrame(uint8 * pBuffer,
                                     uint32 offset,
                                     uint32 seekPoint,
                                     PVFile* aFile)
{
    bool bCRCPresent = false;
    MP3ErrorType err = MP3_SUCCESS;
    // Is the MP3 Frame Header Valid?
    err = IsValidFrameHeader(pBuffer, bCRCPresent, offset, seekPoint, aFile);
    return err;
}

/***********************************************************************
 * FUNCTION:    IsValidFrameHeader
 * DESCRIPTION: This function now check the sync word and then with the
 *				information retrieved from the header, predict the location
 *				of the following 3 headers. Then, if the sampling frequencies
 *				and number of channels match for all headers, the frame header
 *				is considered valid
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
MP3ErrorType MP3Parser::IsValidFrameHeader(uint8 *mp3Frame, bool &bCRCPresent,
        uint32 firstSyncOffset, uint32 seekPoint,
        PVFile* aFile)
{
    OSCL_UNUSED_ARG(firstSyncOffset);
    OSCL_UNUSED_ARG(seekPoint);

    PVFile* fpUsed = fp;
    if (aFile)
    {
        fpUsed = aFile;
    }
    MP3HeaderType     mp3HeaderInfo = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    MP3ConfigInfoType mp3CDInfo  = {0, 0, 0, 0, 0};
    MP3ConfigInfoType mp3CDInfo2 = {0, 0, 0, 0, 0};
    MP3ConfigInfoType mp3CDInfo3 = {0, 0, 0, 0, 0};
    MP3ConfigInfoType mp3CDInfo4 = {0, 0, 0, 0, 0};
    int32 flength = 0;
    int32 revSeek = 0;
    bool status;

    uint32 mp3Header = SwapFileToHostByteOrderInt32(mp3Frame);


    bCRCPresent = false;
    if (!GetMP3Header(mp3Header, mp3HeaderInfo))
    {
        return MP3_FILE_HDR_READ_ERR;
    }

    if (!DecodeMP3Header(mp3HeaderInfo, mp3CDInfo, false))
    {
        return MP3_FILE_HDR_READ_ERR;
    }

    // A flag of 0 means the CRC is present
    bCRCPresent = !(mp3HeaderInfo.crcFollows);

    /*
     *  Search 4 consecutives header to guarantee that we
     *  really latch on a valid sync word
     */
    flength = mp3CDInfo.FrameLengthInBytes;
    MP3ErrorType err = MP3Utils::SeektoOffset(fpUsed, flength, Oscl_File::SEEKCUR);
    if (MP3_SUCCESS != err)
    {
        return err;
    }
    revSeek -= flength;

    if (!MP3FileIO::readByteData(fpUsed, MP3_FRAME_HEADER_SIZE, (uint8 *)&mp3Header))
    {
        MP3Utils::SeektoOffset(fpUsed, revSeek , Oscl_File::SEEKCUR);
        return MP3_INSUFFICIENT_DATA;
    }

    mp3Header = SwapFileToHostByteOrderInt32((uint8 *) & mp3Header);

    status = GetMP3Header(mp3Header, mp3HeaderInfo);
    status = DecodeMP3Header(mp3HeaderInfo, mp3CDInfo2, false);

    flength = mp3CDInfo2.FrameLengthInBytes;

    err = MP3Utils::SeektoOffset(fpUsed, flength - MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);
    if (MP3_SUCCESS != err)
    {
        MP3Utils::SeektoOffset(fpUsed, revSeek - MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);
        return err;
    }
    revSeek -= flength;

    if (!MP3FileIO::readByteData(fpUsed, MP3_FRAME_HEADER_SIZE, (uint8 *)&mp3Header))
    {
        MP3Utils::SeektoOffset(fpUsed, revSeek , Oscl_File::SEEKCUR);
        return MP3_INSUFFICIENT_DATA;
    }

    mp3Header = SwapFileToHostByteOrderInt32((uint8 *) & mp3Header);

    status = GetMP3Header(mp3Header, mp3HeaderInfo);
    status = DecodeMP3Header(mp3HeaderInfo, mp3CDInfo3, false);

    flength = mp3CDInfo3.FrameLengthInBytes;
    err = MP3Utils::SeektoOffset(fpUsed, flength - MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);
    if (MP3_SUCCESS != err)
    {
        MP3Utils::SeektoOffset(fpUsed, revSeek - MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);
        return err;
    }
    revSeek -= flength;

    if (!MP3FileIO::readByteData(fpUsed, MP3_FRAME_HEADER_SIZE, (uint8 *)&mp3Header))
    {
        MP3Utils::SeektoOffset(fpUsed, revSeek, Oscl_File::SEEKCUR);
        return MP3_INSUFFICIENT_DATA;
    }
    revSeek -= MP3_FRAME_HEADER_SIZE;

    mp3Header = SwapFileToHostByteOrderInt32((uint8 *) & mp3Header);

    status = GetMP3Header(mp3Header, mp3HeaderInfo);
    status = DecodeMP3Header(mp3HeaderInfo, mp3CDInfo4, false);


    /*
     *  Check that the sampling rate and the number of channels is
     *  the same for all the frames (everything else may change)
     */
    if ((mp3CDInfo.SamplingRate != mp3CDInfo2.SamplingRate) |
            (mp3CDInfo3.SamplingRate != mp3CDInfo4.SamplingRate) |
            (mp3CDInfo3.SamplingRate != mp3CDInfo.SamplingRate))
    {
        MP3Utils::SeektoOffset(fpUsed, revSeek, Oscl_File::SEEKCUR);
        return MP3_FILE_HDR_READ_ERR;
    }

    if ((mp3CDInfo.NumberOfChannels != mp3CDInfo2.NumberOfChannels) |
            (mp3CDInfo3.NumberOfChannels != mp3CDInfo4.NumberOfChannels) |
            (mp3CDInfo3.NumberOfChannels != mp3CDInfo.NumberOfChannels))
    {
        MP3Utils::SeektoOffset(fpUsed, revSeek, Oscl_File::SEEKCUR);
        return MP3_FILE_HDR_READ_ERR;
    }

    // seek back to position from where we started
    MP3Utils::SeektoOffset(fpUsed, revSeek, Oscl_File::SEEKCUR);
    return MP3_SUCCESS;
}

/***********************************************************************
 * FUNCTION:   mp3VerifyCRC
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS: verifies the crc if crc check is enabled and
 *							crc flag is present
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
MP3ErrorType MP3Parser::mp3VerifyCRC(MP3HeaderType mp3HdrInfo, MP3ConfigInfoType mp3CI)
{
    if (!iEnableCrcCalc || mp3HdrInfo.crcFollows)
    {
        return MP3_SUCCESS;
    }

    uint32 numberOfBits;
    uint32 bound = 32;
    uint32 numberOfBytes = 0;
    uint8 crcData[2];

    if (mp3HdrInfo.chMode == CHANNEL_MODE_JOINT_STEREO)
        bound = 4 + mp3HdrInfo.modeExtn * 4;


    switch (mp3HdrInfo.layerID)
    {
        case MPEG_LAYER_I:
            numberOfBits = 4 * (mp3CI.NumberOfChannels * bound + (32 - bound));
            break;
        case MPEG_LAYER_II:
            // no check for Layer II
            return MP3_SUCCESS;

        case MPEG_LAYER_III:
            numberOfBits = (mp3HdrInfo.frameVer == FRAME_VESION_MPEG_1) ?
                           (mp3HdrInfo.chMode == CHANNEL_MODE_MONO ? 17 * 8 : 32 * 8) :
                                   (mp3HdrInfo.chMode == CHANNEL_MODE_MONO ? 9 * 8 : 17 * 8) ;

            break;
        default:
            return MP3_SUCCESS;
    }

    // Add header size and CRC value.
    //CalcCRC16 will take care of removing first 2 bytes of Hdr and 2 bytes of CRCvalue
    numberOfBits += MP3_FRAME_HEADER_SIZE * 8 + 16;

    numberOfBytes = numberOfBits % 8 ? numberOfBits / 8 + 1 : numberOfBits / 8;

    uint8 *buffer = OSCL_ARRAY_NEW(uint8 , numberOfBytes + 1);
    int32 revSeek = 0;
    if (!MP3FileIO::readByteData(fp, numberOfBytes, (uint8 *)buffer))
{
        return MP3_INSUFFICIENT_DATA;
    }

    revSeek -= numberOfBytes;

    uint16 calcCRC16 = CalcCRC16(buffer, numberOfBits);

    // read crc from frame
    uint32 remBytes = 0;
    if (fp->GetRemainingBytes(remBytes))
    {
        if (remBytes >= MP3_FRAME_HEADER_SIZE)
        {
            MP3Utils::SeektoOffset(fp, MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);
            revSeek -= MP3_FRAME_HEADER_SIZE;
        }
        else
        {
            MP3Utils::SeektoOffset(fp, revSeek, Oscl_File::SEEKCUR);
            return MP3_INSUFFICIENT_DATA;
        }
    }
    else
    {
        MP3Utils::SeektoOffset(fp, revSeek, Oscl_File::SEEKCUR);
        return MP3_ERROR_UNKNOWN;
    }

    if (!MP3FileIO::readByteData(fp, 2, (uint8 *)crcData))
    {
        MP3Utils::SeektoOffset(fp, revSeek, Oscl_File::SEEKCUR);
        return MP3_INSUFFICIENT_DATA;
    }
    revSeek -= 2;

    uint16 crcVal = SwapFileToHostByteOrderInt16(&crcData[0]);

    // seek back to original start position
    MP3ErrorType err = MP3Utils::SeektoOffset(fp, revSeek, Oscl_File::SEEKCUR);
    if (MP3_SUCCESS != err)
    {
        return err;
    }

    if (calcCRC16 == crcVal)
    {
        OSCL_ARRAY_DELETE(buffer);
        return MP3_SUCCESS;
    }

    OSCL_ARRAY_DELETE(buffer);
    return MP3_CRC_ERR;

}
uint16 MP3Parser::CalcCRC16(uint8* pBuffer, uint32 dwBitSize)
{
    uint32 n;
    uint16 tmpchar, crcmask, tmpi;
    crcmask = tmpchar = 0;
    uint16 crc = 0xffff;			// start with inverted value of 0

    // start with byte 2 of header
    for (n = 16;  n < dwBitSize;  n++)
    {
        if (n < 32 || n >= 48) // skip the 2 bytes of the crc itself
        {
            if (n % 8 == 0)
            {
                crcmask = 1 << 8;
                tmpchar = pBuffer[n/8];
            }
            crcmask >>= 1;
            tmpi = crc & 0x8000;
            crc <<= 1;

            if (!tmpi ^ !(tmpchar & crcmask))
                crc ^= 0x8005;
        }
    }
    crc &= 0xffff;	// invert the result
    return crc;
}

/***********************************************************************
 * FUNCTION:    GetSampleCountInFile
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
uint32 MP3Parser::GetSampleCountInFile()
{
    return iNumberOfFrames;
}

/***********************************************************************
 * FUNCTION:    GetMaximumDecodeBufferSize
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
uint32 MP3Parser::GetMaximumDecodeBufferSize()
{
    // Compressed Frame Size
    uint32 maxBitRate;
    uint32 minSamplingRate ;
    uint32 frameLengthInBytes;
    uint32 samplesPerFrame;

    if (iMP3HeaderInfo.frameVer == 3) // MPEG Ver 1
    {
        maxBitRate = brIndexTableV1[iMP3HeaderInfo.layerID][14];
    }
    else // MPEG Ver 2, 2.5
    {
        maxBitRate = brIndexTableV2[iMP3HeaderInfo.layerID][14];
    }

    samplesPerFrame = spfIndexTable[iMP3HeaderInfo.frameVer][iMP3HeaderInfo.layerID];
    minSamplingRate = srIndexTable[((iMP3HeaderInfo.frameVer)*4) + 2];

    if (minSamplingRate != 0)
    {
        frameLengthInBytes = 125 * (maxBitRate * samplesPerFrame) / (minSamplingRate);
    }
    else
    {
        frameLengthInBytes = KMAX_MP3FRAME_LENGTH_IN_BYTES;  // allow for pad byte
    }

    return frameLengthInBytes;
}


/***********************************************************************
 * FUNCTION:    GetFileSize
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
uint32 MP3Parser::GetFileSize()
{
    return iLocalFileSize;
}

/***********************************************************************
 * FUNCTION:    GetTimestampForCurrentSample
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
uint32 MP3Parser::GetTimestampForCurrentSample() const
{
    return iTimestamp;
}

uint32 MP3Parser::GetTimestampForSample(int32 aFrameNumber) const
{
    uint32 timestamp = 0;
    timestamp = (uint32)((1000.00f * (OsclFloat)aFrameNumber *
                          (OsclFloat)iMP3ConfigInfo.FrameSizeUnComp) /
                         (OsclFloat)iMP3ConfigInfo.SamplingRate);
    return timestamp;
}

/***********************************************************************
 * FUNCTION:    GetDecoderSpecificInfoSize
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
uint32  MP3Parser::GetDecoderSpecificInfoSize()
{
    return ConfigSize;
}

/***********************************************************************
 * FUNCTION:    GetDecoderSpecificInfoContent
 * DESCRIPTION:
 * INPUT/OUTPUT PARAMETERS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 ***********************************************************************/
uint8 const * MP3Parser::GetDecoderSpecificInfoContent() const
{
    return (const uint8 *)&(ConfigData);
}

int32 MP3Parser :: CalculateBufferSizeForHeader(uint8 *VbriHead)
{
    int32 tableLength;
    int32  tempEntriesTOC, tempSizePerTableEntry;

    VbriHead += 18;
    tempEntriesTOC = SwapFileToHostByteOrderInt16(VbriHead);
    VbriHead += 4;

    tempSizePerTableEntry = SwapFileToHostByteOrderInt16(VbriHead);
    tableLength = tempEntriesTOC * tempSizePerTableEntry;

    int32 returnValue = tableLength + 26; //TOC bytes + Upper header bytes

    VbriHead -= 22;	//Reset the file pointer to its original place

    return returnValue;
}

/***********************************************************************
 *	Function : IsMp3File
 *	Purpose  : Verifies whether the file passed in is a possibly
 *			   valid mp3 clip
 *  Input	 : aFile, file to check
 * 			   aInitSearchFileSize, amount of data to use for verification
 *  Output	 : None
 *	Return   : error code
 *	Modified :
 ***********************************************************************/
MP3ErrorType MP3Parser::IsMp3File(MP3_FF_FILE* aFile, uint32 aInitSearchFileSize)
{
    MP3ErrorType errCode = MP3_SUCCESS;
    uint8 pFrameHeader[MP3_FRAME_HEADER_SIZE];
    uint32 firstHeader = 0;
    StartOffset = 0;
    int32 revSeek = 0;

    // get the file pointer
    fp = &(aFile->_pvfile);

    errCode = MP3Utils::SeektoOffset(fp, 0, Oscl_File::SEEKSET);
    // try to retrieve the file size
    if (MP3Utils::getCurrentFileSize(fp, iLocalFileSize))
    {
        iLocalFileSizeSet = true;
        iInitSearchFileSize = OSCL_MIN(aInitSearchFileSize, iLocalFileSize);
        if (iLocalFileSize == 0)
        {
            return MP3_END_OF_FILE;
        }
    }

    if (!iLocalFileSizeSet)
    {
        uint32 remBytes = 0;
        if (fp->GetRemainingBytes(remBytes))
        {
            iInitSearchFileSize = OSCL_MIN(iInitSearchFileSize, remBytes);
        }
    }

    // seek to the begining position in the file

    // verify if the id3 tags are present in this clip
    PVID3ParCom tagParser;
    iTagSize = 0;
    if (true == tagParser.IsID3V2Present(fp, iTagSize))
    {
        // move the file read pointer to begining of audio data
        StartOffset += iTagSize;
    }

    // seek to the begining position in the file
    MP3Utils::SeektoOffset(fp, StartOffset, Oscl_File::SEEKSET);

    if (!MP3FileIO::readByteData(fp, MP3_FRAME_HEADER_SIZE, (uint8 *)pFrameHeader))
    {
        return MP3_INSUFFICIENT_DATA;
    }
    revSeek = 0 - MP3_FRAME_HEADER_SIZE;

    firstHeader = SwapFileToHostByteOrderInt32(pFrameHeader);

    if (!GetMP3Header(firstHeader, iMP3HeaderInfo))
    {
        uint32 seekOffset = 0;
        MP3Utils::SeektoOffset(fp, 0 - MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);
        errCode = mp3FindSync(StartOffset, seekOffset);
        if (errCode == MP3_SUCCESS)
        {
            errCode = MP3Utils::SeektoOffset(fp, seekOffset, Oscl_File::SEEKCUR);
            if (MP3_SUCCESS != errCode)
            {
                return errCode;
            }
            StartOffset += seekOffset;

            if (!MP3FileIO::readByteData(fp, MP3_FRAME_HEADER_SIZE, pFrameHeader))
            {
                return MP3_INSUFFICIENT_DATA;
            }

            firstHeader = SwapFileToHostByteOrderInt32(pFrameHeader);

            if (!GetMP3Header(firstHeader, iMP3HeaderInfo))
            {
                return MP3_FILE_HDR_READ_ERR;
            }
            // retrieval of header was successful, try to decode it here.
            if (! DecodeMP3Header(iMP3HeaderInfo, iMP3ConfigInfo, false))
            {
                // Header was invalid, decoding of header failed.
                return MP3_FILE_HDR_DECODE_ERR;
            }
            return MP3_SUCCESS;
        }
        else if (errCode == MP3_INSUFFICIENT_DATA)
        {
            MP3Utils::SeektoOffset(fp, fp->Tell() - StartOffset, Oscl_File::SEEKCUR);
            return errCode;
        }
        else
        {
            // File is not identified with the provided data
            return MP3_ERROR_UNKNOWN_OBJECT;
        }
    }

    // retrieval of header was successful, try to decode it here.
    if (! DecodeMP3Header(iMP3HeaderInfo, iMP3ConfigInfo, false))
    {
        // Header was invalid, decoding of header failed.
        return MP3_FILE_HDR_DECODE_ERR;
    }
    // mp3 header was valid it is an mp3 clip
    // return success.
    return MP3_SUCCESS;
}

uint32  MP3Parser::GetFileOffsetForAutoResume(uint32& aOffset)
{
    uint32 ts = GetTimestampForCurrentSample() + 10000;
    uint32 SeekPosition = 0;

    uint32 seekpoint = SeekPointFromTimestamp(ts);
    if (seekpoint)
    {
        SeekPosition = StartOffset + seekpoint;
    }
    aOffset = SeekPosition;
    return 0;
}

/***********************************************************************
 *	Function : SetFileSize
 *	Purpose  : Notification from the lib user for the file size,
 *			   Once File size is received, the same is used to
 *			   estimate the clip duration
 *	Return   : error code
 *  Input	 : aFileSize
 *  Output	 : None
 *	Modified :
 ***********************************************************************/
MP3ErrorType MP3Parser::SetFileSize(const uint32 aFileSize)
{
    iFileSizeFromExternalSource = aFileSize;
    iLocalFileSize = aFileSize;
    return MP3_SUCCESS;
}

/**
 *	Function : EstimateDurationFromExternalFileSize
 *	Purpose  : Estimates the clip duration from external file size
 *	Return   : error code
 *	Input	 : None
 *	Output	 : Clip duration
 *	Modified :
 **/
MP3ErrorType MP3Parser::EstimateDurationFromExternalFileSize(uint32 &aClipDuration)
{
    if (iClipDurationFromEstimation > 0)
    {
        aClipDuration = iClipDurationFromEstimation;
        return MP3_SUCCESS;
    }

    if (iFileSizeFromExternalSource <= 0 || iMP3ConfigInfo.FrameLengthInBytes <= 0)
    {
        aClipDuration = 0;
        return MP3_ERROR_UNKNOWN;
    }
    uint32 fileSize = iFileSizeFromExternalSource;
    if (iId3TagParser.IsID3V2Present())
    {
        fileSize -= StartOffset;
    }
    if (iId3TagParser.IsID3V1Present())
    {
        // id3v1.x tags are 128 bytes long
        fileSize -= ID3_V1_TAG_SIZE;
    }

    if ((iMP3HeaderInfo.srIndex == 3)  || (iMP3HeaderInfo.brIndex == 15) ||
            (iMP3HeaderInfo.frameVer == 1) || (iMP3HeaderInfo.layerID != 1))
    {
        // invalid frame data, can not estimate duration
        return MP3_SUCCESS;
    }

    iClipDurationFromEstimation = (uint32)((OsclFloat)(fileSize * 8000.00f / iAvgBitrateInbps));
    aClipDuration = iClipDurationFromEstimation;
    return MP3_SUCCESS;
}

/***********************************************************************
 *	Function : GetDurationFromVBRIHeader
 *	Purpose  : Estimates the clip duration from Vbri/Xing headers
 *	Return   : error code
 *	Input	 : None
 *	Output	 : Clip duration
 *	Modified :
 ***********************************************************************/
MP3ErrorType MP3Parser::GetDurationFromVBRIHeader(uint32 &aClipDuration)
{
    if (mp3Type != EVBRIType && mp3Type != EXINGType)
    {
        return MP3_ERROR_UNKNOWN;
    }

    if ((mp3Type == EXINGType) && !(iXingHeader.flags & FRAMES_FLAG))
    {
        return MP3_ERROR_UNKNOWN;
    }

    if (iClipDurationFromVBRIHeader > 0)
    {
        aClipDuration = iClipDurationFromVBRIHeader;
        return MP3_SUCCESS;
    }

    if ((iMP3HeaderInfo.srIndex == 3)  || (iMP3HeaderInfo.brIndex == 15) ||
            (iMP3HeaderInfo.frameVer == 1) || (iMP3HeaderInfo.layerID != 1))
    {
        // invalid frame data, can not estimate duration
        return MP3_ERROR_UNKNOWN;
    }

    iClipDurationFromVBRIHeader = iNumberOfFrames * iSamplesPerFrame / iSamplingRate * 1000;
    aClipDuration = iClipDurationFromVBRIHeader;
    return MP3_SUCCESS;
}

/***********************************************************************
 *	Function : GetDurationFromRandomScan
 *	Purpose  : Estimates the clip duration by average bitrate
 *			   Average bit rate is calculated by randomnly scannning
 *			   predefined number of frames
 *	Return   : error code
 *	Input	 : None
 *	Output	 : Clip duration
 *	Modified :
 ***********************************************************************/
MP3ErrorType MP3Parser::GetDurationFromRandomScan(uint32 &aClipDuration)
{
    if (iClipDurationFromRandomScan > 0)
    {
        aClipDuration = iClipDurationFromRandomScan;
        return MP3_SUCCESS;
    }

    MP3ErrorType status = MP3_SUCCESS;
    uint32 currFilePos = MP3Utils::getCurrentFilePosition(fp);

    status = ComputeDurationFromNRandomFrames(fp);
    if (MP3_ERROR_UNKNOWN != status)
    {
        uint32 fileSz = iLocalFileSize - StartOffset;
        iClipDurationFromRandomScan = (uint32)(fileSz * 8000.00f / iAvgBitrateInbpsFromRandomScan);
        aClipDuration = iClipDurationFromRandomScan;
    }
    MP3Utils::SeektoOffset(fp, currFilePos, Oscl_File::SEEKSET);
    return status;
}

/***********************************************************************
 *	Function : ComputeDurationFromNRandomFrames
 *	Purpose  : Estimates average bit rate by randomnly scannning input
 *			   number of frames
 *	Return   : error code
 *	Input	 : numFrames
 *	Output	 : iAvgBitrateInbpsFromRandomScan
 *	Modified :
 ***********************************************************************/
MP3ErrorType MP3Parser::ComputeDurationFromNRandomFrames(PVFile * fpUsed, int32 aNumFrames, int32 aNumRandomLoc)
{
    uint32 firstHeader = 0;
    uint8 pFrameHeader[MP3_FRAME_HEADER_SIZE];
    uint32 audioOffset = 0;
    int32 totBR = 0;
    int32 avgBitRate = 0;
    int32 framecount = 0;
    uint32 randomByteOffset = 0;
    int32 audioDataSize = 0;
    MP3HeaderType mp3HeaderInfo;
    MP3ConfigInfoType mp3ConfigInfo;
    MP3ErrorType err = MP3_SUCCESS;

    oscl_memset(&mp3ConfigInfo, 0, sizeof(mp3ConfigInfo));
    oscl_memset(&mp3HeaderInfo, 0, sizeof(mp3HeaderInfo));

    // try to fetch file size
    if (iLocalFileSizeSet)
    {
        audioDataSize = iLocalFileSize;
    }

    audioDataSize -= StartOffset;
    if (iId3TagParser.IsID3V1Present())
    {
        audioDataSize -= ID3_V1_TAG_SIZE;
    }

    randomByteOffset = StartOffset;
    uint32 skipMultiple = audioDataSize / (aNumRandomLoc + 1);

    int32 numSearchLoc = 0;
    while (numSearchLoc < aNumRandomLoc)
    {
        // find random location to which we should seek in order to find
        uint32 currFilePosn = MP3Utils::getCurrentFilePosition(fpUsed);
        randomByteOffset = currFilePosn + skipMultiple;

        if (randomByteOffset > iLocalFileSize)
        {
            break;
        }
        // initialize frame count
        framecount = 0;
        audioOffset = randomByteOffset;
        MP3Utils::SeektoOffset(fpUsed, audioOffset, Oscl_File::SEEKSET);
        // Find sync
        uint32 seekOffset = 0;
        err = mp3FindSync(audioOffset, seekOffset, fpUsed);
        if (err != MP3_SUCCESS)
        {
            break;
        }
        audioOffset += seekOffset;
        MP3Utils::SeektoOffset(fpUsed, seekOffset, Oscl_File::SEEKCUR);
        // lets check rest of the frames
        while (framecount < aNumFrames)
        {
            // Read 4 bytes
            if (!MP3FileIO::readByteData(fpUsed, MP3_FRAME_HEADER_SIZE, pFrameHeader))
            {
                err = MP3_INSUFFICIENT_DATA;
                break;
            }

            firstHeader = SwapFileToHostByteOrderInt32(pFrameHeader);
            // Read header
            if (!GetMP3Header(firstHeader, mp3HeaderInfo))
            {
                err = MP3_FILE_HDR_READ_ERR;
                break;
            }
            // Decode header
            if (!DecodeMP3Header(mp3HeaderInfo, mp3ConfigInfo, false))
            {
                err = MP3_FILE_HDR_DECODE_ERR;
                break;
            }

            MP3Utils::SeektoOffset(fpUsed, mp3ConfigInfo.FrameLengthInBytes - MP3_FRAME_HEADER_SIZE, Oscl_File::SEEKCUR);
            framecount++;

            // initialize avgBitRate first time only
            if (1 == framecount)
            {
                avgBitRate = mp3ConfigInfo.BitRate;
            }

            if (mp3ConfigInfo.BitRate != avgBitRate)
            {
                avgBitRate += (mp3ConfigInfo.BitRate - avgBitRate) / framecount;
            }
        }
        totBR += avgBitRate;
        numSearchLoc++;
    }
    // calculate average bitrate
    iAvgBitrateInbpsFromRandomScan = numSearchLoc > 0 ? totBR / numSearchLoc : 0;
    if (!iAvgBitrateInbpsFromRandomScan)
    {
        return MP3_ERROR_UNKNOWN;
    }
    return err;
}

void MP3Parser::GetDurationFromCompleteScan(uint32 &aClipDuration)
{
    if (iClipDurationComputed > 0)
    {
        aClipDuration = iClipDurationComputed;
        return;
    }
    uint32 samplesPerFrame = spfIndexTable[iMP3HeaderInfo.frameVer][iMP3HeaderInfo.layerID];
    uint32 samplingRate = srIndexTable[((iMP3HeaderInfo.frameVer)*4) + iMP3HeaderInfo.srIndex];
    OsclFloat samplingRateinKHz = (OsclFloat)samplingRate / 1000;

    iClipDurationComputed = (uint32)(iScannedFrameCount * (OsclFloat)(samplesPerFrame / samplingRateinKHz));
    aClipDuration = iClipDurationComputed;
}

void MP3Parser::FillTOCTable(uint32 aFilePos, uint32 aTimeStampToFrame)
{
    if (iDurationScanComplete)
    {
        iTOC[iTOCFilledCount] = aFilePos;
        iTOCFilledCount++;
        if (0 == iTimestampPrev)
        {
            GetDurationFromCompleteScan(iBinWidth);
        }
        return;
    }

    if ((iTOCFilledCount < MAX_TOC_ENTRY_COUNT) && ((aTimeStampToFrame - iTimestampPrev) >= iBinWidth))
    {
        if (iTimestampPrev != aTimeStampToFrame)
        {
            if ((aTimeStampToFrame - iTimestampPrev) > iBinWidth)
            {
                iBinWidth = aTimeStampToFrame - iTimestampPrev;
            }
        }
        // push the file offset into TOC table
        iTOC[iTOCFilledCount] = aFilePos - StartOffset;
        iTOCFilledCount++;
        iTimestampPrev = aTimeStampToFrame;
    }
    else if (iTOCFilledCount == MAX_TOC_ENTRY_COUNT)
    {
        // run the compaction algorithm to compress the TOC table
        for (uint32 i = 0; i < (MAX_TOC_ENTRY_COUNT / 2); i++)
        {
            iTOC[i] = iTOC[2*i];
        }
        iTimestampPrev = iTimestampPrev - iBinWidth;
        iBinWidth = 2 * iBinWidth;
        iTOCFilledCount = MAX_TOC_ENTRY_COUNT / 2;
        return;
    }
}




