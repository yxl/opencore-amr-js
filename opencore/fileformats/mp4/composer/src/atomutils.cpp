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
 * This PVA_FF_AtomUtils Class contains sime useful methods for operating on Atoms
 */


#define IMPLEMENT_AtomUtils

#include "atomutils.h"
#include "a_atomdefs.h"
#include <time.h>

// **** FILE STREAM RENDERING METHODS ****

// Render the 64 bits byte by byte and take most significant byte first
bool
PVA_FF_AtomUtils::render64(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint64 data)
{
    if (fp != NULL)
    {
        for (int32 i = 0; i < 64; i += 8)
        {

            uint8 byte = (uint8)((data >> (56 - i)) & 0x000000ff);
            if (fp->_filePtr->Write(&byte, 1, 1) != 1)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

// Render the 32 bits byte by byte and take most significant byte first
bool
PVA_FF_AtomUtils::render32(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 data)
{
    if (fp != NULL)
    {
        for (int32 i = 0; i < 32; i += 8)
        {
            uint8 byte = (uint8)((data >> (24 - i)) & 0x000000ff);
            if (fp->_filePtr->Write(&byte, 1, 1) != 1)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

// Render the 24 bits byte by byte and take most significant byte first
bool
PVA_FF_AtomUtils::render24(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 data)
{
    if (fp != NULL)
    {
        for (int32 i = 0; i < 24; i += 8)
        {
            uint8 byte = (uint8)((data >> (16 - i)) & 0x0000ff);
            if (fp->_filePtr->Write(&byte, 1, 1) != 1)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

// Render the 16 bits byte by byte and take most significant byte first
bool
PVA_FF_AtomUtils::render16(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint16 data)
{
    if (fp != NULL)
    {
        for (int32 i = 0; i < 16; i += 8)
        {
            uint8 byte = (uint8)((data >> (8 - i)) & 0x00ff);
            if (fp->_filePtr->Write(&byte, 1, 1) != 1)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

// Render the 8 bits as a single byte
bool
PVA_FF_AtomUtils::render8(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint8 data)
{
    if (fp != NULL)
    {
        if (fp->_filePtr->Write(&data, 1, 1) != 1)
        {
            return false;
        }
        return true;
    }
    return false;
}

// Render the a string byte by byte
bool
PVA_FF_AtomUtils::renderString(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, PVA_FF_UTF8_STRING_PARAM data)
{
    if (fp != NULL)
    {
        uint8 byte = 0;
        for (uint32 i = 0; i < data.get_size(); i++)
        {
            byte = data[i];
            if (fp->_filePtr->Write(&byte, 1, 1) != 1)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

// Render the a string byte by byte
bool
PVA_FF_AtomUtils::renderUnicodeString(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, PVA_FF_UNICODE_STRING_PARAM data)
{
    if (fp != NULL)
    {
        OSCL_TCHAR *stringptr = (OSCL_TCHAR *)data.get_cstr();

        uint16 temp = 0;
        uint8 lower_byte = 0;
        uint8 upper_byte = 0;
        for (uint32 i = 0; i < data.get_size(); i++)
        {
            // Watch byte ordering in the stored format
            temp = (uint16)(stringptr[i] & 0xFF00);
            temp = (uint8)((temp >> 8) & 0xFF);
            lower_byte = (uint8)(temp);

            if (fp->_filePtr->Write(&lower_byte, 1, 1) != 1)
            {
                return false;
            }

            upper_byte = (uint8)(stringptr[i] & 0x00FF);
            if (fp->_filePtr->Write(&upper_byte, 1, 1) != 1)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

// Render the a string byte by byte PLUS render the trailing \0
bool
PVA_FF_AtomUtils::renderNullTerminatedString(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, PVA_FF_UTF8_STRING_PARAM data)
{
    if (fp != NULL)
    {
        PVA_FF_AtomUtils::renderString(fp, data);
        // put the trailing 0
        uint8 byte = 0;
        if (fp->_filePtr->Write(&byte, 1, 1) != 1)
        {
            return false;
        }
        return true;
    }
    return false;
}

// Render the a string byte by byte PLUS render the trailing \0
bool
PVA_FF_AtomUtils::renderNullTerminatedUnicodeString(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, PVA_FF_UNICODE_STRING_PARAM data)
{
    if (fp != NULL)
    {
        PVA_FF_AtomUtils::renderUnicodeString(fp, data);
        // Need two bytes of zeros for UNICODE null termination
        uint16 data = 0;
        if (fp->_filePtr->Write(&data, 2, 1) != 1)
        {
            return false;
        }
        return true;
    }
    return false;
}

// Render a chunk of uint8 data
bool
PVA_FF_AtomUtils::renderByteData(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 length, uint8 *data)
{
    bool retVal = true;

    if (fp != NULL)
    {
        if (fp->_filePtr->Write(data, 1, length) != length)
        {
            retVal = false;
        }
    }
    else
    {
        retVal = false;
    }

    return (retVal);
}

// Setting the current time value - seconds since 1/1/1904
void
PVA_FF_AtomUtils::setTime(uint32 &ulTime)
{
    // This fp not correct, it needs to adjust to base 1904 rather 1970.
    ulTime = uint32(time(NULL));
}


uint32
PVA_FF_AtomUtils::getNumberOfBytesUsedToStoreContent(uint32 sizeOfClass)
{
    // The content in a descriptor class fp stored immediately after the descriptor tag
    if (sizeOfClass <= 0x7f) return sizeOfClass - 2; // _sizeOfClass field fp 1 byte (7 LS bits)
    else if (sizeOfClass <= 0x3fff) return sizeOfClass - 3; // _sizeOfClass fp 2 bytes (7 LS bits each)
    else if (sizeOfClass <= 0x1fffff) return sizeOfClass - 4; // _sizeOfClass fp 3 bytes (7 LS bits each)
    else if (sizeOfClass <= 0x0fffffff) return sizeOfClass - 5; // _sizeOfClass fp 4 bytes (7 LS bits each)
    else return 0; // ERROR condition
}

uint32
PVA_FF_AtomUtils::getNumberOfBytesUsedToStoreSizeOfClass(uint32 contentSize)
{
    // The actual _sizeOfClass value includes the size of the class's contents PLUS
    // the number of bytes needed to store the _sizeOfClass field. The parameter
    // contentSize represents the number of bytes needed to store ONLY the members
    // of the class NOT including the _sizeOfClass field.
    if (contentSize <= 0x7e) return 1; // _sizeOfClass field can be rendered in 1 byte (7 LS bits)
    else if (contentSize <= 0x3ffd) return 2; // _sizeOfClass field can be rendered in 2 bytes (7 LS bits each)
    else if (contentSize <= 0x1ffffc) return 3; // _sizeOfClass field can be rendered in 3 bytes (7 LS bits each)
    else if (contentSize <= 0xfffffffb) return 4; // _sizeOfClass field can be rendered in 4 bytes (7 LS bits each)
    else return 0; // ERROR condition
}

void PVA_FF_AtomUtils::seekFromCurrPos(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 n)
{
    fp->_filePtr->Seek(n, Oscl_File::SEEKCUR);
}

void PVA_FF_AtomUtils::seekFromStart(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 n)
{
    fp->_filePtr->Seek(n, Oscl_File::SEEKSET);
}

void PVA_FF_AtomUtils::seekToEnd(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    fp->_filePtr->Seek(0, Oscl_File::SEEKEND);
}

void PVA_FF_AtomUtils::rewindFilePointerByN(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 n)
{
    fp->_filePtr->Seek((-1 * (int32) n), Oscl_File::SEEKCUR);
}

int32 PVA_FF_AtomUtils::getCurrentFilePosition(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    return (fp->_filePtr->Tell());
}

bool
PVA_FF_AtomUtils::openFile(MP4_AUTHOR_FF_FILE_IO_WRAP *fp,
                           PVA_FF_UNICODE_STRING_PARAM fileName,
                           int32 mode, uint32 aCacheSize)
{
    if (!fp->_filePtr)
    {
        // Only need to create new file object if it's not passed in from createMp4File
        //fp->_filePtr = OSCL_NEW(Oscl_File, ());
        PV_MP4_FF_NEW(fp->auditCB, Oscl_File, (), fp->_filePtr);
    }

    if (fp->_osclFileServerSession->Connect() != 0)
        return false;

    fp->_filePtr->SetPVCacheSize(aCacheSize);
    return (fp->_filePtr->Open(fileName.get_cstr(), mode, *(fp->_osclFileServerSession)) == 0);
}

void
PVA_FF_AtomUtils::closeFile(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    fp->_filePtr->Close();
    //OSCL_DELETE(fp->_filePtr);
    PV_MP4_FF_DELETE(NULL, Oscl_File, fp->_filePtr);

    fp->_filePtr = NULL;
}

void PVA_FF_AtomUtils::closeFileSession(Oscl_FileServer* fs)
{
    if (fs)
        fs->Close();
}

bool
PVA_FF_AtomUtils::readByteData(MP4_AUTHOR_FF_FILE_IO_WRAP *fp, uint32 length, uint8 *data)
{
    uint32 bytesRead;
    bytesRead = fp->_filePtr->Read(data, 1, length);

    if (bytesRead < (uint32)length) // read byte data failed
        return false;
    return true;
}


