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

//                           M P 3   F I L E   I O

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


/**
 *  @file mp3fileio.cpp
 *  @brief This include file contains the implementation of the MP3 file
 *  IO utility functions.
 */

//----------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------
#define IMPLEMENT_MP3FileIO

//----------------------------------------------------------------------
// Include Files
//----------------------------------------------------------------------
#include "mp3fileio.h"
#include "oscl_utf8conv.h"


//----------------------------------------------------------------------
// Type Declarations
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Global Constant Definitions
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Global Data Definitions
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Static Variable Definitions
//----------------------------------------------------------------------

/*======================================================================
FUNCTION:     **** FILE POINTER READING METHODS

DESCRIPTION:

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:

======================================================================*/

// Read in the 64 bits byte by byte and take most significant byte first
bool MP3FileIO::read64(PVFile *fp, uint64 &data)
{
    const int32 N = 8;
    uint8 bytes[N];
    data = 0;

    int32 retVal = (int32)(fp->Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    for (int32 i = 0; i < N; i++)
    {
        // data = (data<<8) | bytes[i];
    }

    return true;
}

// Read in the 32 bits byte by byte and take most significant byte first
bool MP3FileIO::read32(PVFile *fp, uint32 &data)
{
    const int32 N = 4;
    uint8 bytes[N];
    data = 0;

    int32 retVal = (int32)(fp->Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    for (int32 i = 0; i < N; i++)
        data = (data << 8) | bytes[i];

    return true;
}

// Read in the 32 bits byte by byte and take most significant byte first.
// This is equivalent to two read32 calls.
bool MP3FileIO::read32read32(PVFile *fp, uint32 &data1, uint32 &data2)
{
    const int32 N = 8;
    uint8 bytes[N];
    data1 = 0;
    data2 = 0;

    int32 retVal = (int32)(fp->Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    int32 i;
    for (i = 0; i < 4; i++)
        data1 = (data1 << 8) | bytes[i];

    for (i = 4; i < 8; i++)
        data2 = (data2 << 8) | bytes[i];

    return true;
}

// Read in the 24 bits byte by byte and take most significant byte first
bool MP3FileIO::read24(PVFile *fp, uint32 &data)
{
    const int32 N = 3;
    uint8 bytes[N];
    data = 0;

    int32 retVal = (int32)(fp->Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    for (int32 i = 0; i < N; i++)
        data = (data << 8) | bytes[i];

    return true;
}

// Read in the 16 bits byte by byte and take most significant byte first
bool MP3FileIO::read16(PVFile *fp, uint16 &data)
{
    const int32 N = 2;
    uint8 bytes[N];
    data = 0;

    int32 retVal = (int32)(fp->Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    for (int32 i = 0; i < N; i++)
        data = (uint16)((data << 8) | (uint16)bytes[i]);

    return true;
}

// Read in the 16 bits byte by byte and take most significant byte first
// This is equivalent to two read16 calls
bool MP3FileIO::read16read16(PVFile *fp, uint16 &data1, uint16 &data2)
{
    const int32 N = 4;
    uint8 bytes[N];
    data1 = 0;
    data2 = 0;

    int32 retVal = (int32)(fp->Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    int32 i;
    for (i = 0; i < 2; i++)
        data1 = (uint16)((data1 << 8) | (uint16) bytes[i]);

    for (i = 2; i < 4; i++)
        data2 = (uint16)((data2 << 8) | (uint16) bytes[i]);

    return true;
}

// Read in the 8 bit byte
bool MP3FileIO::read8(PVFile *fp, uint8 &data)
{
    data = 0;

    int32 retVal = (int32)(fp->Read((void*) & data, 1, 1));

    if (retVal < 1)
        return false;

    /*
    int8 byte;
    is.get(byte);
    data = (uint8)byte;
    */
    return true;
}

// Read in the 8 bit byte
// This is equivalent to two read8 calls
bool MP3FileIO::read8read8(PVFile *fp, uint8 &data1, uint8 &data2)
{
    const int32 N = 2;
    uint8 bytes[N];
    data1 = 0;
    data2 = 0;

    int32 retVal = (int32)(fp->Read((void*)bytes, 1, N));

    if (retVal < N)
        return false;

    data1 = bytes[0];
    data2 = bytes[1];

    return true;
}

// Read in byte data and take most significant byte first
bool MP3FileIO::readByteData(PVFile *fp, uint32 length, uint8 *data, uint32 *numbytes)
{
    uint32 bytesRead;
    bytesRead = fp->Read(data, 1, length);

    if (numbytes)
        *numbytes = bytesRead;

    if (bytesRead < (uint32)length) // read byte data failed
    {
        int32 seekback = 0;
        // bytes to seek backwards
        seekback -= (int32) bytesRead;
        fp->Seek(seekback, Oscl_File::SEEKCUR);
        return false;
    }

    return true;
}

// Peeks and returns the next Nth tag (32 bits) from the file
uint32 MP3FileIO::peekNextNthBytes(PVFile *fp, int32 n)
{
    uint32 tag = 0;
    for (int32 i = 0; i < n; i++)
    {
        MP3FileIO::read32(fp, tag);
    }
    fp->Seek(-4*n, Oscl_File::SEEKCUR);
    return tag;
}

// Peeks and returns the next Nth bytes (8 bits) from the file
uint8 MP3FileIO::peekNextByte(PVFile *fp)
{
    uint8 tag = 0;
    MP3FileIO::read8(fp, tag);

    fp->Seek(-1, Oscl_File::SEEKCUR);

    return tag;
}

// Returns the content length if known
// return 0 if unknown
uint32 MP3FileIO::getContentLength(PVFile *fp)
{
    return fp->GetContentLength();
}
