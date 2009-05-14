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

//                 A A C   F I L E   P A R S E R

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/**
 *  @file aacfileparser.h
 *  @brief This file defines the raw AAC file parser.
 */

#ifndef AACFILEPARSER_H_INCLUDED
#define AACFILEPARSER_H_INCLUDED

//----------------------------------------------------------------------------
// INCLUDES
//----------------------------------------------------------------------------

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef PV_GAU_H
#include "pv_gau.h"
#endif

#ifndef PVFILE_H_INCLUDED
#include "pvfile.h"
#endif

#ifndef PV_ID3_PARCOM_H_INCLUDED
#include "pv_id3_parcom.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

//----------------------------------------------------------------------------
// CONSTANTS
//----------------------------------------------------------------------------

#define PACKET_INDICATOR_LENGTH           4
#define ADIF_HEADER_MINUS_INDICTATOR     16
#define ADTS_HEADER_LENGTH                7
#define BYTES_PER_SILENCE_FRAME          23
#define AAC_DECODER_SPECIFIC_INFO_SIZE    2
#define MAX_AAC_FRAME_SIZE              8192 // 8192 = 2^13, 13bit AAC frame size (in bytes)
#define MAX_ADTS_PACKET_LENGTH          MAX_AAC_FRAME_SIZE
#define AAC_DECODER_INPUT_BUFF_SIZE     1536 // 6144 (bits) * 2 (channels) / 8 (bits per byte)

//  ADTS sync  parameters

//#define PERCENTAGE_IN_POW_2    6  //   2^(-6) ==  1.56 %, search over  1.56% of file size
#define PERCENTAGE_IN_POW_2    5    //   2^(-5) ==  3.13 %, search over  3.13% of file size 
//#define PERCENTAGE_IN_POW_2    4  //   2^(-4) ==  6.25 %, search over  6.25% of file size
//#define PERCENTAGE_IN_POW_2    3  //   2^(-3) == 12.50 %, search over 12.50% of file size

#define ADTS_SYNC_SEARCH_LENGTH(m, PERCENTAGE_IN_POW_2)    (m>>PERCENTAGE_IN_POW_2)


#define PVMF_AACPARSER_LOGDIAGNOSTICS(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iDiagnosticLogger,PVLOGMSG_INFO,m);
#define PVMF_AACPARSER_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);


#define PV_AAC_FF_NEW(auditCB,T,params,ptr)\
{\
ptr = OSCL_NEW(T,params);\
}


#define PV_AAC_FF_DELETE(auditCB,T,ptr)\
{\
OSCL_DELETE(ptr);\
}

#define PV_AAC_FF_TEMPLATED_DELETE(auditCB,T,Tsimple,ptr)\
{\
OSCL_DELETE(ptr);\
}

#define PV_AAC_FF_ARRAY_MALLOC(auditCB,T,count,ptr)\
{\
    ptr = (T*)OSCL_MALLOC(count);\
}


#define PV_AAC_ARRAY_FREE(auditCB,ptr)\
{\
    OSCL_FREE(ptr);\
}

#define PV_AAC_FF_ARRAY_NEW(auditCB, T, count, ptr)\
{\
    ptr = OSCL_ARRAY_NEW(T, count);\
}

#define PV_AAC_ARRAY_DELETE(auditCB, ptr)\
{\
    OSCL_ARRAY_DELETE(ptr);\
}

enum ParserErrorCode
{
    GENERIC_ERROR     = -4,
    INSUFFICIENT_DATA = -3,
    FILE_OPEN_ERROR = -2,
    MEMORY_ERROR    = -1,
    OK              = 1
};

/*
 * AAC format types supported
 */
enum TAACFormat
{
    EAACADTS,
    EAACADIF,
    EAACRaw,
    EAACUnrecognized
};

/*
 * Sampling Frequency look up table
 * The look up index is found in the
 * header of an ADTS packet
 */
static const int32 ADTSSampleFreqTable[16] =
{
    96000, /* 96000 Hz */
    88200, /* 88200 Hz */
    64000, /* 64000 Hz */
    48000, /* 48000 Hz */
    44100, /* 44100 Hz */
    32000, /* 32000 Hz */
    24000, /* 24000 Hz */
    22050, /* 22050 Hz */
    16000, /* 16000 Hz */
    12000, /* 12000 Hz */
    11025, /* 11025 Hz */
    8000, /*  8000 Hz */
    7350, /*  7350 Hz */
    -1, /* future use */
    -1, /* future use */
    -1  /* escape value */
};

/*
 * Table containing silence frame to be generated
 */
static const uint8 SilenceFrameStereo[BYTES_PER_SILENCE_FRAME] =
{
    0x21, 0x10, 0x03, 0x20, 0x64, 0x1B,
    0xC0, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x38
};

typedef struct
{
    int32       iTimescale;
    int32       iDuration;
    int32       iSampleFrequency;
    int32       iBitrate;
    TAACFormat  iFormat;
    int32       iFileSize;
} TPVAacFileInfo;


//----------------------------------------------------------------------------
// FORWARD CLASS DECLARATIONS
//----------------------------------------------------------------------------


/**
 *  @brief The AACBitstreamObject Class is the class used by the AAC parser to
 *  manipulate the bitstream read from the file.
 */

class AACBitstreamObject
{
    public:
        enum
        {
            MAIN_BUFF_SIZE = 8192,

            // error types for GetNextBundledAccessUnits(),
            // the definition is consistent with MP4_ERROR_CODE in iSucceedFail.h
            MISC_ERROR = -2,
            READ_ERROR = -1,
            EVERYTHING_OK = 0,
            END_OF_FILE = 62,
            INSUFFICIENT_DATA = 141
        };

        /**
        * @brief Constructor
        *
        * @param pFile Pointer to file pointer containing bitstream
        * @returns None
        */
        AACBitstreamObject(PVFile* file)
        {
            iLogger = PVLogger::GetLoggerObject("pvBitstream object");
            oscl_memset(this, 0, sizeof(AACBitstreamObject));
            init(file);
            iBuffer = OSCL_ARRAY_NEW(uint8, AACBitstreamObject::MAIN_BUFF_SIZE);
            if (!iBuffer)
            {
                iStatus = true;
            }
            else
            {
                iStatus = false;
            }
            id3Parser = OSCL_NEW(PVID3ParCom, ());
        }

        /**
        * @brief Destructor
        *
        * @param None
        * @returns None
        */
        ~AACBitstreamObject()
        {
            /*init();*/
            if (ipAACFile != NULL)
            {
                ipAACFile->Close();
                ipAACFile = NULL;
            }
            if (iBuffer)
            {
                OSCL_ARRAY_DELETE(iBuffer);
                iBuffer = NULL;
            }

            if (id3Parser)
            {
                OSCL_DELETE(id3Parser);
                id3Parser = NULL;
            }


        }

        /**
        * @brief Returns current bitstream status
        *
        * @param None
        * @returns true=Bitstream instantiated; false=no bitstream
        */
        inline bool get()
        {
            return iStatus;
        }

        /**
        * @brief Returns status of CRC
        *
        * @param None
        * @returns true=CRC enabled; false=CRC disabled
        */
        inline bool isCRCEnabled()
        {
            return ibCRC_Check;
        }

        /**
        * @brief Re-positions the file pointer. Specially used in ResetPlayback()
        *
        * @param filePos Position in file to move to.
        * @returns Result of operation: EVERYTHING_OK, READ_ERROR etc.
        */
        int32 reset(int32 filePos);

        /**
        * @brief Retrieves clip information: file size, format(ADIF or ADTS),
        * sampling rate index, bitrate and header length
        *
        * @param fileSize Size of file
        * @param format Format of fikle (ADIF or ADTS)
        * @param sampleFreqInd Sampling frequency index
        * @param bitRate Bit rate
        * @param adifHeaderLen Length of header
        * @returns Result of operation: EVERYTHING_OK, READ_ERROR etc.
        */
        int32 getFileInfo(int32& fileSize, TAACFormat& format, uint8& sampleFreqIndex, uint32& bitRate, uint32& adifHeaderLen, OSCL_wString&);

        /**
        * @brief Expanded adts search for file information
        * sampling rate index, bitrate and header length
        *
        * @param fileSize Size of file
        * @param sampleFreqInd Sampling frequency index
        * @param bitRate Bit rate
        * @returns Result of operation: EVERYTHING_OK, READ_ERROR etc.
        */
        int32 extendedAdtsSearchForFileInfo(TAACFormat& format, uint8& sampleFreqIndex);

        /**
        * @brief Retrieves frame size and number of data blocks for the next frame, in
        * preparation of getNextFrame()
        *
        * @param frame_size Length of next frame
        * @param numDateBlocks Number of data blocks of next frame
        * @returns Result of operation: EVERYTHING_OK, READ_ERROR etc.
        */
        int32 getNextFrameInfo(int32& frame_size, int32& numDateBlocks);

        /**
        * @brief get one frame data plus frame size and number of data blocks,
        * used in getNextBundledAccessUnits()
        *
        * @param frameBuffer Buffer containing frame read
        * @param frame_size Length of frame
        * @param bHeaderIncluded Indicates whether header is to be included or not
        * @returns Result of operation: EVERYTHING_OK, READ_ERROR etc.
        */
        int32 getNextFrame(uint8* frameBuffer, int32 &frame_size, int32& header_size, bool bHeaderIncluded = false);

        /**
        * @brief Parses the ADTS header and construct MPEG-4 AAC decoder config header
        *
        * @param headerBuffer Buffer containing header
        * @returns Result of operation: EVERYTHING_OK, READ_ERROR etc.
        */
        int32 getDecoderConfigHeader(uint8* headerBuffer);

        /**
        * @brief Parses the ID3 header
        *
        * @param none
        * @returns none
        */
        void parseID3Header(PVFile&);

        /**
        * @brief Find  ADTS sync word
        *
        * @param frameBuffer Buffer containing frame read
        * @returns Result of operation: offset on bytes on success, -1 when no sync word found.
        */
        int32 find_adts_syncword(uint8 *pBuffer);

        /**
        * @brief Determines if clip is AAC
        *
        * @param PVFile&
        * @returns Result of operation: EVERYTHING_OK, READ_ERROR etc.
        */
        int32 isAACFile();

    private:

        /**
        * @brief Initialization
        *
        * @param pFile Pointer to file pointer containing bitstream
        * @returns None
        */
        inline void init(PVFile* pFile = NULL)
        {
            iFileSize = iBytesRead = iBytesProcessed = 0;
            ipAACFile = pFile;
            iActual_size = iMax_size = AACBitstreamObject::MAIN_BUFF_SIZE;
            iPos = AACBitstreamObject::MAIN_BUFF_SIZE;
            iAACFormat = EAACUnrecognized;
            if (ipAACFile)
            {
                ipAACFile->Seek(0, Oscl_File::SEEKSET);
            }
        }

        /**
        * @brief Reads data from bitstream, this is the only function to read data from file
        *
        * @param None
        * @returns Result of operation: EVERYTHING_OK, READ_ERROR etc.
        */
        int32 refill();

    private:
        int32 iPos;             // pointer for buffer[]
        int32 iActual_size;     // number of bytes read from a file once <= max_size
        int32 iMax_size;        // max_size = bitstreamStruc::MAIN_BUFF_SIZE
        int32 iBytesRead;       // (cumulative) number of bytes read from a file so far
        int32 iBytesProcessed;  // (cumulative) number of bytes processed so far.
        int32 iFileSize;        // file size of the ipAACFile
        TAACFormat iAACFormat;  // 0 : ADTS  1: ADIF  2 : Raw
        bool ibCRC_Check;       // CRC check flag
        bool iStatus;           // 1: ok for memory allocation in constructor
        // 0: fail in memory allocation in constructor
        uint8 iSampleFreqIndex; // index for sampling rate
        int32 iPosSyncAdtsFound;// pointer for where in buffer[] the search algo. found a
        // possible match, used for extended search

        uint8 *iBuffer;
        uint8 iAACHeaderBuffer[PACKET_INDICATOR_LENGTH+12];
        PVFile* ipAACFile; // bitstream file

        uint8 iAudioObjectType; // audio object type
        uint32 iChannelConfig;  // channel configuration
        uint32 iADIFHeaderLen;  // variable length
        uint32 iRawAACHeaderLen; // Audio specific config size in bytes for raw AAC bitstream files
        uint32 iBitrate;        // max bitrate for variable rate bitstream
        PVID3ParCom* id3Parser;
        PVLogger  *iLogger;

    public:
        uint32 GetByteOffsetToStartOfAudioFrames()
        {
            return id3Parser->GetByteOffsetToStartOfAudioFrames();

        }

        void ID3MetaData(PvmiKvpSharedPtrVector &id3Frames)
        {
            id3Parser->GetID3Frames(id3Frames);

        }

        bool IsID3Frame(const OSCL_String& frameType)
        {
            return id3Parser->IsID3FrameAvailable(frameType);
        }

        void GetID3Frame(const OSCL_String& aFrameType, PvmiKvpSharedPtrVector& aFrame)
        {
            id3Parser->GetID3Frame(aFrameType, aFrame);
        }

        PVID3Version GetID3Version() const
        {
            return id3Parser->GetID3Version();
        }

};


/**
 *  @brief The CAACFileParser Class is the class that will construct and maintain all the
 *  necessary data structures to be able to render a valid AAC file to disk.
 *
 *  This class supports the following AAC file format specs:
 *     1) ADIF
 *     2) ADTS
 *     3) Raw bitstream with audio specific config
 */
class CAACFileParser
{
    public:
        typedef OsclMemAllocator alloc_type;

        /**
        * @brief Constructor
        *
        * @param None
        * @returns None
        */
        OSCL_IMPORT_REF  CAACFileParser();

        /**
        * @brief Destructor
        *
        * @param None
        * @returns None
        */
        OSCL_IMPORT_REF ~CAACFileParser();

        /**
        * @brief Opens the specified file and performs initialization of the parser
        *
        * @param aClip Filename to parse
        * @param aInitParsingEnable Indicates whether to setup random positioning (true)
        * or not (false)
        * @param aFileSession Pointer to opened file server session. Used when opening
        * and reading the file on certain operating systems.
        * @returns true if the init succeeds, else false.
        */
        OSCL_IMPORT_REF bool InitAACFile(OSCL_wString& aClip,  bool aInitParsingEnable = true, Oscl_FileServer* aFileSession = NULL, PVMFCPMPluginAccessInterfaceFactory* aCPMAccess = NULL, OsclFileHandle* aHandle = NULL);

        /**
        * @brief Resets the parser variables so playback can be restarted at the
        * specified time.
        *
        * @param aStartTime value as where to start repositioning to
        * @returns Result of operation: EVERYTHING_OK,
        */
        OSCL_IMPORT_REF int32 ResetPlayback(uint32 aStartTime, uint32& aActualStartTime);

        /**
        * @brief Returns the actual starting timestamp for a specified start time
        *
        * @param aStartTime Time where to start playback from
        * @returns Timestamp corresponding to the actual start position
        */
        OSCL_IMPORT_REF uint32 SeekPointFromTimestamp(uint32 aStartTime = 0);

        /**
        * @brief Attempts to read in the number of audio frames specified by aNumSamples
        *
        * @param aNumSamples Requested number of frames to be read from file
        * @param aGau Frame information structure of type GAU
        * @param bADTSHeaderIncluded, set to true in case ADTS headers need to be included
        * @returns Result of operation: EVERYTHING_OK,
        */
        OSCL_IMPORT_REF int32 GetNextBundledAccessUnits(uint32 *aNumSamples,
                GAU *aGau,
                bool bADTSHeaderIncluded = false);

        /**
        * @brief Returns the value of the timestamp for the next frame, as would
        *    be returned by GetNextBundledAccessUnits, but does not process any data.
        *
        * @param aTimestamp Next timestamp.
        * @returns Result of operation: EVERYTHING_OK,
        */
        OSCL_IMPORT_REF int32 PeekNextTimestamp(uint32&aTimestamp);

        /**
        * @brief Retrieves information about the clip such as bit rate, sampling frequency, etc.
        *
        * @param aInfo Storage for information retrieved
        * @returns True if successful, False otherwise.
        */
        OSCL_IMPORT_REF bool RetrieveFileInfo(TPVAacFileInfo& aInfo);

        /**
        * @brief Retrieves information about the clip from the ID3 tags if any.
        *
        * @param aInfo Storage for information retrieved
        * @returns True if successful, False otherwise.
        */
        OSCL_IMPORT_REF bool RetrieveID3Info(PvmiKvpSharedPtrVector& aID3MetaData);
        /**
        * @brief checks if ID3 frame is available.
        *
        * @param frameType
        * @returns True if successful, False otherwise.
        */
        OSCL_IMPORT_REF bool IsID3Frame(const OSCL_String &frameType);

        /**
        * @brief retrieves ID3 Frame.
        *
        * @param frameType ID of the frame to be retrieved
        * @param aFrame data of the frame of type frametype
        * @returns True if successful, False otherwise.
        */

        OSCL_IMPORT_REF void GetID3Frame(const OSCL_String& aFrameType, PvmiKvpSharedPtrVector& aFrame);

        /**
        * @brief Returns the size of the decoder specific info
        *
        * @param None
        * @returns Length of decoder specific info
        */
        OSCL_IMPORT_REF int32 GetTrackDecoderSpecificInfoSize(void);

        /**
        * @brief Returns the content of the decoder specific info
        *
        * @param None
        * @returns Pointer to decoder specific info
        */
        OSCL_IMPORT_REF uint8* GetTrackDecoderSpecificInfoContent(void);

        /**
        * @brief Returns the format (ADTS, ADIF) of the file
        *
        * @param None
        * @returns Format type
        */
        OSCL_IMPORT_REF TAACFormat GetAACFormat(void);

        OSCL_IMPORT_REF  ParserErrorCode getAACHeaderLen(OSCL_wString& aClip,  bool aInitParsingEnable, Oscl_FileServer* iFileSession, PVMFCPMPluginAccessInterfaceFactory* aCPMAccess, OsclFileHandle*aHandle, uint32* HeaderLen);

        OSCL_IMPORT_REF  PVID3Version GetID3Version() const;

        /**
        * @brief Returns if file is AAC
        *
        * @param
        * @returns status
        */
        OSCL_IMPORT_REF ParserErrorCode IsAACFile(OSCL_wString& aClip, Oscl_FileServer* aFileSession, PVMFCPMPluginAccessInterfaceFactory* aCPMAccess, OsclFileHandle* aHandle = NULL);

    private:
        PVFile     iAACFile;

        int32       iAACDuration;
        int32       iAACSampleFrequency;

        int32       iAACBitRate;
        int32       iAACHeaderLen;
        bool        iFirstTime;

        int32       iAACFileSize;
        int32       iTotalNumFramesRead;
        Oscl_Vector<int32, alloc_type> iRPTable;

        TAACFormat  iAACFormat;
        bool        iEndOfFileReached;

        // Decoder input buffer for 1 raw encoded speech frame
        uint8 iAACFrameBuffer[PACKET_INDICATOR_LENGTH + 12];

        //uint8 iAACHeaderReferenceBuffer[PACKET_INDICATOR_LENGTH];
        PVLogger*     iLogger;
        PVLogger*     iDiagnosticLogger;

        AACBitstreamObject *ipBSO;


};

#endif //AACFILEPARSER_H_INCLUDED
