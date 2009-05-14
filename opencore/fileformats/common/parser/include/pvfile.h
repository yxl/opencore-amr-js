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

//           A U D I O    M P 3   G E T    I D 3    I N F O

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


/**
 *  @file pvfile.h
 *  @brief This include file contains the definitions and classes needed
 *  to access a file either through Oscl File I/O or CPM.
 */


#ifndef PVFILE_H_INCLUDED
#define PVFILE_H_INCLUDED

#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

/**
* PVFile is a class with an API similar to Oscl File I/O.
* It allows the parser libraries to access a local file
* source either through CPM or through Oscl File I/O, depending
* on the parameters provided in the local source data.
*/

class PVMFCPMPluginAccessInterfaceFactory;
class PVMIDataStreamSyncInterface;
class PvmiDataStreamObserver;

#define PVFILE_DEFAULT_CACHE_SIZE 4*1024
#define PVFILE_DEFAULT_ASYNC_READ_BUFFER_SIZE 0
#define PVFILE_DEFAULT_NATIVE_ACCESS_MODE 0

class PVFileCacheParams
{
    public:
        PVFileCacheParams()
        {
            iCacheSize = PVFILE_DEFAULT_CACHE_SIZE;
            iAsyncReadBuffSize = PVFILE_DEFAULT_ASYNC_READ_BUFFER_SIZE;
            iNativeAccessMode = PVFILE_DEFAULT_NATIVE_ACCESS_MODE;
            iPVLoggerEnableFlag = false;
            iPVLoggerStateEnableFlag = false;
        }

        PVFileCacheParams(const PVFileCacheParams& a)
        {
            MyCopy(a);
        }

        /**
         * The assignment operator
         */
        PVFileCacheParams& operator=(const PVFileCacheParams& a)
        {
            if (&a != this)
            {
                MyCopy(a);
            }
            return *this;
        }

        uint32 iCacheSize;
        uint32 iAsyncReadBuffSize;
        bool iPVLoggerEnableFlag;
        bool iPVLoggerStateEnableFlag;
        uint32 iNativeAccessMode;

    private:
        void MyCopy(const PVFileCacheParams& a)
        {
            iCacheSize = a.iCacheSize;
            iAsyncReadBuffSize = a.iAsyncReadBuffSize;
            iPVLoggerEnableFlag = a.iPVLoggerEnableFlag;
            iPVLoggerStateEnableFlag = a.iPVLoggerStateEnableFlag;
            iNativeAccessMode = a.iNativeAccessMode;
        }
};

class PVFile
{
    public:
        //default constructor.
        PVFile()
        {
            Reset();
        }

        //API to copy all fields from another object.
        void Copy(const PVFile&a)
        {
            iFile = a.iFile;
            iFilePtr = a.iFilePtr;
            iFileHandle = a.iFileHandle;
            iCPMAccessFactory = a.iCPMAccessFactory;
            iDataStreamAccess = a.iDataStreamAccess;
            iFileSize = a.iFileSize;
            iFileSizeAvailable = a.iFileSizeAvailable;
            iDataStreamSession = a.iDataStreamSession;
            iRequestReadCapacityNotificationID = a.iRequestReadCapacityNotificationID;
            iOsclFileCacheParams = a.iOsclFileCacheParams;
        }

        //copy constructor.
        PVFile(const PVFile&a)
        {
            Copy(a);
        }

        ~PVFile()
        {
        }

        void Reset()
        {
            iFile = NULL;
            iFileHandle = NULL;
            iCPMAccessFactory = NULL;
            iFilePtr = NULL;
            iDataStreamAccess = NULL;
            iFileSize = 0;
            iFileSizeAvailable = false;
            iRequestReadCapacityNotificationID = 0;
        }

        //API to set the CPM access interface.
        //The CPM access interface is provided in the
        //local file source data.
        void SetCPM(PVMFCPMPluginAccessInterfaceFactory*a)
        {
            iCPMAccessFactory = a;
        }

        //API to get the CPM access interface factory
        PVMFCPMPluginAccessInterfaceFactory* GetCPM()
        {
            return iCPMAccessFactory;
        }

        //API to set the internal file handle.
        //The file handle is provided in the local file source data.
        void SetFileHandle(OsclFileHandle*a)
        {
            iFileHandle = a;
        }

        //To support access through an existing, open Oscl_File object.
        //This was an add-on for some MP4 file parser libraries-- not
        //used with the normal local file source access.
        //With this access mode, Open/Close should not be called.
        void SetFilePtr(Oscl_File*a)
        {
            iFilePtr = a;
        }

        //API to copy the access parameters from another object
        void SetAccess(const PVFile& a)
        {
            iCPMAccessFactory = a.iCPMAccessFactory;
            iFileHandle = a.iFileHandle;
            iFilePtr = a.iFilePtr;
            iFile = a.iFile;
        }

        //API to tell whether file is open.
        bool IsOpen()
        {
            if ((iFilePtr != NULL) ||
                    (iFile != NULL) ||
                    (iDataStreamAccess != NULL))
            {
                return true;
            }
            return false;
        }

        //Only valid in case Oscl_File class is being used
        //internally for file access. In case of datastream
        //access or in case of Oscl_File ptr provided for outside
        //via SetFilePtr API, it is assumed that implementor of datastream
        //has picked appropriate caching mechanism.
        void SetFileCacheParams(PVFileCacheParams aParams)
        {
            iOsclFileCacheParams = aParams;
        }

        //Access APIs, same functionality as Oscl File I/O.
        OSCL_IMPORT_REF int32 Seek(int32 offset, Oscl_File::seek_type origin);
        OSCL_IMPORT_REF int32 Tell();
        OSCL_IMPORT_REF uint32 Read(OsclAny *buffer, uint32 size, uint32 numelements) ;
        OSCL_IMPORT_REF int32 Close();
        OSCL_IMPORT_REF int32 Flush();
        OSCL_IMPORT_REF int32 Open(const oscl_wchar *filename,
                                   uint32 mode,
                                   Oscl_FileServer& fileserv);
        OSCL_IMPORT_REF bool GetRemainingBytes(uint32& aNumBytes);

        OSCL_IMPORT_REF bool RequestReadCapacityNotification(PvmiDataStreamObserver& aObserver,
                uint32 aCapacity,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF bool CancelNotificationSync();

        // For memory buffer data stream
        // file size is content length from HTTP HEAD response
        OSCL_IMPORT_REF uint32 GetContentLength();

        // memory cache size, is 0 if not MBDS
        OSCL_IMPORT_REF uint32 GetFileBufferingCapacity();

        // For memory buffer data stream
        // portion of the data can be copied to persistent storage for the duration of playback
        OSCL_IMPORT_REF bool MakePersistent(int32 offset, uint32 size);

        // For memory buffer data stream
        // to trigger a http GET request
        // like a seek but read pointer doesn't change
        OSCL_IMPORT_REF int32 Skip(int32 offset, Oscl_File::seek_type origin);

        // For memory buffer data stream
        // returns the byte range availabe in the stream cache
        OSCL_IMPORT_REF void GetCurrentByteRange(uint32& aCurrentFirstByteOffset, uint32& aCurrentLastByteOffset);

    private:
        //Access parameters from local source data
        PVMFCPMPluginAccessInterfaceFactory* iCPMAccessFactory;
        OsclFileHandle* iFileHandle;

        //Internal implementation.
        Oscl_File* iFile;
        PVMIDataStreamSyncInterface* iDataStreamAccess;
        int32 iDataStreamSession;

        //For the alternate access method-- through an existing file object.
        Oscl_File* iFilePtr;
        uint32 iFileSize;
        bool iFileSizeAvailable;
        uint32 iRequestReadCapacityNotificationID;
        PVFileCacheParams iOsclFileCacheParams;
};



#endif  // PVFILE_H_INCLUDED

