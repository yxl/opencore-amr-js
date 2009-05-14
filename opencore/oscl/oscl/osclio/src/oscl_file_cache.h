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

//                     O S C L _ F I L E  _ C A C H E

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \addtogroup osclio OSCL IO
 *
 * @{
 */


/*! \file oscl_file_cache.h
    \brief The file oscl_file_cache.h defines the  class OsclFileCache

*/

#ifndef OSCL_FILE_CACHE_H_INCLUDED
#define OSCL_FILE_CACHE_H_INCLUDED

#ifndef OSCLCONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#include "oscl_file_io.h"

class Oscl_File;

class OsclFileCache : public HeapBase
{
    public:
        OsclFileCache(Oscl_File& aContainer);
        ~OsclFileCache();

        int32 Open(uint32 mode, uint32 cache_size);

        void Close();

        uint32 Read(void* outputBuffer, uint32 size, uint32 numelements);

        uint32 Write(const void* inputBuffer, uint32 size, uint32 numelements);

        TOsclFileOffset  FileSize()
        {
            return _fileSize;
        }

        int32 Seek(TOsclFileOffset  offset, Oscl_File::seek_type origin);

        TOsclFileOffset  Tell()
        {
            return (_cacheFilePosition + _currentCachePos);
        }

        int32 Flush();

        int32 EndOfFile()
        {
            return (Tell() == FileSize()) ? 1 : 0;
        }

    private:
        Oscl_File& iContainer;

        //file mode from the Open call.
        uint32 _mode;

        //Size of the cache buffer, set by the Open call.
        uint32 _cacheSize;

        //constant pointer to cache buffer
        uint8*    _pCacheBufferStart;

        //the native file position corresponding to the start of the
        //cache
        TOsclFileOffset  _cacheFilePosition;

        //current working position (virtual file pointer) in the cache.
        //units: 0-based byte offset from beginning of cache
        uint32	  _currentCachePos;

        //end of valid data in the cache.
        //units: 0-based byte offset from beginning of cache
        uint32	  _endCachePos;

        //variables to track the range of data in the cache that
        //has been updated by write operations, but has not yet
        //been written to disk.
        //units: 0-based byte offset from beginning of cache
        uint32	  _cacheUpdateStart;
        uint32    _cacheUpdateEnd;

        //Current file size.  This is a virtual file size and
        //may not match the native file size when there is
        //cached data.
        TOsclFileOffset 	_fileSize;

        //Current true native file position.
        TOsclFileOffset  _nativePosition;

        int32 SetCachePosition(TOsclFileOffset  pos);
        int32 FillCacheFromFile();
        int32 WriteCacheToFile();

        PVLogger* iLogger;
};


#endif // OSCL_FILE_CACHE_H_INCLUDED

/*! @} */

