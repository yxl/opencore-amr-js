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
/*! \file oscl_file_io.cpp
    \brief This file contains file io APIs
*/

#include "oscl_file_io.h"
#include "oscl_dll.h"
#include "oscl_mem.h"
#include "oscl_file_native.h"
#include "oscl_file_handle.h"
#include "oscl_file_cache.h"
#include "pvlogger.h"
#include "oscl_string_containers.h"
#include "oscl_file_stats.h"
#include "oscl_file_async_read.h"

#ifndef OSCL_COMBINED_DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()
#endif


OSCL_EXPORT_REF Oscl_File::Oscl_File()
{
    Construct();

    //for back-compatibility, set the old compile-time defaults.
    OldCacheDefaults();
}

void Oscl_File::Construct()
{
    iOpenFileHandle = NULL;
    iLogger = NULL;
    iStatsLogger = NULL;
    iNativeLogger = NULL;
    iAsyncLogger = NULL;
    iFileStats = NULL;
    iNativeFile = NULL;
    iFileCache = NULL;
    iIsOpen = false;
    iNativeBufferSize = 0;
    iNativeAccessMode = 0;
    iPVCacheSize = 0;
    iAsyncReadBufferSize = 0;
    iAsyncFile = NULL;

    //Create the native file I/O implementation
    int32 err;
    OSCL_TRY(err, iNativeFile = OSCL_NEW(OsclNativeFile, ()););
}

//For back-compatibility, this sets cache defaults using the
//compile-switches (if defined).
void Oscl_File::OldCacheDefaults()
{

    //native file cache enable chooses RFileBuf mode over RFile mode
    //for symbian.

    SetNativeAccessMode(ESymbianAccessMode_RfileBuf);

#if defined(OSCL_FILE_BUFFER_MAX_SIZE) 
    //native buffer size defaults to max buffer size

    SetNativeBufferSize(OSCL_FILE_BUFFER_MAX_SIZE);
#endif

#if defined(OSCL_ASYNC_READ_BUFFER_SIZE) 
    // enable async file read operation

    SetAsyncReadBufferSize(OSCL_ASYNC_READ_BUFFER_SIZE);
#endif


}

//For back-compatibility constructors.
//This sets the cache settings using a combination of the input value
//and the compile-time settings (if defined).
void Oscl_File::OldCacheSelect(uint32 aCacheSize)
{
    uint32 cacheSize = aCacheSize;

    //file buffer max limits the actual cache size when defined.
#if defined(OSCL_FILE_BUFFER_MAX_SIZE)
    if (cacheSize > OSCL_FILE_BUFFER_MAX_SIZE)
        cacheSize = OSCL_FILE_BUFFER_MAX_SIZE;
#endif


    //cache option is "off" (or not defined)

    //cache option is "on"
    SetNativeBufferSize(cacheSize);

}

//This constructor is deprecated, but present for back-compatibility.
OSCL_EXPORT_REF Oscl_File::Oscl_File(uint32 cacheSize)
{
    Construct();
    OldCacheDefaults();
    OldCacheSelect(cacheSize);
}

//This constructor is deprecated, but present for back-compatibility.
OSCL_EXPORT_REF Oscl_File::Oscl_File(uint32 cacheSize, OsclFileHandle* aHandle)
{
    Construct();
    OldCacheDefaults();
    OldCacheSelect(cacheSize);
    SetFileHandle(aHandle);
}

OSCL_EXPORT_REF Oscl_File::~Oscl_File()
{
    if (iIsOpen)
        Close();

    if (iOpenFileHandle)
        OSCL_DELETE(iOpenFileHandle);
    iOpenFileHandle = NULL;

    if (iFileCache)
        OSCL_DELETE(iFileCache);
    iFileCache = NULL;

    if (iAsyncFile)
        OsclAsyncFile::Delete(iAsyncFile);
    iAsyncFile = NULL;

    if (iNativeFile)
        OSCL_DELETE(iNativeFile);
    iNativeFile = NULL;

    if (iFileStats)
        OSCL_DELETE(iFileStats);
    iFileStats = NULL;
}

OSCL_EXPORT_REF void Oscl_File::SetPVCacheSize(uint32 aSize)
{
    //just save the value now-- it will take effect on the next open.
    iPVCacheSize = aSize;
}

OSCL_EXPORT_REF void Oscl_File::SetAsyncReadBufferSize(uint32 aSize)
{
    //just save the value now-- it will take effect on the next open.
    iAsyncReadBufferSize = aSize;

}

OSCL_EXPORT_REF void Oscl_File::SetLoggingEnable(bool aEnable)
{
    if (aEnable)
    {
        iLogger = PVLogger::GetLoggerObject("Oscl_File");
        iNativeLogger = PVLogger::GetLoggerObject("OsclNativeFile");
        iAsyncLogger = PVLogger::GetLoggerObject("OsclAsyncFile");
    }
    else
    {
        iLogger = NULL;
        iNativeLogger = NULL;
        iAsyncLogger = NULL;
    }
}

OSCL_EXPORT_REF void Oscl_File::SetSummaryStatsLoggingEnable(bool aEnable)
{
    if (aEnable)
    {
        iStatsLogger = PVLogger::GetLoggerObject("OsclFileStats");
    }
    else
    {
        iStatsLogger = NULL;
    }
    CreateFileStats();
}

void Oscl_File::CreateFileStats()
{
    if (iFileStats)
        OSCL_DELETE(iFileStats);
    iFileStats = NULL;
    if (iStatsLogger)
    {
        int32 err;
        OSCL_TRY(err, iFileStats = OSCL_NEW(OsclFileStats, (this)););
    }
}


OSCL_EXPORT_REF void Oscl_File::SetNativeAccessMode(uint32 aMode)
{
    //just save the value now-- it will take effect on the next open.
    iNativeAccessMode = aMode;
}

OSCL_EXPORT_REF void Oscl_File::SetNativeBufferSize(int32 aSize)
{
    //just save the value now-- it will take effect on the next open.
    iNativeBufferSize = aSize;
}

OSCL_EXPORT_REF int32 Oscl_File::SetFileHandle(OsclFileHandle* aHandle)
{
    if (iIsOpen)
        return -1;//can't reset file handle when file is open.

    if (iOpenFileHandle)
        OSCL_DELETE(iOpenFileHandle);
    iOpenFileHandle = NULL;

    //make a copy to avoid memory persistence issues
    if (aHandle)
    {
        int32 err;
        OSCL_TRY(err, iOpenFileHandle = OSCL_NEW(OsclFileHandle, (*aHandle)););
        if (!iOpenFileHandle)
            return (-1);//allocation failed.
    }

    return 0;
}


#include "pvlogger.h"

int32 Oscl_File::OpenFileCacheOrAsyncBuffer(const char *filename
        , const oscl_wchar* wfilename
        , uint32 mode
        , const OsclNativeFileParams& params
        , Oscl_FileServer& fileserv)
//open pv cache or async file implementation for the given open mode.
//if cache is not enabled, then cleanup any old implementation.
{
    //use async buffer when read-only mode and iAsyncReadBufferSize is > 0
    bool asyncEnable = ((mode == MODE_READ || mode == MODE_READ + MODE_BINARY) && iAsyncReadBufferSize > 0);

    //use cache when *not* using async file and iPVCacheSize is > 0
    bool cacheEnable = !asyncEnable && (iPVCacheSize > 0);

    if (cacheEnable)
    {
        //create file cache implementation if needed.
        if (!iFileCache)
        {
            int32 err;
            OSCL_TRY(err, iFileCache = OSCL_NEW(OsclFileCache, (*this)););
            if (!iFileCache)
                return -1;//allocation failed
        }
    }
    else
    {
        //cleanup any old implementation of file cache
        if (iFileCache)
            OSCL_DELETE(iFileCache);
        iFileCache = NULL;
    }

    if (asyncEnable)
    {
        //async file implementation isn't reusable so destroy
        //& re-create it
        if (iAsyncFile)
            OsclAsyncFile::Delete(iAsyncFile);
        iAsyncFile = NULL;
        int32 err;
        OSCL_TRY(err, iAsyncFile = OsclAsyncFile::NewL(*iNativeFile, iAsyncReadBufferSize, iAsyncLogger););
        if (!iAsyncFile)
            return -1;//allocation failed
    }
    else
    {
        //cleanup any old implementation of async file.
        if (iAsyncFile)
            OsclAsyncFile::Delete(iAsyncFile);
        iAsyncFile = NULL;
    }

    //at this point we have either iFileCache, iAsyncFile, or neither.

    if (iFileCache)
    {
        return iFileCache->Open(mode, iPVCacheSize);
    }
    else if (iAsyncFile)
    {
        if (filename)
            return iAsyncFile->Open(filename, mode, params, fileserv);
        else if (wfilename)
            return iAsyncFile->Open(wfilename, mode, params, fileserv);
        else
            return -1;
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF int32 Oscl_File::Open(const oscl_wchar *filename, uint32 mode, Oscl_FileServer& fileserv)
{
    if (iLogger)
    {
        OSCL_wHeapString<OsclMemAllocator> wstr(filename);
        OSCL_HeapString<OsclMemAllocator> str;
        char buf[2];
        buf[1] = '\0';
        for (uint32 i = 0;i < wstr.get_size();i++)
        {
            buf[0] = (char)wstr[i];
            str += buf;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Open IN name '%s' mode %d serv 0x%x", this, str.get_cstr(), mode, &fileserv));
    }

    int32 result = (-1);

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    //protect against duplicate open calls
    if (iIsOpen)
        goto ErrorExit;

    //do native open or attach
    {
        OsclNativeFileParams params(iNativeAccessMode, iNativeBufferSize, iAsyncReadBufferSize);
        if (iOpenFileHandle)
            result = CallNativeOpen(*iOpenFileHandle, mode, params, fileserv);
        else
            result = CallNativeOpen(filename, mode , params, fileserv);

        if (result != 0)
            goto ErrorExit;

        //create cache implementation if needed.
        result = OpenFileCacheOrAsyncBuffer(NULL, filename, mode, params, fileserv);
    }
    if (result != 0)
        goto ErrorExit;

ErrorExit:

    if (result == 0)
        iIsOpen = true;

    if (iFileStats
            && result == 0)
        iFileStats->End(EOsclFileOp_Open, ticks);

    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Open OUT result %d", this, result));
    }

    return result;
}


OSCL_EXPORT_REF int32 Oscl_File::Open(const char *filename, uint32 mode, Oscl_FileServer& fileserv)
{
    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Open IN name '%s' mode %d serv 0x%x", this, filename, mode, &fileserv));
    }

    int32 result = (-1);

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    //protect against duplicate open calls
    if (iIsOpen)
        goto ErrorExit;

    //do native open or attach
    {
        OsclNativeFileParams params(iNativeAccessMode, iNativeBufferSize, iAsyncReadBufferSize);
        if (iOpenFileHandle)
            result = CallNativeOpen(*iOpenFileHandle, mode, params, fileserv);
        else
            result = CallNativeOpen(filename, mode , params, fileserv);

        if (result != 0)
            goto ErrorExit;

        //create file cache implementation if needed
        result = OpenFileCacheOrAsyncBuffer(filename, NULL, mode, params, fileserv);
    }
    if (result != 0)
        goto ErrorExit;

ErrorExit:

    if (result == 0)
        iIsOpen = true;

    if (iFileStats
            && result == 0)
        iFileStats->End(EOsclFileOp_Open, ticks);

    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Open OUT result %d", this, result));
    }

    return result;
}

OSCL_EXPORT_REF int32 Oscl_File::Close()
{
    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Close IN", this));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    int32 result = (-1);

    if (iIsOpen)
    {
        if (iFileCache)
            iFileCache->Close();
        else if (iAsyncFile)
            iAsyncFile->Close();

        result = CallNativeClose();
    }

    iIsOpen = false;

    if (iFileStats
            && result == 0)
    {
        iFileStats->End(EOsclFileOp_Close, ticks);
        iFileStats->LogAll(iStatsLogger, PVLOGMSG_DEBUG);
    }

    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Close OUT result %d", this, result));
    }

    return result;
}


OSCL_EXPORT_REF uint32 Oscl_File::Read(OsclAny *buffer, uint32 size, uint32 numelements)
{
    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Read IN size %d num %d", this, size, numelements));
    }

    if (!size || !numelements || !buffer)
    {
        OSCL_LEAVE(OsclErrArgument);
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    uint32 result = 0;

    if (iIsOpen)
    {
        if (iFileCache)
            result = iFileCache->Read(buffer, size, numelements);
        else if (iAsyncFile)
            result = iAsyncFile->Read(buffer, size, numelements);
        else
            result = CallNativeRead(buffer, size, numelements);
    }

    if (iFileStats
            && result > 0)
        iFileStats->End(EOsclFileOp_Read, ticks, result*size);

    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Read OUT result %d", this, result));
    }
    return result;
}


OSCL_EXPORT_REF uint32 Oscl_File::Write(const OsclAny *buffer, uint32 size, uint32 numelements)
{
    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Write IN size %d num %d", this, size, numelements));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    uint32 result = 0;

    if (iIsOpen)
    {
        if (iFileCache)
            result = iFileCache->Write(buffer, size, numelements);
        else if (iAsyncFile)
            return iAsyncFile->Write(buffer, size, numelements);
        else
            result = CallNativeWrite(buffer, size, numelements);
    }

    if (iFileStats
            && result > 0)
        iFileStats->End(EOsclFileOp_Write, ticks, result*size);

    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Write OUT result %d", this, result));
    }
    return result;
}

OSCL_EXPORT_REF int32 Oscl_File::Seek(TOsclFileOffset offset, seek_type origin)
{
    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Seek IN offset %d origin %d", this, offset, origin));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    int32 result = (-1);

    if (iIsOpen)
    {
        if (iFileCache)
            result = iFileCache->Seek(offset, origin);
        else if (iAsyncFile)
            result = iAsyncFile->Seek(offset, origin);
        else
            result = CallNativeSeek(offset, origin);
    }

    if (iFileStats
            && result == 0)
        iFileStats->End(EOsclFileOp_Seek, ticks, origin, offset);

    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Seek OUT result %d", this, result));
    }
    return result;
}


OSCL_EXPORT_REF TOsclFileOffset Oscl_File::Tell()
{
    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Tell IN", this));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    TOsclFileOffset result = (-1);

    if (iIsOpen)
    {
        if (iFileCache)
            result = iFileCache->Tell();
        else if (iAsyncFile)
            result = iAsyncFile->Tell();
        else
            result = CallNativeTell();
    }

    if (iFileStats
            && result == 0)
        iFileStats->End(EOsclFileOp_Tell, ticks);

    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Tell OUT result %d", this, result));
    }
    return result;
}

OSCL_EXPORT_REF int32 Oscl_File::Flush()
{
    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Flush IN", this));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    int32 result = (-1);

    if (iIsOpen)
    {
        if (iFileCache)
            result = iFileCache->Flush();
        else if (iAsyncFile)
            return iAsyncFile->Flush();
        else
            result = CallNativeFlush();
    }

    if (iFileStats
            && result == 0)
        iFileStats->End(EOsclFileOp_Flush, ticks);

    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Flush OUT result %d", this, result));
    }
    return result;
}


OSCL_EXPORT_REF int32 Oscl_File::EndOfFile()
{
    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::EndOfFile IN", this));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    int32 result = (-1);
    if (iIsOpen)
    {
        if (iFileCache)
            result = iFileCache->EndOfFile();
        else if (iAsyncFile)
            result = iAsyncFile->EndOfFile();
        else
            result = CallNativeEndOfFile();
    }

    if (iFileStats
            && result != (-1))
        iFileStats->End(EOsclFileOp_EndOfFile, ticks);

    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::EndOfFile OUT result %d", this, result));
    }
    return result;
}

OSCL_EXPORT_REF TOsclFileOffset Oscl_File::Size()
{
    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Size IN"));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    TOsclFileOffset result = (-1);

    if (iIsOpen)
    {
        if (iFileCache)
            result = iFileCache->FileSize();
        else if (iAsyncFile)
            result = iAsyncFile->Size();
        else
            result = CallNativeSize();
    }

    if (iFileStats
            && result != (-1))
        iFileStats->End(EOsclFileOp_Size, ticks);

    if (iLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "Oscl_File(0x%x)::Size OUT result %d", this, result));
    }
    return result;
}

OSCL_EXPORT_REF int32 Oscl_File::GetError()
{
    return CallNativeGetError();
}

int32  Oscl_File::CallNativeOpen(const OsclFileHandle& handle, uint32 mode
                                 , const OsclNativeFileParams& params
                                 , Oscl_FileServer& fileserv)
{
    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Open IN handle 0x%x mode %d serv 0x%x", this, &handle, mode, &fileserv));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    int32 result = (-1);

    if (iNativeFile)
        result = iNativeFile->Open(handle, mode, params, fileserv);

    if (iFileStats
            && result == 0)
        iFileStats->End(EOsclFileOp_NativeOpen, ticks);

    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Open OUT result %d", this, result));
    }

    return result;
}

int32  Oscl_File::CallNativeOpen(const oscl_wchar *filename, uint32 mode
                                 , const OsclNativeFileParams& params
                                 , Oscl_FileServer& fileserv)
{
    OSCL_UNUSED_ARG(params);

    if (iNativeLogger)
    {
        OSCL_wHeapString<OsclMemAllocator> wstr(filename);
        OSCL_HeapString<OsclMemAllocator> str;
        char buf[2];
        buf[1] = '\0';
        for (uint32 i = 0;i < wstr.get_size();i++)
        {
            buf[0] = (char)wstr[i];
            str += buf;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Open IN name '%s' mode %d serv 0x%x", this, str.get_cstr(), mode, &fileserv));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    int32 result = (-1);

    {
        OsclNativeFileParams params(iNativeAccessMode, iNativeBufferSize, iAsyncReadBufferSize);
        if (iNativeFile)
            result = iNativeFile->Open(filename, mode, params, fileserv);
    }

    if (iFileStats
            && result == 0)
        iFileStats->End(EOsclFileOp_NativeOpen, ticks);

    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Open OUT result %d", this, result));
    }

    return result;
}

int32  Oscl_File::CallNativeOpen(const char *filename, uint32 mode
                                 , const OsclNativeFileParams& params
                                 , Oscl_FileServer& fileserv)
{
    OSCL_UNUSED_ARG(params);

    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Open IN name '%s' mode %d serv 0x%x", this, filename, mode, &fileserv));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    int32 result = (-1);

    {
        OsclNativeFileParams params(iNativeAccessMode, iNativeBufferSize, iAsyncReadBufferSize);
        if (iNativeFile)
            result = iNativeFile->Open(filename, mode, params, fileserv);
    }

    if (iFileStats
            && result == 0)
        iFileStats->End(EOsclFileOp_NativeOpen, ticks);

    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Open OUT result %d", this, result));
    }

    return result;
}

uint32 Oscl_File::CallNativeRead(OsclAny *buffer, uint32 size, uint32 numelements)
{
    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Read IN size %d numelements %d", this, size, numelements));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    int32 result = 0;

    if (iNativeFile)
        result = iNativeFile->Read(buffer, size, numelements);

    if (iFileStats
            && result > 0)
        iFileStats->End(EOsclFileOp_NativeRead, ticks, result*size);

    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Read OUT result %d", this, result));
    }

    return result;
}

uint32 Oscl_File::CallNativeWrite(const OsclAny *buffer, uint32 size, uint32 numelements)
{
    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Write IN size %d numelements %d", this, size, numelements));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    int32 result = 0;

    if (iNativeFile)
        result = iNativeFile->Write(buffer, size, numelements);

    if (iFileStats
            && result > 0)
        iFileStats->End(EOsclFileOp_NativeWrite, ticks, result*size);

    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Write OUT result %d", this, result));
    }

    return result;
}

int32  Oscl_File::CallNativeSeek(TOsclFileOffset offset, Oscl_File::seek_type origin)
{
    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Seek IN offset %d origin %d", this, offset, origin));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    int32 result = (-1);

    if (iNativeFile)
        result = iNativeFile->Seek(offset, origin);

    if (iFileStats
            && result == 0)
        iFileStats->End(EOsclFileOp_NativeSeek, ticks, origin, offset);

    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Seek OUT result %d", this, result));
    }

    return result;
}

TOsclFileOffset  Oscl_File::CallNativeTell()
{
    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Tell IN", this));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    TOsclFileOffset result = (-1);

    if (iNativeFile)
        result = iNativeFile->Tell();

    if (iFileStats
            && result == 0)
        iFileStats->End(EOsclFileOp_NativeTell, ticks);

    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Tell OUT result %d", this, result));
    }

    return result;
}


int32  Oscl_File::CallNativeFlush()
{
    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Flush IN", this));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    int32 result = (-1);

    if (iNativeFile)
        result = iNativeFile->Flush();

    if (iFileStats
            && result == 0)
        iFileStats->End(EOsclFileOp_NativeFlush, ticks);

    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Flush OUT result", this, result));
    }

    return result;
}

int32  Oscl_File::CallNativeEndOfFile()
{
    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::EndOfFile IN", this));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    int32 result = (-1);

    if (iNativeFile)
        result = iNativeFile->EndOfFile();

    if (iFileStats
            && result == 0)
        iFileStats->End(EOsclFileOp_NativeEndOfFile, ticks);

    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::EndOfFile OUT result", this, result));
    }

    return result;
}

TOsclFileOffset Oscl_File::CallNativeSize()
{
    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Size IN", this));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    TOsclFileOffset result = (-1);

    if (iNativeFile)
        result = iNativeFile->Size();

    if (iFileStats
            && result >= 0)
        iFileStats->End(EOsclFileOp_NativeSize, ticks);

    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Size OUT result", this, result));
    }

    return result;
}

int32 Oscl_File::CallNativeClose()
{
    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Close IN", this));
    }

    uint32 ticks = 0;
    if (iFileStats)
        iFileStats->Start(ticks);

    int32 result = (-1);

    if (iNativeFile)
        result = iNativeFile->Close();

    if (iFileStats
            && result >= 0)
        iFileStats->End(EOsclFileOp_NativeClose, ticks);

    if (iNativeLogger)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iNativeLogger, PVLOGMSG_DEBUG,
                        (0, "OsclNativeFile(0x%x)::Close OUT result", this, result));
    }

    return result;
}

uint32 Oscl_File::CallNativeMode()
{
    int32 result = (-1);

    if (iNativeFile)
        result = iNativeFile->Mode();

    return result;
}

int32 Oscl_File::CallNativeGetError()
{
    int32 result = (-1);

    if (iNativeFile)
        result = iNativeFile->GetError();

    return result;
}

OSCL_EXPORT_REF uint32 Oscl_File::GetAsyncFileNumOfRun()
{
    if (iAsyncFile)
        return iAsyncFile->iNumOfRun;
    else
        return 0;
}

OSCL_EXPORT_REF uint32 Oscl_File::GetAsyncFileNumOfRunError()
{
    if (iAsyncFile)
        return iAsyncFile->iNumOfRunErr;
    else
        return 0;
}

