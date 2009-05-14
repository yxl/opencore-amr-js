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

//                     O S C L _ F I L E  _ N A T I V E

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \addtogroup osclio OSCL IO
 *
 * @{
 */


/*! \file oscl_file_native.h
    \brief The file oscl_file_native.h defines the class OsclNativeFile.
	This is the porting layer for basic file I/O operations.
*/

#ifndef OSCL_FILE_NATIVE_H_INCLUDED
#define OSCL_FILE_NATIVE_H_INCLUDED

#ifndef OSCLCONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_AOSTATUS_H_INCLUDED
#include "oscl_aostatus.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef OSCL_FILE_TYPES_H_INCLUDED
#include "oscl_file_types.h"
#endif


/**
 * OsclNativeFileIO defines the native file operations that must be implemented
 * by every Oscl platform.
 */

class Oscl_FileServer;
class OsclNativeFile : public HeapBase
{
    public:
        OsclNativeFile();
        ~OsclNativeFile();

        // These are the Native OS File IO calls.
        int32  Open(const OsclFileHandle&, uint32 mode
                    , const OsclNativeFileParams& params
                    , Oscl_FileServer& fileserv);
        int32  Open(const oscl_wchar *filename, uint32 mode
                    , const OsclNativeFileParams& params
                    , Oscl_FileServer& fileserv);
        int32  Open(const char *filename, uint32 mode
                    , const OsclNativeFileParams& params
                    , Oscl_FileServer& fileserv);
        uint32 Read(OsclAny *buffer, uint32 size, uint32 numelements);
        uint32 Write(const OsclAny *buffer, uint32 size, uint32 numelements);
        int32  Seek(TOsclFileOffset offset, Oscl_File::seek_type origin);
        TOsclFileOffset  Tell();
        int32  Flush();
        int32  EndOfFile();
        TOsclFileOffset Size();
        int32 Close();

        //query for current open mode.
        uint32 Mode()
        {
            return iMode;
        }

        //get last filesystem error.
        int32 GetError();

        /*!
        ** Asynchronous read.
        **
        ** @param buffer: data buffer, must be at least size*numelements bytes
        ** @param size: size of elements
        ** @param numelements: number of elements to read
        ** @param status: Request status for asynchronous completion
        ** @returns: 0 for success.
        **/
        int32 ReadAsync(OsclAny*buffer, uint32 size, uint32 numelements, OsclAOStatus& status);
        /*!
        ** Get the number of elements read in the last call to ReadAsync.
        ** @returns: number of elements read.
        **/
        uint32 GetReadAsyncNumElements();
        /*!
        ** @returns: true if async read is supported natively.
        **/
        bool HasAsyncRead();
        /*!
        ** Cancel any pending async read.
        **/
        void ReadAsyncCancel();

    private:

        //current open mode
        uint32 iMode;

        //was an open file handle passed in?
        bool iOpenFileHandle;

        //native file object.
        FILE* iFile;
#if ENABLE_MEMORY_PLAYBACK
        // memory block
        void* membase;
        long long memoffset;
        long long memlen;
        int mempos;
        int memcpyfailed;
#endif

};

#endif // OSCL_FILE_NATIVE_H_INCLUDED

/*! @} */

