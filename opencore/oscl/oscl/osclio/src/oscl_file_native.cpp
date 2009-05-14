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
/*! \file oscl_file_native.cpp
	\brief This file contains file io APIs
*/

#include "oscl_file_native.h"
#include "oscl_stdstring.h"
#include "oscl_utf8conv.h"
#include "oscl_int64_utils.h"

#include "oscl_mem.h"
#include "oscl_file_types.h"
#include "oscl_file_handle.h"


OsclNativeFile::OsclNativeFile()
{
    iOpenFileHandle = false;
    iMode = 0;

    iFile = 0;
#if ENABLE_MEMORY_PLAYBACK
    membase = NULL;
#endif

}

OsclNativeFile::~OsclNativeFile()
{
}

int32  OsclNativeFile::Open(const OsclFileHandle& aHandle, uint32 mode
                            , const OsclNativeFileParams& params
                            , Oscl_FileServer& fileserv)
{
    //open with an external file handle

    OSCL_UNUSED_ARG(fileserv);

    iMode = mode;
    iOpenFileHandle = true;

    {
        OSCL_UNUSED_ARG(params);
        //Just save the open file handle
        iFile = aHandle.Handle();
    }

    return 0;
}

int32 OsclNativeFile::Open(const oscl_wchar *filename, uint32 mode
                           , const OsclNativeFileParams& params
                           , Oscl_FileServer& fileserv)
{
    iMode = mode;
    iOpenFileHandle = false;

    {
        OSCL_UNUSED_ARG(fileserv);
        OSCL_UNUSED_ARG(params);

        if (!filename || *filename == '\0') return -1; // Null string not supported in fopen, error out

        char openmode[4];
        uint32 index = 0;

        if (mode & Oscl_File::MODE_READWRITE)
        {
            if (mode & Oscl_File::MODE_APPEND)
            {
                openmode[index++] = 'a';
                openmode[index++] = '+';
            }
            else
            {
                openmode[index++] = 'w';
                openmode[index++] = '+';
            }
        }
        else if (mode & Oscl_File::MODE_APPEND)
        {
            openmode[index++] = 'a';
            openmode[index++] = '+';
        }
        else if (mode & Oscl_File::MODE_READ)
        {
            openmode[index++] = 'r';
        }
        else if (mode & Oscl_File::MODE_READ_PLUS)
        {
            openmode[index++] = 'r';
            openmode[index++] = '+';
        }



        if (mode & Oscl_File::MODE_TEXT)
        {
            openmode[index++] = 't';
        }
        else
        {
            openmode[index++] = 'b';
        }

        openmode[index++] = '\0';

#ifdef _UNICODE
        oscl_wchar convopenmode[4];
        if (0 == oscl_UTF8ToUnicode(openmode, oscl_strlen(openmode), convopenmode, 4))
        {
            return -1;
        }

        if ((iFile = _wfopen(filename, convopenmode)) == NULL)
        {
            return -1;
        }
#else
        //Convert to UTF8
        char convfilename[OSCL_IO_FILENAME_MAXLEN];
        if (0 == oscl_UnicodeToUTF8(filename, oscl_strlen(filename), convfilename, OSCL_IO_FILENAME_MAXLEN))
        {
            return -1;
        }
#if ENABLE_MEMORY_PLAYBACK
        void* base;
        long long offset;
        long long len;
        if (sscanf(convfilename, "mem://%p:%lld:%lld", &base, &offset, &len) == 3)
        {
            membase = base;
            memoffset = offset;
            memlen = len;
            mempos = 0;
        }
        else
#endif
        {
            if ((iFile = fopen(convfilename, openmode)) == NULL)
            {
                return -1;
            }
        }
#endif

        return 0;
    }

}

int32 OsclNativeFile::Open(const char *filename, uint32 mode
                           , const OsclNativeFileParams& params
                           , Oscl_FileServer& fileserv)
{
    iMode = mode;
    iOpenFileHandle = false;

    {
        OSCL_UNUSED_ARG(fileserv);
        OSCL_UNUSED_ARG(params);

        if (!filename || *filename == '\0') return -1; // Null string not supported in fopen, error out

        char openmode[4];
        uint32 index = 0;

        if (mode & Oscl_File::MODE_READWRITE)
        {
            if (mode & Oscl_File::MODE_APPEND)
            {
                openmode[index++] = 'a';
                openmode[index++] = '+';
            }
            else
            {
                openmode[index++] = 'w';
                openmode[index++] = '+';

            }
        }
        else if (mode & Oscl_File::MODE_APPEND)
        {
            openmode[index++] = 'a';
            openmode[index++] = '+';
        }
        else if (mode & Oscl_File::MODE_READ)
        {
            openmode[index++] = 'r';
        }
        else if (mode & Oscl_File::MODE_READ_PLUS)
        {
            openmode[index++] = 'r';
            openmode[index++] = '+';
        }

        if (mode & Oscl_File::MODE_TEXT)
        {
            openmode[index++] = 't';
        }
        else
        {
            openmode[index++] = 'b';
        }

        openmode[index++] = '\0';
#if ENABLE_MEMORY_PLAYBACK
        void* base;
        long long offset;
        long long len;
        if (sscanf(filename, "mem://%p:%lld:%lld", &base, &offset, &len) == 3)
        {
            membase = (void*)base;
            memoffset = offset;
            memlen = len;
            mempos = 0;
        }
        else
#endif
        {
            if ((iFile = fopen(filename, openmode)) == NULL)
            {
                return -1;
            }
        }

        return 0;
    }

}

TOsclFileOffset OsclNativeFile::Size()
{
    //this is the default for platforms with no
    //native size query.
    //Just do seek to end, tell, then seek back.
    TOsclFileOffset curPos = Tell();
    if (curPos >= 0
            && Seek(0, Oscl_File::SEEKEND) == 0)
    {
        TOsclFileOffset endPos = Tell();
        if (Seek(curPos, Oscl_File::SEEKSET) == 0)
        {
            return endPos;
        }
        else
        {
            return (-1);
        }
    }
    return (-1);
}

int32 OsclNativeFile::Close()
{
    int32 closeret = 0;

    {
        if (iOpenFileHandle)
            closeret = Flush();
        else if (iFile != NULL)
        {
            closeret = fclose(iFile);
            iFile = NULL;
        }
#if ENABLE_MEMORY_PLAYBACK
        else if (membase != NULL)
        {
            membase = NULL;
            return 0;
        }
#endif
        else
        {
            return -1; //Linux Porting : Fix 1
        }
    }

    return closeret;
}

uint32 OsclNativeFile::Read(OsclAny *buffer, uint32 size, uint32 numelements)
{
#if ENABLE_MEMORY_PLAYBACK
    if (membase)
    {
        int req = size * numelements;
        if (mempos + req > memlen)
        {
            req = memlen - mempos;
        }

        memcpyfailed = 0;
        memcpy(buffer, ((char*)membase) + memoffset + mempos, req);
        if (memcpyfailed)
        {
            return 0;
        }
        mempos += req;
        return req / size;
    }
#endif
    if (iFile)
    {
        return fread(buffer, OSCL_STATIC_CAST(int32, size), OSCL_STATIC_CAST(int32, numelements), iFile);
    }
    return 0;
}

bool OsclNativeFile::HasAsyncRead()
{
    return false;//not supported.
}

int32 OsclNativeFile::ReadAsync(OsclAny*buffer, uint32 size, uint32 numelements, OsclAOStatus& status)
{
    OSCL_UNUSED_ARG(buffer);
    OSCL_UNUSED_ARG(size);
    OSCL_UNUSED_ARG(numelements);
    OSCL_UNUSED_ARG(status);
    return -1;//not supported
}

void OsclNativeFile::ReadAsyncCancel()
{
}

uint32 OsclNativeFile::GetReadAsyncNumElements()
{
    return 0;//not supported
}



uint32 OsclNativeFile::Write(const OsclAny *buffer, uint32 size, uint32 numelements)
{
#if ENABLE_MEMORY_PLAYBACK
    if (membase)
        return 0;
#endif
    if (iFile)
    {
        return fwrite(buffer, OSCL_STATIC_CAST(int32, size), OSCL_STATIC_CAST(int32, numelements), iFile);
    }
    return 0;
}

int32 OsclNativeFile::Seek(TOsclFileOffset offset, Oscl_File::seek_type origin)
{

    {
#if ENABLE_MEMORY_PLAYBACK
        if (membase)
        {
            int newpos = mempos;
            if (origin == Oscl_File::SEEKCUR)
                newpos = mempos + offset;
            else if (origin == Oscl_File::SEEKSET)
                newpos = offset;
            else if (origin == Oscl_File::SEEKEND)
                newpos = memlen + offset;
            if (newpos < 0)
                return EINVAL;
            if (newpos > memlen) // is this valid?
                newpos = memlen;
            mempos = newpos;
            return 0;
        }
#endif
        if (iFile)
        {
            int32 seekmode = SEEK_CUR;

            if (origin == Oscl_File::SEEKCUR)
                seekmode = SEEK_CUR;
            else if (origin == Oscl_File::SEEKSET)
                seekmode = SEEK_SET;
            else if (origin == Oscl_File::SEEKEND)
                seekmode = SEEK_END;
#if OSCL_HAS_LARGE_FILE_SUPPORT
            return fseeko(iFile, offset, seekmode);
#else
            return fseek(iFile, offset, seekmode);
#endif
        }
    }
    return -1;
}


TOsclFileOffset OsclNativeFile::Tell()
{
    TOsclFileOffset result = -1;
#if ENABLE_MEMORY_PLAYBACK
    if (membase)
    {
        result = mempos;
    }
#endif
    if (iFile)
    {
#if OSCL_HAS_LARGE_FILE_SUPPORT
        result = ftello(iFile);
#else
        result = ftell(iFile);
#endif
    }
    return result;
}



int32 OsclNativeFile::Flush()
{

#if ENABLE_MEMORY_PLAYBACK
    if (membase)
        return 0;
#endif
    if (iFile)
        return fflush(iFile);
    return EOF;
}



int32 OsclNativeFile::EndOfFile()
{

#if ENABLE_MEMORY_PLAYBACK
    if (membase)
        return mempos >= memlen;
#endif
    if (iFile)
        return feof(iFile);
    return 0;
}


int32 OsclNativeFile::GetError()
{
#if ENABLE_MEMORY_PLAYBACK
    if (membase)
        return 0;
#endif
    if (iFile)
        return ferror(iFile);
    return 0;
}

