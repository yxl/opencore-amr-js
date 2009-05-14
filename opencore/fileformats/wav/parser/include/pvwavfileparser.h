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
#ifndef PVWAVFILEPARSER_H_INCLUDED
#define PVWAVFILEPARSER_H_INCLUDED

//----------------------------------------------------------------------------
// INCLUDES
//----------------------------------------------------------------------------

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
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


//////////////////////////////////WAV File parser

// This enumeration lists the formats currently supported
enum PVWavAudioFormats
{
    PVWAV_UNKNOWN_AUDIO_FORMAT = 0,
    PVWAV_PCM_AUDIO_FORMAT = 1,
    PVWAV_ITU_G711_ALAW = 6,
    PVWAV_ITU_G711_ULAW = 7
};


typedef struct
{
    int32  AudioFormat;
    uint16 NumChannels;
    uint16 BlockAlign;
    uint16 BitsPerSample;
    uint16 BytesPerSample;  // takes into account byte alignment
    uint32 SampleRate;
    uint32 ByteRate;
    uint32 NumSamples;	// Total number of Samples
    bool isLittleEndian;	// 1 if data is little endian and 0 if it is big endian
} PVWAVFileInfo;

enum PVWavParserReturnCode
{
    // Return codes
    PVWAVPARSER_OK = 0,
    PVWAVPARSER_READ_ERROR = -1,
    PVWAVPARSER_MISC_ERROR = -2,
    PVWAVPARSER_UNSUPPORTED_FORMAT = -3,
    PVWAVPARSER_END_OF_FILE = -4
};

class PV_Wav_Parser
{
    public:
        OSCL_IMPORT_REF PV_Wav_Parser()
        {
            AudioFormat = 0;
            NumChannels = 0;
            SampleRate = 0;
            ByteRate = 0;
            BlockAlign = 0;
            BitsPerSample = 0;
            BytesPerSample = 0;
            xLawTable = NULL;
            PCMBytesRead = 0;
            PCMBytesPresent = 0;
            ipWAVFile = NULL;
            iHeaderSize = 0;
            isLittleEndian = 1; // little endian by default
            iEndOfDataSubChunkOffset = 0;
        }

        OSCL_IMPORT_REF ~PV_Wav_Parser();
        //{
        //  CleanupWAVFile();
        //}

        // Will parse WAVE File
        OSCL_IMPORT_REF PVWavParserReturnCode InitWavParser(OSCL_wString& aClip, Oscl_FileServer* aFileSession);

        // Returns number of samples copied to the buffer
        OSCL_IMPORT_REF PVWavParserReturnCode GetPCMData(uint8* inBuff, uint32 inBufSize, uint32 NumberOfSamples, uint32& NumSamplesRead);

        // Copies Number of Channels, Sampling rate and Bits per sample to input parameters
        OSCL_IMPORT_REF bool RetrieveFileInfo(PVWAVFileInfo& aInfo);

        //Seek in PCM Data (Seeks the file position to SampleNumber specified so that user can then get PCM data starting from that Sample)
        OSCL_IMPORT_REF PVWavParserReturnCode SeekPCMSample(uint32 SampleNumber);

        //Set the paser output to uncompressed PCM.
        OSCL_IMPORT_REF bool SetOutputToUncompressedPCM(void);
    private:
        PVWavParserReturnCode ReadData(uint8* buff, uint32 size, uint32& bytesread);
        OSCL_IMPORT_REF void CleanupWAVFile(void);

        //Subchunk 1
        uint16 AudioFormat;
        uint16 NumChannels;
        uint32 SampleRate;
        uint32 ByteRate;
        uint16 BlockAlign;
        uint16 BitsPerSample;
        uint16 BytesPerSample;
        bool isLittleEndian;	// 1 if data is little endian and 0 if it is big endian

        short *xLawTable;

        // It will be incremented when PCM data is read and compared to Subchunk2_size to determine whether whole file has been read or not
        uint32 PCMBytesRead;
        uint32 PCMBytesPresent;// PCM data size in bytes
        uint32 NumSamples;
        uint32 iEndOfDataSubChunkOffset;

        // Pointer to input file (File to be opened and file pointer kept with the class. It will be closed when no data is left in file descriptor of class)
        Oscl_File* ipWAVFile;

        // header size (size of data before data subchunk)
        uint32 iHeaderSize;
};


#endif // PVWAVFILEPARSER_H_INCLUDED
