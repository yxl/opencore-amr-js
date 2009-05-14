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
/*
* ============================================================================
*  Name        : oscl_async_file.h
*  Part of     : osclio
*  Interface   :
*  Description :
*  Version     :
* ==============================================================================
*/

#ifndef OSCL_FILE_ASYNC_READ_H_INCLUDED
#define OSCL_FILE_ASYNC_READ_H_INCLUDED


#include "oscl_base.h"
#include "osclconfig_io.h"
#include "oscl_vector.h"
#include "oscl_mem.h"
#include "oscl_scheduler_ao.h"
#include "oscl_file_io.h"

#ifndef OSCL_SEMAPHORE_H_INCLUDED
#include "oscl_semaphore.h"
#endif

class OsclNativeFile;

//non-modifiable buffer pointer container
class OsclPtrC: public HeapBase
{
    public:
        OsclPtrC(const uint8* ptr, int32 len, int32 max): iPtr(ptr), iMaxLength(max), iLength(len)
        {}
        OsclPtrC(const OsclPtrC& d): HeapBase(d), iPtr(d.iPtr), iMaxLength(d.iMaxLength), iLength(d.iLength)
        {}
        const uint8* Ptr()
        {
            return iPtr;
        }
        void SetLength(int32 l)
        {
            OSCL_ASSERT(l <= iMaxLength);
            iLength = l;
        }
        int32 Length()
        {
            return iLength;
        }
        void Zero()
        {
            iLength = 0;
        }
        void Set(OsclPtrC* v)
        {
            iPtr = v->iPtr;
            iMaxLength = v->iMaxLength;
            iLength = v->iLength;
        }
        void Set(uint8* ptr, int32 len, int32 max)
        {
            iPtr = ptr;
            iLength = len;
            iMaxLength = max;
        }
        //extract the right-most data
        OsclPtrC Right(int32 size)
        {
            OSCL_ASSERT(iLength >= size);
            OsclPtrC des(iPtr + iLength - size, size, size);
            return des;
        }
        //extract the left-most data
        OsclPtrC Left(int32 size)
        {
            OSCL_ASSERT(iLength >= size);
            OsclPtrC des(iPtr, size, size);
            return des;
        }
    private:
        const uint8* iPtr;
        int32 iMaxLength;
        int32 iLength;
};

//modifiable buffer pointer container
class OsclPtr: public HeapBase
{
    public:
        OsclPtr(uint8* ptr, int32& len, int32 max): iPtr(ptr), iMaxLength(max), iLength(len)
        {}
        OsclPtr(const OsclPtr& d): HeapBase(d), iPtr(d.iPtr), iMaxLength(d.iMaxLength), iLength(d.iLength)
        {}
        uint8* Ptr()
        {
            return iPtr;
        }
        void SetLength(int32 l)
        {
            OSCL_ASSERT(l <= iMaxLength);
            iLength = l;
        }
        int32 Length()
        {
            return iLength;
        }
        void Zero()
        {
            iLength = 0;
        }
        void Set(OsclPtr &v)
        {
            iPtr = v.iPtr;
            iMaxLength = v.iMaxLength;
            iLength = v.iLength;
        }
        void Set(uint8* ptr, int32 len, int32 max)
        {
            iPtr = ptr;
            iLength = len;
            iMaxLength = max;
        }
        void Append(OsclPtrC &v)
        {
            OSCL_ASSERT(iLength + v.Length() <= iMaxLength);
            oscl_memmove(iPtr + iLength, v.Ptr(), v.Length());
            iLength += v.Length();
        }
    private:
        uint8* iPtr;
        int32 iMaxLength;
        int32& iLength;
};

//buffer container that allocates from the heap
class OsclBuf: public HeapBase
{
    public:
        static OsclBuf* NewL(int32 size)
        {
            OsclBuf* self = OSCL_NEW(OsclBuf, (size));
            self->iBuffer = (uint8*)OSCL_MALLOC(self->iMaxLength);
            if (!self->iBuffer)
            {
                OSCL_DELETE(self);
                OsclError::Leave(OsclErrNoMemory);
            }
            return self;
        }

        static void Delete(OsclBuf* a)
        {
            if (a)
            {
                if (a->iBuffer)
                    OSCL_FREE(a->iBuffer);
                OSCL_DELETE(a);
            }
        }

        OsclBuf(int32 size): iBuffer(NULL), iMaxLength(size), iLength(0)
        {}

        int32 Length()
        {
            return iLength;
        }

        OsclPtr Des()
        {
            OsclPtr des(iBuffer, iLength, iMaxLength);
            return des;
        }
        OsclPtrC DesC()
        {
            OsclPtrC des(iBuffer, iLength, iMaxLength);
            return des;
        }

        uint8* iBuffer;
        int32 iMaxLength;
        int32 iLength;
};


/*!
** Buffer class used with async read.  We keep an array of these, covering
** consecutive areas of the file.  This allows for some seeking without requiring
** a full flush & refill each time.
*/
class OsclAsyncFileBuffer: public HeapBase
{
    public:
        static OsclAsyncFileBuffer* NewL(int32 aBufferSize, int32 aId);
        ~OsclAsyncFileBuffer();

    public:
        void CleanInUse()
        {
            iInUse = false;
        }
        void SetInUse()
        {
            iInUse = true;
        }
        bool IsInUse()
        {
            return iInUse;
        }
        bool IsValid()
        {
            return iValid;
        }
        TOsclFileOffset Offset()
        {
            return iOffset;
        }
        void SetOffset(TOsclFileOffset aOffset)
        {
            iOffset = aOffset;
        }
        int32 Length()
        {
            return iLength;
        }
        bool HasThisOffset(TOsclFileOffset aOffset);
        int32 Id()
        {
            return iId;
        }
        OsclBuf* Buffer();
        void UpdateData();
        void StartAsyncRead(bool aStartAsyncRead);

    private:
        OsclAsyncFileBuffer(int32 aBufferSize, int32 aId);
        void ConstructL();

    private:
        OsclBuf* iBuffer;
        TOsclFileOffset iOffset;
        bool iInUse;
        int32 iLength;
        bool iValid;
        int32 iBufferSize;
        int32 iId;
};


/**
* OsclAsyncFile
*/
class OsclAsyncFile : public OsclActiveObject
{
    public:
        /**
        * Two-phased constructor.
        *
        * @param aAsyncFile: open handle for async file read.
        *   Note: it is the caller's job to open/close this file handle.
        *
        * @param aSyncFile: duplicate open handle for sync file read.
        *   Note: it is the caller's job to open this file handle, but this
        *     class will close the handle.
        *
        * @param aCacheSize: size of one of the individual cache buffers.  The total
        *    cached data size will be larger, since multiple buffers are used.
        *
        * @param aStartAsyncRead: When true, async file read will start immediately.
        *     When false, read will not begin until StartAsyncRead is called.
        *
        */
        static OsclAsyncFile* NewL(OsclNativeFile& aAsyncFile, int32 aCacheSize, PVLogger*);
        static void Delete(OsclAsyncFile*);

        /**
        * Destructor.
        */
        ~OsclAsyncFile();

    private:
        //From OsclActiveObject
        void Run();
        void DoCancel();

    public:
        ////////////////////////
        // File IO methods.
        ////////////////////////

        int32  Open(const oscl_wchar *filename, uint32 mode
                    , const OsclNativeFileParams& params
                    , Oscl_FileServer& fileserv);
        int32  Open(const char *filename, uint32 mode
                    , const OsclNativeFileParams& params
                    , Oscl_FileServer& fileserv);

        int32 Seek(TOsclFileOffset offset, Oscl_File::seek_type origin);
        TOsclFileOffset Tell();
        uint32 Read(OsclAny* aBuffer1, uint32 aDataSize, uint32 aNumElements);
        int32 EndOfFile();
        TOsclFileOffset Size();
        int32 Close();

        uint32 Write(const OsclAny* aBuffer1, uint32 aDataSize, uint32 aNumElements)
        {
            OSCL_UNUSED_ARG(aBuffer1);
            OSCL_UNUSED_ARG(aDataSize);
            OSCL_UNUSED_ARG(aNumElements);
            return 0;//not supported
        }
        uint32 Flush()
        {
            return ((uint32) - 1);//not supported
        }

    private:
        OsclAsyncFile(OsclNativeFile& aAsyncFile, int32 aCacheSize, PVLogger*);
        /**
        * Second-phase constructor.
        */
        void ConstructL();

    private:
        // private utility methods
        void StartAsyncRead(bool aStartAsyncRead);
        bool FindDataBuffer(OsclAsyncFileBuffer*& aDataBuffer, int32& aBufferId, TOsclFileOffset aOffset, int32 aSize);
        void UpdateReading();
        int32 BytesReadAhead();
        int32 SortDataBuffers();
        bool GetNextDataBuffer(OsclAsyncFileBuffer*& aDataBuffer, TOsclFileOffset aFilePointerToReadFrom);
        void StartNextRead(TOsclFileOffset aPosToReadFrom);
        void ReOrderBuffersQueue(int32 aFirstBufferId);
        bool IsLinkedDataBuffer(OsclAsyncFileBuffer* aDataBuffer);
        bool CanBeLinked(OsclAsyncFileBuffer* aDataBuffer);
        uint32 doRead(uint8*& aBuffer1, uint32 aDataSize, uint32 aNumElements, TOsclFileOffset aOffset);

    private:
        TOsclFileOffset iFileSize;

        // File object to do async read from
        OsclNativeFile& iNativeFile;

        // File position for async reads.
        TOsclFileOffset iAsyncFilePosition;

        // For verification
        OsclNativeFile* iNativeFileVerify;
        int32 iVerifyCount;

        // Duplicate file handle for sync read
        OsclNativeFile* iNativeFileDuplicate;

        // File position for sync reads.
        TOsclFileOffset iSyncFilePosition;

        // Arrays of data buffers
        Oscl_Vector<OsclAsyncFileBuffer*, OsclMemAllocator> iDataBufferArray;
        Oscl_Vector<OsclAsyncFileBuffer*, OsclMemAllocator> iSortedDataBufferArray;
        Oscl_Vector<OsclAsyncFileBuffer*, OsclMemAllocator> iLinkedDataBufferArray;

        // Local data buffer
        OsclAsyncFileBuffer* iDataBuffer;
        OsclAsyncFileBuffer* iDataBufferInUse;

        // Buffer for synchronous read
        OsclBuf* iSyncBuffer;

        // Initialized in constructor. Determines the size of each data buffer
        uint32 iTotalCacheSize;

        // Logical File Position (as seen by the client of this class)
        TOsclFileOffset iFilePosition;

        // Last offset after a user read operation
        TOsclFileOffset iLastUserFileRead;

        // Start async read enable flag
        bool iStartAsyncRead;

        // Pointer to buffer for asynchronous read
        int32 iReadPtrDummyLen;
        OsclPtr iReadPtr;

        // For profiling
        PVLogger* iLogger;

        ////////////////////////
        // Configuration parameters.
        ////////////////////////

        //   Number of buffers in the linked list
        int32 iKCacheBufferCount ;

        //   This defines the limit on how much data we will
        //   read ahead of the current position
        int32 iKMinBytesReadAhead ;

        //   This defines the size of the individual async read operations.
        int32 iKAsyncReadBufferSize ;

        ////////////////////////
        // The non-native async read implementation.
        ////////////////////////

        // Keeps track of whether we have native async read or not
        bool iHasNativeAsyncRead;

        // Thread control sems
        OsclSemaphore iAsyncReadSem;
        OsclSemaphore iAsyncReadExitSem;

        // To keep track of the tread state:
        // EAsyncReadNotActive - the thread is not created or the thread is stopped
        // EAsyncReadActive    - the thread is running
        enum TAsyncReadThreadState {EAsyncReadNotActive, EAsyncReadActive};
        TAsyncReadThreadState iAsyncReadThreadState;

        // To signal the thread to exit
        bool iAsyncReadThreadExitFlag;

        // Number of bytes read in the last call
        int32 iAsyncReadNumBytes;

        // Thread routine
        void InThread();
        static TOsclThreadFuncRet OSCL_THREAD_DECL iAsyncReadThreadFunc(TOsclThreadFuncArg);

        // Thread start/stop.
        void LaunchAsyncReadThread();
        void StopAsyncReadThread();

        // Request an async read.
        void StartNonNativeAsyncRead();

    public:
        // for test&stat
        uint32 iNumOfRun;
        uint32 iNumOfRunErr;
};

#endif

