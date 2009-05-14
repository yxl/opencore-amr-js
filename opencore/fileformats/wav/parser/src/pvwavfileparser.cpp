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
#include "pvwavfileparser.h"
#include "oscl_dll.h"
OSCL_DLL_ENTRY_POINT_DEFAULT()

const short MuLawDecompressTable[256] =
{
    -32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956,
    -23932, -22908, -21884, -20860, -19836, -18812, -17788, -16764,
    -15996, -15484, -14972, -14460, -13948, -13436, -12924, -12412,
    -11900, -11388, -10876, -10364, -9852, -9340, -8828, -8316,
    -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
    -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
    -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
    -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
    -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
    -1372, -1308, -1244, -1180, -1116, -1052,  -988,  -924,
    -876,  -844,  -812,  -780,  -748,  -716,  -684,  -652,
    -620,  -588,  -556,  -524,  -492,  -460,  -428,  -396,
    -372,  -356,  -340,  -324,  -308,  -292,  -276,  -260,
    -244,  -228,  -212,  -196,  -180,  -164,  -148,  -132,
    -120,  -112,  -104,   -96,   -88,   -80,   -72,   -64,
    -56,   -48,   -40,   -32,   -24,   -16,    -8,     0,
    32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
    23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
    15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
    11900, 11388, 10876, 10364,  9852,  9340,  8828,  8316,
    7932,  7676,  7420,  7164,  6908,  6652,  6396,  6140,
    5884,  5628,  5372,  5116,  4860,  4604,  4348,  4092,
    3900,  3772,  3644,  3516,  3388,  3260,  3132,  3004,
    2876,  2748,  2620,  2492,  2364,  2236,  2108,  1980,
    1884,  1820,  1756,  1692,  1628,  1564,  1500,  1436,
    1372,  1308,  1244,  1180,  1116,  1052,   988,   924,
    876,   844,   812,   780,   748,   716,   684,   652,
    620,   588,   556,   524,   492,   460,   428,   396,
    372,   356,   340,   324,   308,   292,   276,   260,
    244,   228,   212,   196,   180,   164,   148,   132,
    120,   112,   104,    96,    88,    80,    72,    64,
    56,    48,    40,    32,    24,    16,     8,     0
};

const short ALawDecompressTable[256] =
{
    -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736,
    -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784,
    -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368,
    -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392,
    -22016, -20992, -24064, -23040, -17920, -16896, -19968, -18944,
    -30208, -29184, -32256, -31232, -26112, -25088, -28160, -27136,
    -11008, -10496, -12032, -11520, -8960, -8448, -9984, -9472,
    -15104, -14592, -16128, -15616, -13056, -12544, -14080, -13568,
    -344,  -328,  -376,  -360,  -280,  -264,  -312,  -296,
    -472,  -456,  -504,  -488,  -408,  -392,  -440,  -424,
    -88,   -72,   -120,  -104,  -24,   -8,    -56,   -40,
    -216,  -200,  -248,  -232,  -152,  -136,  -184,  -168,
    -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184,
    -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696,
    -688,  -656,  -752,  -720,  -560,  -528,  -624,  -592,
    -944,  -912,  -1008, -976,  -816,  -784,  -880,  -848,
    5504,  5248,  6016,  5760,  4480,  4224,  4992,  4736,
    7552,  7296,  8064,  7808,  6528,  6272,  7040,  6784,
    2752,  2624,  3008,  2880,  2240,  2112,  2496,  2368,
    3776,  3648,  4032,  3904,  3264,  3136,  3520,  3392,
    22016, 20992, 24064, 23040, 17920, 16896, 19968, 18944,
    30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136,
    11008, 10496, 12032, 11520, 8960,  8448,  9984,  9472,
    15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568,
    344,   328,   376,   360,   280,   264,   312,   296,
    472,   456,   504,   488,   408,   392,   440,   424,
    88,    72,   120,   104,    24,     8,    56,    40,
    216,   200,   248,   232,   152,   136,   184,   168,
    1376,  1312,  1504,  1440,  1120,  1056,  1248,  1184,
    1888,  1824,  2016,  1952,  1632,  1568,  1760,  1696,
    688,   656,   752,   720,   560,   528,   624,   592,
    944,   912,  1008,   976,   816,   784,   880,   848
};



OSCL_EXPORT_REF PV_Wav_Parser::~PV_Wav_Parser()
{
    CleanupWAVFile();
}

/* ======================================================================== */
/*	Function : ReadData()                                               */
/*	Author   :                                                          */
/*	Date     :                                                          */
/*	Purpose  : Read data from file in a buffer                          */
/*	In/out   :                                                          */
/*	Return   : Result of read operation                                 */
/*	Note     :                                                          */
/*	Modified :                                                          */
/* ======================================================================== */
PVWavParserReturnCode PV_Wav_Parser::ReadData(uint8* buff, uint32 size, uint32& bytesread)
{
    OSCL_ASSERT(ipWAVFile != NULL);

    // read data
    if ((bytesread = ipWAVFile->Read(buff, 1, size)) == 0)
    {
        if (ipWAVFile->EndOfFile())
            return PVWAVPARSER_END_OF_FILE;
        else
            return PVWAVPARSER_READ_ERROR;
    }
    return PVWAVPARSER_OK;
}

/* ======================================================================== */
/*  Function : CleanupWAVFile()                                             */
/*  Author   :                                                              */
/*  Date     :                                                              */
/*  Purpose  : Close the file and reset any associated variables            */
/*  In/out   :                                                              */
/*  Return   : Result of read operation                                     */
/*  Note     :                                                              */
/*  Modified :                                                              */
/* ======================================================================== */
OSCL_EXPORT_REF void PV_Wav_Parser::CleanupWAVFile(void)
{
    if (ipWAVFile != NULL)
        ipWAVFile->Close();
    OSCL_DELETE(ipWAVFile);
    ipWAVFile = NULL;

    AudioFormat = 0;
    NumChannels = 0;
    SampleRate = 0;
    ByteRate = 0;
    BlockAlign = 0;
    BitsPerSample = 0;
    BytesPerSample = 0;
    PCMBytesRead = 0;
    PCMBytesPresent = 0;
    iHeaderSize = 0;
    isLittleEndian = 1; // little endian by default
    xLawTable = NULL;
}


/* ======================================================================== */
/*	Function : InitWavParser()                                          */
/*	Author   :                                                          */
/*	Date     :                                                          */
/*	Purpose  : Initialize WAVE Parser                                   */
/*	In/out   :                                                          */
/*	Return   : Result of reading the WAV file                           */
/*	Note     :                                                          */
/*	Modified :                                                          */
/* ======================================================================== */
OSCL_EXPORT_REF PVWavParserReturnCode PV_Wav_Parser::InitWavParser(OSCL_wString& aClip, Oscl_FileServer* aFileSession)
{
    //buffer
    uint8 iBuffer[36];

    // If a WAV file is already open, close it first and delete the file pointer
    CleanupWAVFile();

    // Open the file (aClip)
    ipWAVFile = OSCL_NEW(Oscl_File, (4096));
    if (ipWAVFile == NULL)
    {
        return PVWAVPARSER_READ_ERROR;
    }

    if (ipWAVFile->Open(aClip.get_cstr(), (Oscl_File::MODE_READ | Oscl_File::MODE_BINARY), *aFileSession) != 0)
    {
        OSCL_DELETE(ipWAVFile);
        ipWAVFile = NULL;
        return PVWAVPARSER_READ_ERROR;
    }

    // Get file size at the very first time
    int32 filesize = 0;
    if (ipWAVFile->Seek(0, Oscl_File::SEEKEND))
    {
        CleanupWAVFile();
        return PVWAVPARSER_MISC_ERROR;
    }

    filesize = (TOsclFileOffsetInt32)ipWAVFile->Tell();

    if (filesize <= 0)
    {
        CleanupWAVFile();
        return PVWAVPARSER_MISC_ERROR;
    }

    if (ipWAVFile->Seek(0, Oscl_File::SEEKSET))
    {
        CleanupWAVFile();
        return PVWAVPARSER_MISC_ERROR;
    }
    int32 filepos = 0;

    // read 12 bytes of data for complete WAVE header including RIFF chunk dDescriptor

    uint32 bytesread = 0;
    if (ReadData(iBuffer, 12, bytesread) != PVWAVPARSER_OK)
    {
        CleanupWAVFile();
        return PVWAVPARSER_READ_ERROR;
    }
    if (bytesread != 12)
    {
        CleanupWAVFile();
        return PVWAVPARSER_READ_ERROR;
    }
    // Update file position counter by 36 bytes
    filepos += 12;

    // Check for RIFF/RIFX
    uint8* pBuffer = &iBuffer[0];
    if (pBuffer[0] == 'R' &&
            pBuffer[1] == 'I' &&
            pBuffer[2] == 'F' &&
            pBuffer[3] == 'F')
    {
        isLittleEndian = 1; // Little endian data
    }
    else if (pBuffer[0] == 'R' &&
             pBuffer[1] == 'I' &&
             pBuffer[2] == 'F' &&
             pBuffer[3] == 'X')
    {
        isLittleEndian = 0; // Big endian data
    }
    else
    {
        CleanupWAVFile();
        return PVWAVPARSER_UNSUPPORTED_FORMAT;
    }

    // If a .wav file is clipped manually , the ChunkSize would not relate to the file size. However, we
    // should still attempt to play that clip for the existing length of the file. To enforce the restriction,
    // we may opt to uncomment the following lines of code.

    // To read ChunkSize (in RIFF chunk descriptor) from little endian
    /*
    uint32 ChunkSize;

    ChunkSize = (((*(pBuffer + 7)) <<24)|((*(pBuffer + 6)) << 16)|((*(pBuffer + 5)) << 8)|(*(pBuffer + 4)));
    if((int32)ChunkSize!= (filesize - 8))
    {
        CleanupWAVFile();
        return PVWAVPARSER_MISC_ERROR;
    }
    */

    // Check for WAVE in Format field
    if (pBuffer[ 8] != 'W' ||
            pBuffer[ 9] != 'A' ||
            pBuffer[10] != 'V' ||
            pBuffer[11] != 'E')
    {
        CleanupWAVFile();
        return PVWAVPARSER_UNSUPPORTED_FORMAT;
    }

    uint32 SubChunk_Size = 0;

    bool fmtSubchunkFound = false;
    while (!fmtSubchunkFound)
    {
        // read 8 bytes from file to check for next subchunk
        bytesread = 0;
        if (ReadData(iBuffer, 8, bytesread) != PVWAVPARSER_OK)
        {
            CleanupWAVFile();
            return PVWAVPARSER_READ_ERROR;
        }
        if (bytesread != 8)
        {
            CleanupWAVFile();
            return PVWAVPARSER_READ_ERROR;
        }
        // Update file position counter by 8 bytes
        filepos += 8;
        uint8* pTempBuffer = &iBuffer[0];
        SubChunk_Size = (((*(pTempBuffer + 7)) << 24) | ((*(pTempBuffer + 6)) << 16) | ((*(pTempBuffer + 5)) << 8) | (*(pTempBuffer + 4)));

        // typecast filesize as uint32 - to get to this point, it MUST be
        // greater than 0.
        if ((filepos + SubChunk_Size) > (uint32)filesize)
        {
            CleanupWAVFile();
            return PVWAVPARSER_MISC_ERROR;
        }
        // Check for FMT subchunk
        if (pTempBuffer[0] != 'f' ||
                pTempBuffer[1] != 'm' ||
                pTempBuffer[2] != 't' ||
                pTempBuffer[3] != ' ')
        {
            // "fmt " chunk not found - Unknown subchunk
            filepos += SubChunk_Size;
            if (ipWAVFile->Seek(filepos, Oscl_File::SEEKSET))
            {
                CleanupWAVFile();
                return PVWAVPARSER_MISC_ERROR;
            }
        }
        else
            fmtSubchunkFound = true;
    }
    if (ReadData(iBuffer, 16, bytesread) != PVWAVPARSER_OK)
    {
        CleanupWAVFile();
        return PVWAVPARSER_READ_ERROR;
    }
    if (bytesread != 16)
    {
        CleanupWAVFile();
        return PVWAVPARSER_READ_ERROR;
    }
    filepos += 16;

    pBuffer = &iBuffer[0];

    AudioFormat = (unsigned short)(((*(pBuffer + 1)) << 8) | (*pBuffer));	// Save AudioFormat (PCM = 1)
    xLawTable = NULL;

    NumChannels = (unsigned short)(((*(pBuffer + 3)) << 8) | (*(pBuffer + 2)));	// Save Number of Channels
    SampleRate = (((*(pBuffer + 7)) << 24) | ((*(pBuffer + 6)) << 16) | ((*(pBuffer + 5)) << 8) | (*(pBuffer + 4)));	//	Save Sampling rate
    ByteRate = (((*(pBuffer + 11)) << 24) | ((*(pBuffer + 10)) << 16) | ((*(pBuffer + 9)) << 8) | (*(pBuffer + 8)));	//	Save ByteRate ( == SampleRate*NumChannels*BitsPerSample/8)
    BlockAlign = (unsigned short)(((*(pBuffer + 13)) << 8) | (*(pBuffer + 12)));	//	Save BlockAlign	( == NumChannels*BitsPerSample/8)
    BitsPerSample = (unsigned short)(((*(pBuffer + 15)) << 8) | (*(pBuffer + 14))); //	Save BitsPerSample	(8 bits == 8, 16 bits == 16 etc.)
    BytesPerSample = (BitsPerSample + 7) / 8;  // compute (ceil(BitsPerSample/8))

    // Check for SubChunk_Size (It should be 16 for PCM)
    // skip remaining bytes if any because we don't currently support any compressed audio formats
    if (SubChunk_Size != 16)
    {
        // seek ahead by Subchunk1_Size - 16
        filepos += SubChunk_Size - 16;
        if (ipWAVFile->Seek(filepos, Oscl_File::SEEKSET))
        {
            CleanupWAVFile();
            return PVWAVPARSER_MISC_ERROR;
        }
    }

    bool DataSubchunkFound = false;

    while (!DataSubchunkFound)
    {
        // read 8 bytes from file to check for next subchunk
        bytesread = 0;
        if (ReadData(iBuffer, 8, bytesread) != PVWAVPARSER_OK)
        {
            CleanupWAVFile();
            return PVWAVPARSER_READ_ERROR;
        }
        if (bytesread != 8)
        {
            CleanupWAVFile();
            return PVWAVPARSER_READ_ERROR;
        }
        // Update file position counter by 8 bytes
        filepos += 8;

        uint8* pTempBuffer = &iBuffer[0];

        //It means that some unknown subchunk is present
        // Calculate  SubChunk Size
        SubChunk_Size = (((*(pTempBuffer + 7)) << 24) | ((*(pTempBuffer + 6)) << 16) | ((*(pTempBuffer + 5)) << 8) | (*(pTempBuffer + 4)));

        // Check for DATA subchunk ID
        if (pTempBuffer[0] != 'd' ||
                pTempBuffer[1] != 'a' ||
                pTempBuffer[2] != 't' ||
                pTempBuffer[3] != 'a')
        {
            // we need to skip this many bytes
            filepos += SubChunk_Size;

            // seek file pointer to current file position
            if (ipWAVFile->Seek(filepos, Oscl_File::SEEKSET))
            {
                CleanupWAVFile();
                return PVWAVPARSER_MISC_ERROR;
            }
        }
        else
        {
            // data subchunk found

            // header size equals current file pos
            iHeaderSize = filepos;

            //data subchunk is found
            DataSubchunkFound = true;

            // Read data SubChunk Size (is number of bytes in data or PCMBytesPresent)
            PCMBytesPresent = SubChunk_Size;
            iEndOfDataSubChunkOffset = filepos + PCMBytesPresent; //This is where the DataSubChunk is supposed to end

            //(this check is required to avoid memory crash if any of BytesPerSample or NumChannels is '0')
            if (BytesPerSample && NumChannels)
            {
                NumSamples = ((PCMBytesPresent / (BytesPerSample)) / NumChannels);
            }
        }
    }

    //return error if any of these value is not given in wav file header
    //(AudioFormat check is done at node level)
    if (!NumChannels || !NumSamples || !SampleRate || !BitsPerSample || !BytesPerSample || !ByteRate)
    {
        CleanupWAVFile();
        return PVWAVPARSER_UNSUPPORTED_FORMAT;	//any error fom parse will be handled as PVMFFailure at corresponsding node level
    }

    return PVWAVPARSER_OK;
}


/* ======================================================================== */
/*	Function : GetPCMData()                                             */
/*	Author   :                                                          */
/*	Date     :                                                          */
/*	Purpose  : Copy specified number of samples to output buffer        */
/*	In/out   :                                                          */
/*	Return   : Result of read operation                                 */
/*	Note     :                                                          */
/*	Modified :                                                          */
/* ======================================================================== */
OSCL_EXPORT_REF PVWavParserReturnCode PV_Wav_Parser::GetPCMData(uint8* inBuff, uint32 inBufSize, uint32 NumberOfSamples, uint32& NumSamplesRead)
{
    NumSamplesRead = 0;
    uint32 BytesRead = 0;

    uint32 myBufSize = NumberOfSamples * BytesPerSample * NumChannels;
    if ((ipWAVFile->Tell() + myBufSize) > iEndOfDataSubChunkOffset)
    {//we don't have enough data to fulfill this request
        int32 pos = (TOsclFileOffsetInt32)ipWAVFile->Tell();
        if (((uint32)(BytesPerSample*NumChannels) + pos) > iEndOfDataSubChunkOffset)
        {
            return PVWAVPARSER_END_OF_FILE;
        }
        NumberOfSamples = (iEndOfDataSubChunkOffset - pos) / (BytesPerSample * NumChannels);
        myBufSize = NumberOfSamples * BytesPerSample * NumChannels;
    }


    if (xLawTable)
    {
        //PCM16 2 bytes per sample
        if (myBufSize*2 > inBufSize)
        {
            //buffer too small
            return PVWAVPARSER_MISC_ERROR ;
        }
        if ((uint32)inBuff & 0x3)
        {
            //buffer not word aligned
            return PVWAVPARSER_MISC_ERROR ;
        }

        PVWavParserReturnCode retcode = ReadData(inBuff, myBufSize, BytesRead);
        if (retcode == PVWAVPARSER_OK)
        {
            uint8 *mySrcBuf = &inBuff[BytesRead-1];
            short *myDstBuf = &(((short*) inBuff)[BytesRead-1]);
            for (int i = BytesRead; i > 0; i--)
                *myDstBuf-- = xLawTable[*mySrcBuf--];

            // Calculate the number of samples read based on the bytes read from file
            NumSamplesRead = (BytesRead / BytesPerSample) / NumChannels;
        }
        return retcode;
    }

    if (myBufSize > inBufSize)
    {
        //buffer too small
        return PVWAVPARSER_MISC_ERROR ;
    }
    PVWavParserReturnCode retcode = ReadData(inBuff, myBufSize, BytesRead);
    if (retcode == PVWAVPARSER_OK)
    {
        // Calculate the number of samples read based on the bytes read from file
        NumSamplesRead = (BytesRead / BytesPerSample) / NumChannels;
    }
    return retcode;
}


/* ======================================================================== */
/*	Function : RetrieveFileInfo()                                       */
/*	Author   :                                                          */
/*	Date     :                                                          */
/*	Purpose  :                                                          */
/*	In/out   :                                                          */
/*	Return   : true / false                                             */
/*	Note     :                                                          */
/*	Modified :                                                          */
/* ======================================================================== */
OSCL_EXPORT_REF bool PV_Wav_Parser::RetrieveFileInfo(PVWAVFileInfo& aInfo)
{
    // If the file pointer is valid, then WAV file has been parsed
    if (ipWAVFile)
    {
        aInfo.AudioFormat = AudioFormat;
        aInfo.NumChannels = NumChannels;
        aInfo.SampleRate = SampleRate;
        aInfo.ByteRate = ByteRate;
        aInfo.BlockAlign = BlockAlign;
        aInfo.BitsPerSample = BitsPerSample;
        aInfo.BytesPerSample = BytesPerSample;
        aInfo.NumSamples = NumSamples;
        aInfo.isLittleEndian = isLittleEndian;

        return true;
    }
    else
    {
        return false;
    }
}

OSCL_EXPORT_REF bool PV_Wav_Parser::SetOutputToUncompressedPCM(void)
{
    xLawTable = NULL;
    if (PVWAV_ITU_G711_ULAW == AudioFormat)
    {
        xLawTable = OSCL_CONST_CAST(short*, MuLawDecompressTable);
        return true;
    }
    else if (PVWAV_ITU_G711_ALAW == AudioFormat)
    {
        xLawTable = OSCL_CONST_CAST(short*, ALawDecompressTable);
        return true;
    }
    else if (AudioFormat == PVWAV_PCM_AUDIO_FORMAT)
    {
        if ((8 == BitsPerSample) || isLittleEndian)
            return true;
    }
    return false;
}

/* ======================================================================== */
/*	Function : SeekPCMSample()                                          */
/*	Author   :                                                          */
/*	Date     :                                                          */
/*	Purpose  :                                                          */
/*	In/out   :                                                          */
/*	Return   : Result of seek operation                                 */
/*	Note     :                                                          */
/*	Modified :                                                          */
/* ======================================================================== */
OSCL_EXPORT_REF PVWavParserReturnCode PV_Wav_Parser::SeekPCMSample(uint32 SampleNumber)
{
    if (ipWAVFile == NULL)
    {
        return PVWAVPARSER_MISC_ERROR;
    }

    if (SampleNumber >= NumSamples)
    {
        // seek to the end of the file and return;
        ipWAVFile->Seek(0, Oscl_File::SEEKEND);
        return PVWAVPARSER_OK;
    }

    // valid header size must be present(it is initialized only if valid data subchunk is present)
    if (iHeaderSize)
    {
        // Seek to file position corresponding to headersize+(sample# * numchannels * bytespersample)
        if (ipWAVFile->Seek((iHeaderSize + (SampleNumber*NumChannels*(BytesPerSample))), Oscl_File::SEEKSET))
        {
            return PVWAVPARSER_MISC_ERROR;
        }
    }
    else
    {
        return PVWAVPARSER_MISC_ERROR;
    }

    return PVWAVPARSER_OK;
}









