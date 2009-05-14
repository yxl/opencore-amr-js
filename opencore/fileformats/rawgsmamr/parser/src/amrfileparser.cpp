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

//                 A M R    F I L E    P A R S E R

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


/**
 *  @file amrfileparser.cpp
 *  @brief This file contains the implementation of the raw GSM-AMR file parser.
 */

//----------------------------------------------------------------------------
// INCLUDES
//----------------------------------------------------------------------------
#include "amrfileparser.h"

// Use default DLL entry point for Symbian
#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()


/* Table containing the sum of the frame type byte     */
/* and the number of speech bytes for each codec mode  */
/* for IETF input format                                */
static const int32 IetfDecInputBytes[NO_POSSIBLE_MODES] =
{
    13, /* 4.75 */
    14, /* 5.15 */
    16, /* 5.90 */
    18, /* 6.70 */
    20, /* 7.40 */
    21, /* 7.95 */
    27, /* 10.2 */
    32, /* 12.2 */
    6, /* GsmAmr comfort noise */
    7, /* Gsm-Efr comfort noise */
    6, /* IS-641 comfort noise */
    6, /* Pdc-Efr comfort noise */
    1, /* future use; 0 length but set to 1 to skip the frame type byte */
    1, /* future use; 0 length but set to 1 to skip the frame type byte */
    1, /* future use; 0 length but set to 1 to skip the frame type byte */
    1  /* No transmission */
};

/* Table containing the number of speech bytes for each codec mode  */
/* for IF2 input format                                             */
static const int32 If2DecInputBytes[NO_POSSIBLE_MODES] =
{
    13, /* 4.75 */
    14, /* 5.15 */
    16, /* 5.90 */
    18, /* 6.70 */
    19, /* 7.40 */
    21, /* 7.95 */
    26, /* 10.2 */
    31, /* 12.2 */
    6, /* GsmAmr comfort noise */
    6, /* Gsm-Efr comfort noise */
    6, /* IS-641 comfort noise */
    6, /* Pdc-Efr comfort noise */
    1, /* future use; 0 length but set to 1 to skip the frame type byte */
    1, /* future use; 0 length but set to 1 to skip the frame type byte */
    1, /* future use; 0 length but set to 1 to skip the frame type byte */
    1  /* No transmission */
};

static const int32 IetfWBDecInputBytes[NO_POSSIBLE_MODES] =
{
    18, /* 6.60 */
    24, /* 8.85 */
    33, /* 12.65 */
    37, /* 14.25 */
    41, /* 15.85 */
    47, /* 18.25 */
    51, /* 19.85 */
    59, /* 23.05 */
    61, /* 23.85 */
    6,  /* SID */
    1,  /* For future use : Set to 1 to skip the frame type byte */
    1,  /* For future use : Set to 1 to skip the frame type byte */
    1,  /* For future use : Set to 1 to skip the frame type byte */
    1,  /* For future use : Set to 1 to skip the frame type byte */
    1,  /* Speech lost */
    1   /* No data/transmission */
};

//! specially used in ResetPlayback(), re-position the file pointer
int32 bitstreamObject::reset(int32 filePos)
{
    iFrame_type = 0;
    iBytesRead  = iInitFilePos + filePos; // set the initial value
    iBytesProcessed = iBytesRead;
    if (ipAMRFile)
    {
        ipAMRFile->Seek(iInitFilePos + filePos, Oscl_File::SEEKSET);
    }
    iPos = bitstreamObject::MAIN_BUFF_SIZE + bitstreamObject::SECOND_BUFF_SIZE;
    return refill();
}

//! read data from bitstream, this is the only function to read data from file
int32 bitstreamObject::refill()
{
    PVMF_AMRPARSER_LOGDEBUG((0, "Refill In ipos=%d, iBytesRead=%d, iBytesProcessed=%d, iActualSize=%d, iFileSize=%d", iPos, iBytesRead, iBytesProcessed, iActual_size, iFileSize));

    if (iBytesRead > 0 && iFileSize > 0 && iBytesRead >= iFileSize)
    {
        // if number of bytes read so far exceed the file size,
        // then first update the file size (PDL case).
        if (!UpdateFileSize()) return bitstreamObject::MISC_ERROR;

        //At this point we're within 32 bytes of the end of data.
        //Quit reading data but don't return EOF until all data is processed.
        if (iBytesProcessed < iBytesRead)
        {
            return bitstreamObject::EVERYTHING_OK;
        }
        else
        {
            //there is no more data to read.
            if (iBytesRead >= iFileSize || iBytesProcessed >= iFileSize)
                return bitstreamObject::DATA_INSUFFICIENT;
        }
    }

    if (!ipAMRFile)
    {
        return bitstreamObject::MISC_ERROR;
    }

    // Get file size at the very first time
    if (iFileSize == 0)
    {
        if (ipAMRFile->Seek(0, Oscl_File::SEEKEND))
        {
            return bitstreamObject::MISC_ERROR;
        }

        iFileSize = ipAMRFile->Tell();

        if (iFileSize <= 0)
        {
            return bitstreamObject::MISC_ERROR;
        }

        if (ipAMRFile->Seek(0, Oscl_File::SEEKSET))
        {
            return bitstreamObject::MISC_ERROR;
        }

        // first-time read, set the initial value of iPos
        iPos = bitstreamObject::SECOND_BUFF_SIZE;
        iBytesProcessed = 0;
    }
    // we are currently positioned at the end of the data buffer.
    else if (iPos == bitstreamObject::MAIN_BUFF_SIZE + bitstreamObject::SECOND_BUFF_SIZE)
    {
        // reset iPos and refill from the beginning of the buffer.
        iPos = bitstreamObject::SECOND_BUFF_SIZE;
    }

    else if (iPos >= iActual_size)
    {
        int32 len = 0;
        // move the remaining stuff to the beginning of iBuffer
        if (iActual_size + bitstreamObject::SECOND_BUFF_SIZE > iPos)
        {
            // we are currently positioned within SECOND_BUFF_SIZE bytes from the end of the buffer.
            len = iActual_size + bitstreamObject::SECOND_BUFF_SIZE - iPos;
        }
        else
        {
            // no leftover data.
            len = 0;
        }

        oscl_memcpy(&iBuffer[bitstreamObject::SECOND_BUFF_SIZE-len], &iBuffer[iPos], len);
        iPos = bitstreamObject::SECOND_BUFF_SIZE - len;

        // update the file size for the PDL scenario where more data has been downloaded
        // into the file but the file size has not been updated yet.
        if (iBytesRead + iMax_size > iFileSize)
        {
            if (!UpdateFileSize()) return bitstreamObject::MISC_ERROR;
        }
    }

    // read data
    if ((iActual_size = ipAMRFile->Read(&iBuffer[bitstreamObject::SECOND_BUFF_SIZE], 1, iMax_size)) == 0)
    {
        return bitstreamObject::READ_ERROR;
    }

    iBytesRead += iActual_size;

    PVMF_AMRPARSER_LOGDEBUG((0, "Refill Out ipos=%d, iBytesRead=%d, iBytesProcessed=%d, iActualSize=%d, iFileSize=%d", iPos, iBytesRead, iBytesProcessed, iActual_size, iFileSize));

    return bitstreamObject::EVERYTHING_OK;
}

//! most important function to get one frame data plus frame type, used in getNextBundledAccessUnits()
int32 bitstreamObject::getNextFrame(uint8* frameBuffer, uint8& frame_type, bool bHeaderIncluded)
{
    PVMF_AMRPARSER_LOGDEBUG((0, "GetNextFrame In ipos=%d, iBytesRead=%d, iBytesProcessed=%d, iActualSize=%d, iFileSize=%d", iPos, iBytesRead, iBytesProcessed, iActual_size, iFileSize));
    if (!frameBuffer)
    {
        return bitstreamObject::MISC_ERROR;
    }

    int32 ret_value = bitstreamObject::EVERYTHING_OK;

    // Need to refill?
    if (iFileSize == 0 || iPos >= iActual_size)
    {
        ret_value = refill();
        if (ret_value)
        {
            return ret_value;
        }
    }

    int32 frame_size = 0;
    uint8 *pBuffer = &iBuffer[iPos];

    if (EAMRIETF_SingleNB == iAmrFormat) // IETF format single channel NB
    {
        // Search the next IETF frame header (NOT returning error when the error is corrupt)
        while ((iBuffer[iPos] & 0x83) != 0)
        {
            // Wrong IETF frame header(P+FT(4bits)+Q+PP)
            if (iPos + 1 >= iActual_size)
            {
                ret_value = refill();
                if (ret_value)
                {
                    return ret_value;
                }
            }
            iPos++;
            iBytesProcessed++;
        }
        pBuffer = &iBuffer[iPos];
        frame_type = (uint8)((pBuffer[0] >> 3) & 0x0f);
        frame_size = IetfDecInputBytes[(uint16)frame_type];
    }
    else if (EAMRIETF_SingleWB == iAmrFormat) // IETF format single channel WB
    {
        frame_type = (uint8)((pBuffer[0] >> 3) & 0x0f);
        frame_size = IetfWBDecInputBytes[(uint16)frame_type];
    }
    else if (iAmrFormat == EAMRIF2) // IF2 format
    {
        frame_type = (uint8)(pBuffer[0] & 0x0f);
        frame_size = If2DecInputBytes[(uint16)frame_type];
    }
    else
    {
        return bitstreamObject::MISC_ERROR;
    }

    PVMF_AMRPARSER_LOGDEBUG((0, "GetNextFrame Before Read frame ipos=%d, iBytesRead=%d, iBytesProcessed=%d, iActualSize=%d, iFileSize=%d", iPos, iBytesRead, iBytesProcessed, iActual_size, iFileSize));

    if (frame_size > 0)
    {
        if (bHeaderIncluded)
        {
            oscl_memcpy(frameBuffer, &pBuffer[0], frame_size);   // With frame header
        }
        else
        {
            oscl_memcpy(frameBuffer, &pBuffer[1], frame_size - 1); // NO frame header
        }
    }
    iPos += frame_size;
    iBytesProcessed += frame_size;

    PVMF_AMRPARSER_LOGDEBUG((0, "GetNextFrame Out ipos=%d, iBytesRead=%d, iBytesProcessed=%d, iActualSize=%d, iFileSize=%d", iPos, iBytesRead, iBytesProcessed, iActual_size, iFileSize));
    return ret_value;
}

//! parse the IETF bitstream header: "$!AMR" + 0x0a, and get format(IETF, IF2, or WMF)
int32 bitstreamObject::parseIETFHeader()
{
    int32 returnValue = reset();
    if (returnValue == bitstreamObject::EVERYTHING_OK)
    {
        iAmrFormat = EAMRUnrecognized;

        // first time read, we don't use iSecond_buffer
        uint8 *pBuffer = &iBuffer[iPos];
        if (iActual_size >= 5 &&
                pBuffer[0] == '#' &&
                pBuffer[1] == '!' &&
                pBuffer[2] == 'A' &&
                pBuffer[3] == 'M' &&
                pBuffer[4] == 'R')
        {

            if (pBuffer[5] == 0x0a)
            {
                // single channel AMR file
                iAmrFormat  = EAMRIETF_SingleNB;
                iInitFilePos = 6;
            }

            else if (iActual_size >= 11 &&
                     pBuffer[5]  == '_' &&
                     pBuffer[6]  == 'M' &&
                     pBuffer[7]  == 'C' &&
                     pBuffer[8]  == '1' &&
                     pBuffer[9]  == '.' &&
                     pBuffer[10] == '0' &&
                     pBuffer[11] == 0x0a)
            {
                // multi-channel AMR file
                iAmrFormat = EAMRIETF_MultiNB;
                iInitFilePos = 12;
            }
            else if (iActual_size >= 8  &&
                     pBuffer[5] == '-'  &&
                     pBuffer[6] == 'W'  &&
                     pBuffer[7] == 'B')
            {

                if (pBuffer[8] == 0x0a)
                {
                    // single channel AMR-WB file
                    iAmrFormat  = EAMRIETF_SingleWB;
                    iInitFilePos = 9;
                }
                else if (iActual_size >= 14     &&
                         pBuffer[8]   == '_'    &&
                         pBuffer[9]   == 'M'    &&
                         pBuffer[10]  == 'C'    &&
                         pBuffer[11]  == '1'    &&
                         pBuffer[12]  == '.'    &&
                         pBuffer[13]  == '0'    &&
                         pBuffer[14]  == 0x0a)
                {
                    // multi-channel AMR-WB file
                    iAmrFormat = EAMRIETF_MultiWB;
                    iInitFilePos = 15;
                }
            }
        }
        iPos += iInitFilePos;
        iBytesProcessed += iInitFilePos;

        // get the frame header
        if (iAmrFormat == EAMRUnrecognized)
        {
            iFrame_type = iBuffer[iPos] & 0xf;
        }
        else // IETF
        {
            iFrame_type = (iBuffer[iPos] >> 3) & 0xf;
        }
    }

    return returnValue;
}

//! get clip information: file size, format(IETF or IF2) and frame_type(bitrate)
int32 bitstreamObject::getFileInfo(int32& fileSize, int32& format, int32& frame_type)
{
    fileSize = format = 0;
    int32 ret_value = bitstreamObject::EVERYTHING_OK;
    if (iFileSize == 0)
    {
        ret_value = parseIETFHeader();
        if (ret_value)
        {
            return ret_value;
        }
    }

    fileSize = iFileSize;
    format = iAmrFormat;
    frame_type = iFrame_type;
    return ret_value;
}

//! get the updated file size
bool bitstreamObject::UpdateFileSize()
{
    if (ipAMRFile != NULL)
    {
        uint32 aRemBytes = 0;
        if (ipAMRFile->GetRemainingBytes(aRemBytes))
        {
            uint32 currPos = (uint32)(ipAMRFile->Tell());
            iFileSize = currPos + aRemBytes;
            return true;
        }
    }
    return false;
}


//----------------------------------------------------------------------------
// FUNCTION NAME: CAMRFileParser::CAMRFileParser
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    None
//
//  Outputs:
//    None
//
//  Returns:
//    None
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  Constructor for CAMRFileParser class
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF CAMRFileParser::CAMRFileParser(void)
{
    iAMRDuration        = -1;
    iAMRBitRate         = 0;
    iTotalNumFramesRead = 0;
    iAMRFormat          = EAMRUnrecognized;
    iEndOfFileReached   = false;
    iRandomAccessTimeInterval = 0;
    iCountToClaculateRDATimeInterval = 0;
    iLogger = PVLogger::GetLoggerObject("pvamr_parser");
    iDiagnosticLogger = PVLogger::GetLoggerObject("playerdiagnostics.pvamr_parser");

    ipBSO = NULL;
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAMRFileParser::~CAMRFileParser
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    None
//
//  Outputs:
//    None
//
//  Returns:
//    None
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  Destructor for CAMRFileParser class
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF CAMRFileParser::~CAMRFileParser(void)
{
    iAMRFile.Close();
    OSCL_DELETE(ipBSO);
    ipBSO = NULL;
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAMRFileParser::InitAMRFile
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    iClip = pointer to the AMR file name to be played of type TPtrC
//
//  Outputs:
//    None
//
//  Returns:
//    returnValue = true if the init succeeds, else false.
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function opens the AMR file, checks for AMR format type, calculates
//  the track duration, and sets the AMR bitrate value.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF bool CAMRFileParser::InitAMRFile(OSCL_wString& aClip, bool aInitParsingEnable, Oscl_FileServer* aFileSession, PVMFCPMPluginAccessInterfaceFactory*aCPM, OsclFileHandle*aHandle, uint32 countToClaculateRDATimeInterval)
{
    iAMRFile.SetCPM(aCPM);
    iAMRFile.SetFileHandle(aHandle);

    // Open the file (aClip)
    if (iAMRFile.Open(aClip.get_cstr(), (Oscl_File::MODE_READ | Oscl_File::MODE_BINARY), *aFileSession) != 0)
    {
        PVMF_AMRPARSER_LOGERROR((0, "CAMRFileParser::InitAMRFile- File Open failed"));
        return false;
    }

    // create ipBSO
    ipBSO = OSCL_NEW(bitstreamObject, (iLogger, &iAMRFile));
    if (!ipBSO)
    {
        return false;
    }
    if (!ipBSO->get())
    {
        return false; // make sure the memory allocation is going well
    }

    // get file info
    int32 frameTypeIndex = 0;
    if (ipBSO->getFileInfo(iAMRFileSize, iAMRFormat, frameTypeIndex))
    {
        PVMF_AMRPARSER_LOGERROR((0, "CAMRFileParser::InitAMRFile- getFileInfo failed "));
        return false;
    }

    // Reject unsupported AMR file types
    if (frameTypeIndex >= 16 ||
            iAMRFormat == EAMRETS ||
            iAMRFormat == EAMRIETF_MultiNB ||
            iAMRFormat == EAMRIETF_MultiWB ||
            iAMRFormat == EAMRWMF ||
            iAMRFormat == EAMRUnrecognized)
    {
        PVMF_AMRPARSER_LOGERROR((0, "CAMRFileParser::Unsupported AMR type "));
        return false;
    }

    // Set bitrate
    if (EAMRIETF_SingleWB != iAMRFormat)
    {
        AMRFF_Frame_Type_3GPP frameType3GPP = (AMRFF_Frame_Type_3GPP)frameTypeIndex;
        SetBitRate(frameType3GPP);
    }
    else // for wide band
    {
        AMRFF_WB_Frame_Type_3GPP frameType3GPP = (AMRFF_WB_Frame_Type_3GPP)frameTypeIndex;
        SetBitRate(frameType3GPP);
    }

    // Determine file duration and set up random positioning table if needed
    CalculateDuration(aInitParsingEnable, countToClaculateRDATimeInterval);
    return true;
}

bool CAMRFileParser::CalculateDuration(bool aInitParsingEnable, uint32 countToClaculateRDATimeInterval)
{
    iCountToClaculateRDATimeInterval = countToClaculateRDATimeInterval;
    uint32 FrameCount = iCountToClaculateRDATimeInterval;
    iRandomAccessTimeInterval = countToClaculateRDATimeInterval * TIME_STAMP_PER_FRAME;

    if (aInitParsingEnable)
    {
        // Go through each frame to calculate AMR file duration.
        int32 status = bitstreamObject::EVERYTHING_OK;
        uint8 frame_type = 15;
        iAMRDuration = 0;

        int32 error = 0;

        int32 filePos = 0;

        OSCL_TRY(error, iRPTable.push_back(filePos));
        OSCL_FIRST_CATCH_ANY(error, return false);

        while (status == bitstreamObject::EVERYTHING_OK)
        {
            // get the next frame

            status = ipBSO->getNextFrame(iAMRFrameBuffer, frame_type); // NO IETF frame header

            if (status == bitstreamObject::EVERYTHING_OK)
            {
                // calculate the number of frames // BX
                iAMRDuration += TIME_STAMP_PER_FRAME;

                // set up the table for randow positioning
                int32 frame_length = 0;

                if (iAMRFormat == EAMRIF2)
                {
                    frame_length = If2DecInputBytes[(uint16)frame_type];
                }
                else if (iAMRFormat == EAMRIETF_SingleNB)
                {
                    frame_length = IetfDecInputBytes[(uint16)frame_type];
                }
                else if (EAMRIETF_SingleWB == iAMRFormat)
                {
                    frame_length = IetfWBDecInputBytes[(uint16)frame_type];
                }
                else
                {
                    PVMF_AMRPARSER_LOGERROR((0, "CAMRFileParser::CalculateDuration- Format unknown"));
                    return false;
                }

                filePos += frame_length;

                error = 0;
                if (!FrameCount)
                {
                    OSCL_TRY(error, iRPTable.push_back(filePos));
                    OSCL_FIRST_CATCH_ANY(error, return false);
                    FrameCount = countToClaculateRDATimeInterval;
                }
            }

            else if (status == bitstreamObject::END_OF_FILE)
            {
                break;
            }

            else
            {
                // error happens!
                PVMF_AMRPARSER_LOGERROR((0, "CAMRFileParser::getNextFrame Fails Error Code %d", status));
                if (ipBSO->reset())
                {
                    return false;
                }

                return false;
            }
            FrameCount--;
        }

        ResetPlayback(0);
    }
    return true;
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAMRFileParser::RetrieveAMRFileInfo
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    aMedia     = pointer to CPVMedia class
//    aTrackId   = pointer to the ID specific to the current AMR track
//    aTimescale = pointer to the sampling frequency value, for AMR it is 8000 Hz.
//
//  Outputs:
//    None
//
//  Returns:
//    false if an error happens, else true
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function opens the AMR file, checks for AMR format type, calculates
//  the track duration, and sets the AMR bitrate value.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF bool CAMRFileParser::RetrieveFileInfo(TPVAmrFileInfo& aInfo)
{
    if (iAMRFormat == EAMRUnrecognized)
    {
        // File is not open and parsed
        return false;
    }

    aInfo.iBitrate = iAMRBitRate;
    aInfo.iTimescale = 1000;
    aInfo.iDuration = iAMRDuration;
    aInfo.iFileSize = iAMRFileSize;
    aInfo.iAmrFormat = iAMRFormat;
    PVMF_AMRPARSER_LOGDIAGNOSTICS((0, "CAMRFileParser::RetrieveFileInfo- duration = %d, bitrate = %d, filesize = %d", iAMRDuration, iAMRBitRate, iAMRFileSize));

    return true;
}


//----------------------------------------------------------------------------
// FUNCTION NAME: CAMRFileParser::SetBitRate
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    aFrameType3GPP = 3GPP frame type for the AMR frame of type AMRFF_Frame_Type_3GPP
//
//  Outputs:
//    None
//
//  Returns:
//    None
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function takes in the 3GPP frametype and sets the AMR bitrate value
//  depending on the AMR frametype.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
void CAMRFileParser::SetBitRate(AMRFF_Frame_Type_3GPP aFrameType3GPP)
{
    switch (aFrameType3GPP)
    {
        case AMRFF_AMR_475:
        {
            iAMRBitRate =  4750;
            break;
        }
        case AMRFF_AMR_515:
        {
            iAMRBitRate =  5150;
            break;
        }
        case AMRFF_AMR_59:
        {
            iAMRBitRate =  5900;
            break;
        }
        case AMRFF_AMR_67:
        {
            iAMRBitRate =  6700;
            break;
        }
        case AMRFF_AMR_74:
        {
            iAMRBitRate =  7400;
            break;
        }
        case AMRFF_AMR_795:
        {
            iAMRBitRate =  7950;
            break;
        }
        case AMRFF_AMR_102:
        {
            iAMRBitRate = 10200;
            break;
        }
        case AMRFF_AMR_122:
        {
            iAMRBitRate = 12200;
            break;
        }
        default:
        {
            iAMRBitRate = 0;
            break;
        }
    }
    return;
}


void CAMRFileParser::SetBitRate(AMRFF_WB_Frame_Type_3GPP aFrameType3GPP)
{
    switch (aFrameType3GPP)
    {
        case AMRFF_WB_660:
        {
            iAMRBitRate =  6600;
            break;
        }
        case AMRFF_WB_885:
        {
            iAMRBitRate =  8850;
            break;
        }
        case AMRFF_WB_1265:
        {
            iAMRBitRate =  12650;
            break;
        }
        case AMRFF_WB_1425:
        {
            iAMRBitRate =  14250;
            break;
        }
        case AMRFF_WB_1585:
        {
            iAMRBitRate =  15850;
            break;
        }
        case AMRFF_WB_1825:
        {
            iAMRBitRate =  18250;
            break;
        }
        case AMRFF_WB_1985:
        {
            iAMRBitRate = 19850;
            break;
        }
        case AMRFF_WB_2305:
        {
            iAMRBitRate = 23050;
            break;
        }
        case AMRFF_WB_2385:
        {
            iAMRBitRate = 23850;
            break;
        }
        default:
        {
            iAMRBitRate = 0;
            break;
        }
    }
    return;
}



//----------------------------------------------------------------------------
// FUNCTION NAME: CAMRFileParser::ResetPlayback
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    aStartTime = integer value as where to move the playback positioning to.
//
//  Outputs:
//    None
//
//  Returns:
//    0 if success, -1 if failure
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function sets the file pointer to the location that aStartTime would
//  point to in the file.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF int32 CAMRFileParser::ResetPlayback(int32 aStartTime)
{
    // get file size info, //iAMRFile.Size(fileSize)
    int32 result;
    if (iAMRFileSize <= 0)
    {
        int32 frameTypeIndex;
        if (ipBSO->getFileInfo(iAMRFileSize, iAMRFormat, frameTypeIndex))
        {
            PVMF_AMRPARSER_LOGERROR((0, "CAMRFileParser::Reset Playback Failed"));
            return bitstreamObject::MISC_ERROR;
        }
    }

    iEndOfFileReached = false;
    // initialize "iTotalNumFramesRead"
    // note: +1 means we choose the next frame(ts>=aStartTime)
    iTotalNumFramesRead = aStartTime / TIME_STAMP_PER_FRAME + (aStartTime > 0) * 1;

    uint32 tblIdx = aStartTime / (iRandomAccessTimeInterval);// +(aStartTime>0)*1;
    iTotalNumFramesRead = tblIdx * iCountToClaculateRDATimeInterval;

    PVMF_AMRPARSER_LOGDIAGNOSTICS((0, "CAMRFileParser::resetplayback - TotalNumFramesRead=%d", iTotalNumFramesRead));
    // set new file position
    int32 newPosition = 0;
    if (iTotalNumFramesRead > 0)
    {
        // At the first time, don't do reset
        if (iAMRDuration != 0 && iRPTable.size() == 0)
        {
            newPosition = (iAMRFileSize * aStartTime) / iAMRDuration;
            PVMF_AMRPARSER_LOGDIAGNOSTICS((0, "CAMRFileParser::resetplayback - newPosition=%d", newPosition));
            if (newPosition < 0)
            {
                // if we have no duration information, reset the file position at 0.
                newPosition = 0;
            }
        }
        else if (iRPTable.size() > 0)
        {
            // use the randow positioning table to determine the file position
            if (tblIdx  >= iRPTable.size())
            {
                // Requesting past the end of table so set to (end of table-1)
                // to be at the last sample
                tblIdx = ((int32)iRPTable.size()) - 2;
            }
            newPosition = iRPTable[tblIdx];
        }
    }
    result = ipBSO->reset(newPosition);
    if (newPosition >= 0 && result)
    {
        PVMF_AMRPARSER_LOGERROR((0, "AMRBitstreamObject::refill- Misc Error"));
        return result;
    }
    iEndOfFileReached = false;

    return bitstreamObject::EVERYTHING_OK;
}


//----------------------------------------------------------------------------
// FUNCTION NAME: CAMRFileParser::SeekPointFromTimestamp
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    aStartTime = integer value as for the specified start time
//
//  Outputs:
//    None
//
//  Returns:
//    Timestamp in milliseconds of the actual position
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function returns the timestamp for an actual position corresponding
//  to the specified start time
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF uint32 CAMRFileParser::SeekPointFromTimestamp(uint32 aStartTime)
{
    // get file size info, //iAMRFile.Size(fileSize)
    if (iAMRFileSize <= 0)
    {
        int32 frameTypeIndex;
        if (ipBSO->getFileInfo(iAMRFileSize, iAMRFormat, frameTypeIndex))
        {
            return 0;
        }
    }

    // Determine the frame number corresponding to timestamp
    // note: +1 means we choose the next frame(ts>=aStartTime)
    uint32 startframenum = aStartTime / TIME_STAMP_PER_FRAME + (aStartTime > 0) * 1;

    // Correct the frame number if necessary
    if (startframenum > 0)
    {
        if (iAMRDuration != 0 && iRPTable.size() <= 0)
        {
            // Duration not known and reposition table not available so go to first frame
            startframenum = 0;
        }
        else if (iRPTable.size() > 0)
        {
            if (startframenum >= iRPTable.size())
            {
                // Requesting past the end of table so set to (end of table-1)
                // to be at the last sample
                startframenum = ((int32)iRPTable.size()) - 2;
            }
        }
    }

    return (startframenum*TIME_STAMP_PER_FRAME);
}

//----------------------------------------------------------------------------
// FUNCTION NAME: CAMRFileParser::GetNextBundledAccessUnits
//----------------------------------------------------------------------------
// INPUT AND OUTPUT DEFINITIONS
//
//  Inputs:
//    aNumSamples = requested number of frames to be read from file
//    aGau        = frame information structure of type GAU
//
//  Outputs:
//    None
//
//  Returns:
//    0 if success, -1 if failure
//
//  Global Variables Used:
//    None
//
//----------------------------------------------------------------------------
// FUNCTION DESCRIPTION
//
//  This function attempts to read in the number of AMR frames specified by
//  aNumSamples. It formats the read data to WMF bit order and stores it in
//  the GAU structure.
//
//----------------------------------------------------------------------------
// REQUIREMENTS
//
//----------------------------------------------------------------------------
// REFERENCES
//
//------------------------------------------------------------------------------
// CAUTION
//
//------------------------------------------------------------------------------
OSCL_EXPORT_REF int32 CAMRFileParser::GetNextBundledAccessUnits(uint32 *aNumSamples, GAU *aGau)
{
    // AMR format has already been identified in InitAMRFile function.
    // Check if AMR format is valid as the safeguard
    if (iAMRFormat == EAMRUnrecognized)
    {
        PVMF_AMRPARSER_LOGERROR((0, "CAMRFileParser::GetNextBundledAccessUnits Failed - Unrecognized format"));
        return bitstreamObject::MISC_ERROR;
    }

    // Check the requested number of frames is not greater than the max supported
    if (*aNumSamples > MAX_NUM_FRAMES_PER_BUFF)
    {
        PVMF_AMRPARSER_LOGERROR((0, "CAMRFileParser::GetNextBundledAccessUnits Failed - requested number of frames is greater than the max supported"));
        return bitstreamObject::MISC_ERROR;
    }

    int32 returnValue = bitstreamObject::EVERYTHING_OK;

    if (iEndOfFileReached)
    {
        *aNumSamples = 0;
        return bitstreamObject::END_OF_FILE;
    }

    uint8* pTempGau = (uint8 *) aGau->buf.fragments[0].ptr;
    uint32 gauBufferSize = aGau->buf.fragments[0].len;
    uint32 i, bytesReadInGau = 0, numSamplesRead = 0;

    for (i = 0; i < *aNumSamples && !iEndOfFileReached; i++)
    {
        // get the next frame
        bool bHeaderIncluded = true;
        returnValue = ipBSO->getNextFrame(iAMRFrameBuffer, iAMRFrameHeaderBuffer[i], bHeaderIncluded);
        if (returnValue == bitstreamObject::END_OF_FILE)
        {
            iEndOfFileReached = true;
            break;
        }
        else if (returnValue == bitstreamObject::EVERYTHING_OK)

        {
        }
        else if (returnValue == bitstreamObject::DATA_INSUFFICIENT)
        {
            *aNumSamples = 0;
            return returnValue;
        }
        else
        {   // error happens!!
            *aNumSamples = 0;
            return bitstreamObject::READ_ERROR;
        }

        // Now a frame exists in iAMRFrameBuffer, move it to aGau
        int32 frame_size = 0;
        if (iAMRFormat == EAMRIF2)
        {
            frame_size = If2DecInputBytes[(uint16)iAMRFrameHeaderBuffer[i]];
        }
        else if (iAMRFormat == EAMRIETF_SingleNB)
        {
            frame_size = IetfDecInputBytes[(uint16)iAMRFrameHeaderBuffer[i]];
        }
        else if (iAMRFormat == EAMRIETF_SingleWB)
        {
            frame_size = IetfWBDecInputBytes[(uint16)iAMRFrameHeaderBuffer[i]];
        }
        else
        {
            PVMF_AMRPARSER_LOGERROR((0, "AMRBitstreamObject::refill- Misc Error"));
            return bitstreamObject::MISC_ERROR;
        }

        // Check whether the gau buffer will be overflow
        if (bytesReadInGau + frame_size >= gauBufferSize)
        {
            // Undo the read
            ipBSO->undoGetNextFrame(frame_size);
            break;
        }

        if (frame_size > 0)
        {
            oscl_memcpy(pTempGau, iAMRFrameBuffer, frame_size);

            pTempGau += frame_size;
            bytesReadInGau += frame_size;
        }
        aGau->info[i].len = frame_size;
        aGau->info[i].ts  = (iTotalNumFramesRead + (numSamplesRead++)) * TIME_STAMP_PER_FRAME;

    } // end of: for(i = 0; i < *aNumSamples && !iEndOfFileReached; i++)

    aGau->info[0].ts = iTotalNumFramesRead * TIME_STAMP_PER_FRAME;

    *aNumSamples = numSamplesRead;
    iTotalNumFramesRead += numSamplesRead;

    //We may have reached EOF but also found some samples.
    //don't return EOF until there are no samples left.
    if (returnValue == bitstreamObject::END_OF_FILE
            && numSamplesRead > 0)
        return bitstreamObject::EVERYTHING_OK;

    return returnValue;
}

OSCL_EXPORT_REF int32 CAMRFileParser::PeekNextTimestamp(uint32 *aTimestamp)
{

    *aTimestamp = iTotalNumFramesRead * TIME_STAMP_PER_FRAME;

    return bitstreamObject::EVERYTHING_OK;
}


OSCL_EXPORT_REF uint8 CAMRFileParser::GetFrameTypeInCurrentBundledAccessUnits(uint32 frameIndex)
{
    if (frameIndex >= MAX_NUM_FRAMES_PER_BUFF)
    {
        return 0;
    }
    return iAMRFrameHeaderBuffer[frameIndex];
}

OSCL_EXPORT_REF int32 CAMRFileParser::getTrackMaxBufferSizeDB()
{
    if (ipBSO)
    {
        return ipBSO->getTrackMaxBufferSizeDB();
    }
    else
    {
        return -1;
    }
}


OSCL_EXPORT_REF uint8* CAMRFileParser::getCodecSpecificInfo()
{

    bool bHeaderIncluded = true;
    ipBSO->getNextFrame(iAMRFrameBuffer, iAMRFrameHeaderBuffer[1], bHeaderIncluded);
    return 	iAMRFrameHeaderBuffer;
}


