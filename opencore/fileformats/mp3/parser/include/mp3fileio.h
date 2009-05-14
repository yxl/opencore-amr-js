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
 *  @file mp3fileio.h
 *  @brief This include file contains the definitions for the MP3 file
 *  utility functions.
 */

#ifndef MP3FILEIO_H_INCLUDED
#define MP3FILEIO_H_INCLUDED


//----------------------------------------------------------------------
// Include Files
//----------------------------------------------------------------------
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef PVFILE_H_INCLUDED
#include "pvfile.h"
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

/**
 *  @brief The MP3FileIO Class is the class that reads information
 *  from a MP3 file.
 */

class MP3FileIO
{
    public:
        /**
        * @brief Reads in 64 bits byte by byte, taking the most significant byte first.
        *
        * @param fp Pointer to file to read from
        * @param data Data read
        * @returns True if read is successful; False otherwise
        */
        static bool read64(PVFile *fp, uint64 &data);

        /**
        * @brief Reads in 32 bits byte by byte, taking the most significant byte first.
        *
        * @param fp Pointer to file to read from
        * @param data Data read
        * @returns True if read is successful; False otherwise
        */
        static bool read32(PVFile *fp, uint32 &data);

        /**
        * @brief Reads in 32 bits byte by byte, taking the most significant byte first.
        * This is equivalent to two read32 calls.
        *
        * @param fp Pointer to file to read from
        * @param data1 First data read
        * @param data2 Second data read
        * @returns True if read is successful; False otherwise
        */
        static bool read32read32(PVFile *fp, uint32 &data1, uint32 &data2);

        /**
        * @brief Read in 24 bits byte by byte, taking the most significant byte first.
        *
        * @param fp Pointer to file to read from
        * @param data Data read
        * @returns True if read is successful; False otherwise
        */
        static bool read24(PVFile *fp, uint32 &data);

        /**
        * @brief Read in 16 bits byte by byte, taking the most significant byte first.
        *
        * @param fp Pointer to file to read from
        * @param data Data read
        * @returns True if read is successful; False otherwise
        */
        static bool read16(PVFile *fp, uint16 &data);

        /**
        * @brief Read in 16 bits byte by byte, taking the most significant byte first.
        * This is equivalent to two read16 calls.
        *
        * @param fp Pointer to file to read from
        * @param data1 First data read
        * @param data2 Second data read
        * @returns True if read is successful; False otherwise
        */
        static bool read16read16(PVFile *fp, uint16 &data1, uint16 &data2);

        /**
        * @brief Read in 8 bits of data.
        *
        * @param fp Pointer to file to read from
        * @param data Data read
        * @returns True if read is successful; False otherwise
        */
        static bool read8(PVFile *fp, uint8 &data);

        /**
        * @brief Read in 8 bits of data. This is equivalent to two read8 calls.
        *
        * @param fp Pointer to file to read from
        * @param data1 First data read
        * @param data2 Second data read
        * @returns True if read is successful; False otherwise
        */
        static bool read8read8(PVFile *fp, uint8 &data1, uint8 &data2);

        /**
        * @brief Reads in byte data, taking the most significant byte first.
        *
        * @param fp Pointer to file to read from
        * @param length Number of bytes to read
        * @param data Data read
        * @returns True if read is successful; False otherwise
        */
        static bool readByteData(PVFile *fp, uint32 length, uint8 *data, uint32* numbytes = NULL);

        /**
        * @brief Peeks and returns the next Nth tag (32 bits) from the file.
        *
        * @param fp Pointer to file to read from
        * @param n Byte position to peek
        * @returns None
        */
        static uint32 peekNextNthBytes(PVFile *fp, int32 n);

        /**
        * @brief Peeks and returns the next Nth bytes (8 bits) from the file.
        *
        * @param fp Pointer to file to read from
        * @returns None
        */
        static uint8  peekNextByte(PVFile *fp);

        /**
        * @brief Returns content length if known
        * @param fp Pointer to fill to read from
        * @return Content length (0 if unknown)
        */
        static uint32 getContentLength(PVFile *fp);
};

#endif // #ifdef MP3FILEIO_H_INCLUDED
