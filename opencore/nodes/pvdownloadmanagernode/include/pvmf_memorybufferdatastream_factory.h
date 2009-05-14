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
// only include pvmf_downloadmanager_config.h if CML2 is NOT being used
#ifndef USE_CML2_CONFIG
#ifndef PVMF_DOWNLOADMANAGER_CONFIG_H_INCLUDED
#include "pvmf_downloadmanager_config.h"
#endif
#endif

#if(PVMF_DOWNLOADMANAGER_SUPPORT_PPB)

#ifndef PVMF_MEMORYBUFFERDATASTREAM_FACTORY_H_INCLUDED
#define PVMF_MEMORYBUFFERDATASTREAM_FACTORY_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_QUEUE_H_INCLUDED
#include "oscl_queue.h"
#endif
#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif
#ifndef PVMI_DATA_STREAM_INTERFACE_H_INCLUDED
#include "pvmi_data_stream_interface.h"
#endif
#ifndef PVMF_CPMPLUGIN_ACCESS_INTERFACE_FACTORY_H_INCLUDED
#include "pvmf_cpmplugin_access_interface_factory.h"
#endif
#ifndef PVMI_DATASTREAMUSER_INTERFACE_H_INCLUDED
#include "pvmi_datastreamuser_interface.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef OSCL_MUTEX_H_INCLUDED
#include "oscl_mutex.h"
#endif
#ifndef OSCL_SEMAPHORE_H_INCLUDED
#include "oscl_semaphore.h"
#endif


#define PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS	16

#define PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS	1

#define PV_MBDS_MAX_NUMBER_OF_TOTAL_CONNECTIONS  PV_MB_MAX_NUMBER_OF_WRITE_CONNECTIONS + PV_MB_MAX_NUMBER_OF_READ_CONNECTIONS


#define PV_MBDS_TEMP_CACHE_TRIM_MARGIN_PS					64000
#define PV_MBDS_TEMP_CACHE_TRIM_THRESHOLD_PS(capacity)				(capacity * 2) / 3
// for shoutcast
#define PV_MBDS_TEMP_CACHE_TRIM_MARGIN_SC					4096
#define PV_MBDS_TEMP_CACHE_TRIM_THRESHOLD_SC(capacity)                          capacity / 6

#define NO_LIMIT		0
#define PV_MBDS_PERM_CACHE_SIZE		NO_LIMIT

// how many bytes are we willing to wait for assuming they are coming
// instead of repositioning
// this depends on channel bandwidth and network condition
#define PV_MBDS_BYTES_TO_WAIT         4 * 1024

// In forward repositioning, if the data is going to come in soon,
// which is defined as requested offset minus the download offset (aka current write pointer)
// being less than this threshold, then don't disconnect to send a new GET request.
#define PV_MBDS_FWD_SEEKING_NO_GET_REQUEST_THRESHOLD 64000

typedef enum
{
    MBDS_CACHE_TRIM_NONE,		// invalid node
    MBDS_CACHE_TRIM_HEAD_ONLY,	// trim the beginning of cache only
    MBDS_CACHE_TRIM_TAIL_ONLY,	// trim from the end of cache only
    MBDS_CACHE_TRIM_HEAD_AND_TAIL,	// trim the both ends of cache
    MBDS_CACHE_TRIM_EMPTY,		// empty the cache
} MBDSCacheTrimMode;

typedef enum
{
    MBDS_REPOSITION_EXACT,
    MBDS_REPOSITION_WITH_MARGIN
} MBDSRepositionMode;

typedef enum
{
    MBDS_STREAM_FORMAT_UNKNOWN,
    MBDS_STREAM_FORMAT_PROGRESSIVE_PLAYBACK,
    MBDS_STREAM_FORMAT_SHOUTCAST
} MBDSStreamFormat;

class PVMFMemoryBufferWriteDataStreamImpl;

class PVMFMemoryBufferReadDataStreamImpl;

class PVMFMemoryBufferDataStreamTempCache
{
    public:
        PVMFMemoryBufferDataStreamTempCache();
        ~PVMFMemoryBufferDataStreamTempCache();

        uint32 GetTotalBytes();

        void GetFileOffsets(uint32& aFirstByte, uint32& aLastByte);

        PvmiDataStreamStatus AddEntry(OsclRefCounterMemFrag* aFrag, uint8* aFragPtr, uint32 aFragSize, uint32 aFileOffset);

        uint32 ReadBytes(uint8* aBuffer, uint32 aFirstByte, uint32 aLastByte, uint32& firstEntry);

        bool RemoveFirstEntry(OsclRefCounterMemFrag*& aFrag, uint8*& aFragPtr);

        bool RemoveLastEntry(OsclRefCounterMemFrag*& aFrag, uint8*& aFragPtr);

        void GetFirstEntryInfo(uint32& entryOffset, uint32& entrySize);

        void GetLastEntryInfo(uint32& entryOffset, uint32& entrySize);

        uint32 GetNumEntries();

    private:

        struct MBDSTempCacheEntry
        {
            // mem frag was allocated by protocol engine
            OsclRefCounterMemFrag* frag;
            // mem ptr
            uint8* fragPtr;
            // size of mem frag
            uint32 fragSize;
            // file offset corresponding to the first byte of the mem frag
            uint32 fileOffset;
        };

        // total number of bytes of data in this cache
        uint32 iTotalBytes;
        // file offset of first byte in cache, use iLock
        uint32 iFirstByteFileOffset;
        // file offset of last byte in cache, use iLock
        uint32 iLastByteFileOffset;
        // list of temp cache entries
        Oscl_Vector<MBDSTempCacheEntry*, OsclMemAllocator> iEntries;

        PVLogger* iLogger;
};

class PVMFMemoryBufferDataStreamPermCache
{
    public:
        PVMFMemoryBufferDataStreamPermCache();
        ~PVMFMemoryBufferDataStreamPermCache();

        uint32 GetTotalBytes();

        // these are the offsets of bytes already in the cache
        void GetFileOffsets(uint32& aFirstByte, uint32& aLastByte);

        // this byte range have made persistent, but not all the bytes are there yet
        // use GetFileOffsets to check what bytes are available for reading
        void GetPermOffsets(uint32& aFirstByte, uint32& aLastByte);

        PvmiDataStreamStatus AddEntry(uint8* aBufPtr, uint32 aBufSize, uint8* aFillPtr, uint32 aFirstOffset, uint32 aLastOffset, uint32 aFillOffset, uint32 aFillSize);

        PvmiDataStreamStatus WriteBytes(uint8* aFragPtr, uint32 aFragSize, uint32 aFileOffset);

        uint32 ReadBytes(uint8* aBuffer, uint32 aFirstByte, uint32 aLastByte);

        bool RemoveFirstEntry(uint8*& aFragPtr);

        uint32 GetNumEntries();

        uint32 GetCacheSize();

    private:

        struct MBDSPermCacheEntry
        {
            // mem ptr from malloc, saved for freeing later
            uint8* bufPtr;
            // size of mem from malloc
            uint32 bufSize;
            // mem ptr to the next byte to be written to
            uint8* fillBufPtr;
            // file offset of first byte in buffer
            uint32 firstFileOffset;
            // file offset of last byte in buffer
            uint32 lastFileOffset;
            // file offset of the next byte to be written to
            uint32 fillFileOffset;
            // number of bytes already written to buffer
            uint32 fillSize;
        };

        // total number of bytes of data in this cache
        uint32 iTotalBytes;
        // total number of bytes allocated in this cache
        uint32 iTotalBufferAlloc;
        // file offset of first readable byte in cache
        uint32 iFirstByteFileOffset;
        // file offset of last readable byte in cache
        uint32 iLastByteFileOffset;
        // file offset of first byte to be made persistent
        uint32 iFirstPermByteOffset;
        // file offset of last byte to be made persistent
        uint32 iLastPermByteOffset;
        // list of perm cache entries
        Oscl_Vector<MBDSPermCacheEntry*, OsclMemAllocator> iEntries;

        PVLogger* iLogger;
};

//////////////////////////////////////////////////////////////////////
// PVMFMemoryBufferReadDataStreamFactoryImpl
//////////////////////////////////////////////////////////////////////
class PVMFMemoryBufferReadDataStreamFactoryImpl : public PVMFDataStreamFactory
{
    public:
        OSCL_IMPORT_REF PVMFMemoryBufferReadDataStreamFactoryImpl(PVMFMemoryBufferDataStreamTempCache* aTempCache,
                PVMFMemoryBufferDataStreamPermCache* aPermCache);

        OSCL_IMPORT_REF void SetWriteDataStreamPtr(PVInterface* aWriteDataStream);

        OSCL_IMPORT_REF PVMFStatus QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids);

        OSCL_IMPORT_REF PVInterface* CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid);

        OSCL_IMPORT_REF void DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid, PVInterface* aPtr);

        OSCL_IMPORT_REF void NotifyDownloadComplete();

    private:
        void addRef() {};

        void removeRef() {};

        bool queryInterface(const PVUuid&, PVInterface*&)
        {
            return false;
        };

        PVMFMemoryBufferWriteDataStreamImpl *iWriteDataStream;

        PVMFMemoryBufferDataStreamTempCache* iTempCache;

        PVMFMemoryBufferDataStreamPermCache* iPermCache;

        bool iDownloadComplete;

        Oscl_Vector<PVMFMemoryBufferReadDataStreamImpl*, OsclMemAllocator> iReadStreamVec;
};


//////////////////////////////////////////////////////////////////////
// PVMFMemoryBufferWriteDataStreamFactoryImpl
//////////////////////////////////////////////////////////////////////
class PVMFMemoryBufferWriteDataStreamFactoryImpl : public PVMFDataStreamFactory
{
    public:
        OSCL_IMPORT_REF PVMFMemoryBufferWriteDataStreamFactoryImpl(PVMFMemoryBufferDataStreamTempCache* aTempCache,
                PVMFMemoryBufferDataStreamPermCache* aPermCache, MBDSStreamFormat aStreamFormat, uint32 aTempCacheCapacity);

        OSCL_IMPORT_REF ~PVMFMemoryBufferWriteDataStreamFactoryImpl();

        OSCL_IMPORT_REF PVMFStatus QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids);

        OSCL_IMPORT_REF PVInterface* CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid);

        OSCL_IMPORT_REF void DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid, PVInterface* aPtr);

        OSCL_IMPORT_REF void NotifyDownloadComplete();

    private:
        void addRef() {};

        void removeRef() {};

        bool queryInterface(const PVUuid&, PVInterface*&)
        {
            return false;
        };

        PVMFMemoryBufferWriteDataStreamImpl *iWriteDataStream;

        PVMFMemoryBufferDataStreamTempCache* iTempCache;

        PVMFMemoryBufferDataStreamPermCache* iPermCache;

        bool iDownloadComplete;

        MBDSStreamFormat iStreamFormat;

        uint32 iTempCacheCapacity;
};


//////////////////////////////////////////////////////////////////////
// PVMFMemoryBufferReadDataStreamImpl
//////////////////////////////////////////////////////////////////////
class PVMFMemoryBufferReadDataStreamImpl : public PVMIDataStreamSyncInterface
{
    public:
        OSCL_IMPORT_REF PVMFMemoryBufferReadDataStreamImpl(PVMFMemoryBufferWriteDataStreamImpl* aWriteDataStream,
                PVMFMemoryBufferDataStreamTempCache* aTempCache,
                PVMFMemoryBufferDataStreamPermCache* aPermCache);

        OSCL_IMPORT_REF ~PVMFMemoryBufferReadDataStreamImpl();

        // From PVInterface
        void addRef() {};

        void removeRef() {};

        OSCL_IMPORT_REF bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        OSCL_IMPORT_REF PvmiDataStreamStatus OpenSession(PvmiDataStreamSession& aSessionID,
                PvmiDataStreamMode aMode,
                bool nonblocking = false);

        OSCL_IMPORT_REF PvmiDataStreamStatus CloseSession(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF PvmiDataStreamRandomAccessType QueryRandomAccessCapability();

        OSCL_IMPORT_REF PvmiDataStreamStatus QueryReadCapacity(PvmiDataStreamSession aSessionID,
                uint32& capacity);

        OSCL_IMPORT_REF PvmiDataStreamCommandId RequestReadCapacityNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& observer,
                uint32 capacity,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF PvmiDataStreamStatus QueryWriteCapacity(PvmiDataStreamSession aSessionID,
                uint32& capacity);

        OSCL_IMPORT_REF PvmiDataStreamCommandId RequestWriteCapacityNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& observer,
                uint32 capacity,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF PvmiDataStreamCommandId CancelNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& observer,
                PvmiDataStreamCommandId aID,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF PvmiDataStreamStatus CancelNotificationSync(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF PvmiDataStreamStatus Read(PvmiDataStreamSession aSessionID, uint8* buffer,
                uint32 size, uint32& numelements);

        OSCL_IMPORT_REF PvmiDataStreamStatus Write(PvmiDataStreamSession aSessionID, uint8* buffer,
                uint32 size, uint32& numelements);

        OSCL_IMPORT_REF PvmiDataStreamStatus Write(PvmiDataStreamSession aSessionID, OsclRefCounterMemFrag* frag,
                uint32& aNumElements);

        OSCL_IMPORT_REF PvmiDataStreamStatus Seek(PvmiDataStreamSession aSessionID,
                int32 offset, PvmiDataStreamSeekType origin);

        OSCL_IMPORT_REF uint32 GetCurrentPointerPosition(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF PvmiDataStreamStatus Flush(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF void NotifyDownloadComplete();

        void SetContentLength(uint32 aContentLength)
        {
            OSCL_UNUSED_ARG(aContentLength);
        }

        OSCL_IMPORT_REF uint32 GetContentLength();

        OSCL_IMPORT_REF PvmiDataStreamStatus SetSourceRequestObserver(PvmiDataStreamRequestObserver& aObserver);

        OSCL_IMPORT_REF uint32 QueryBufferingCapacity();

        void SourceRequestCompleted(const PVMFCmdResp& aResponse)
        {
            OSCL_UNUSED_ARG(aResponse);
        }

        OSCL_IMPORT_REF PvmiDataStreamStatus MakePersistent(int32 aOffset, uint32 aSize);

        // This returns the offsets in the temp cache
        OSCL_IMPORT_REF void GetCurrentByteRange(uint32& aCurrentFirstByteOffset, uint32& aCurrentLastByteOffset);

    public:
        bool iDownloadComplete;

    private:
        PVMFMemoryBufferDataStreamTempCache* iTempCache;

        PVMFMemoryBufferDataStreamPermCache* iPermCache;

        PVMFMemoryBufferWriteDataStreamImpl* iWriteDataStream;

        OSCL_wHeapString<OsclMemAllocator> iFileName;

        PvmiDataStreamSession iSessionID;

        PVLogger* iLogger;

        uint32 iFilePtrPos;

        bool iReadSessionOpened;
};


//////////////////////////////////////////////////////////////////////
// PVMFMemoryBufferWriteDataStreamImpl
//////////////////////////////////////////////////////////////////////
class PVMFMemoryBufferWriteDataStreamImpl : public PVMIDataStreamSyncInterface
{
    public:
        OSCL_IMPORT_REF PVMFMemoryBufferWriteDataStreamImpl(PVMFMemoryBufferDataStreamTempCache* aTempCache,
                PVMFMemoryBufferDataStreamPermCache* aPermCache, MBDSStreamFormat aStreamFormat,
                uint32 aTempCacheCapacity);

        OSCL_IMPORT_REF ~PVMFMemoryBufferWriteDataStreamImpl();

        // From PVInterface
        void addRef() {};

        void removeRef() {};

        OSCL_IMPORT_REF bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        OSCL_IMPORT_REF PvmiDataStreamStatus OpenSession(PvmiDataStreamSession& aSessionID,
                PvmiDataStreamMode aMode,
                bool nonblocking = false);

        OSCL_IMPORT_REF PvmiDataStreamStatus CloseSession(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF PvmiDataStreamStatus OpenReadSession(PvmiDataStreamSession& aSessionID,
                PvmiDataStreamMode aMode,
                bool nonblocking = false,
                PVMFMemoryBufferReadDataStreamImpl* aReadDataStream = NULL);

        OSCL_IMPORT_REF PvmiDataStreamRandomAccessType QueryRandomAccessCapability();

        OSCL_IMPORT_REF PvmiDataStreamStatus QueryReadCapacity(PvmiDataStreamSession aSessionID,
                uint32& aCapacity);

        OSCL_IMPORT_REF PvmiDataStreamCommandId RequestReadCapacityNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& aobserver,
                uint32 aCapacity,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF PvmiDataStreamStatus QueryWriteCapacity(PvmiDataStreamSession aSessionID,
                uint32& aCapacity);

        OSCL_IMPORT_REF PvmiDataStreamCommandId RequestWriteCapacityNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& aObserver,
                uint32 aCapacity,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF PvmiDataStreamCommandId CancelNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& aObserver,
                PvmiDataStreamCommandId aID,
                OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PvmiDataStreamStatus CancelNotificationSync(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF PvmiDataStreamStatus Read(PvmiDataStreamSession aSessionID, uint8* buffer,
                uint32 size, uint32& numelements);

        OSCL_IMPORT_REF PvmiDataStreamStatus Write(PvmiDataStreamSession aSessionID, uint8* buffer,
                uint32 size, uint32& numelements);

        OSCL_IMPORT_REF PvmiDataStreamStatus Write(PvmiDataStreamSession aSessionID, OsclRefCounterMemFrag* frag,
                uint32& aNumElements);


        OSCL_IMPORT_REF PvmiDataStreamStatus Seek(PvmiDataStreamSession aSessionID, int32 offset,
                PvmiDataStreamSeekType origin);

        OSCL_IMPORT_REF PvmiDataStreamStatus Reposition(PvmiDataStreamSession aSessionID,
                uint32 aOffset,	MBDSRepositionMode aMode);

        OSCL_IMPORT_REF uint32 GetCurrentPointerPosition(PvmiDataStreamSession aSessionID) ;

        OSCL_IMPORT_REF PvmiDataStreamStatus Flush(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF void NotifyDownloadComplete();

        OSCL_IMPORT_REF void SetContentLength(uint32 aContentLength);

        OSCL_IMPORT_REF uint32 GetContentLength();

        OSCL_IMPORT_REF PvmiDataStreamStatus SetSourceRequestObserver(PvmiDataStreamRequestObserver& aObserver);

        OSCL_IMPORT_REF void SourceRequestCompleted(const PVMFCmdResp& aResponse);

        OSCL_IMPORT_REF PvmiDataStreamStatus MakePersistent(int32 aOffset, uint32 aSize);

        OSCL_IMPORT_REF uint32 QueryBufferingCapacity();

        OSCL_IMPORT_REF PvmiDataStreamStatus SetReadPointerPosition(PvmiDataStreamSession aSessionID, uint32 aFilePosition);


        OSCL_IMPORT_REF PvmiDataStreamStatus SetReadPointerCacheLocation(PvmiDataStreamSession aSessionID, bool aInTempCache);

        OSCL_IMPORT_REF void ManageCache();

        OSCL_IMPORT_REF void TrimTempCache(MBDSCacheTrimMode aTrimMode);

        OSCL_IMPORT_REF void UpdateReadPointersAfterMakePersistent();

        OSCL_IMPORT_REF bool GetPermCachePersistence(uint32& aFirstOffset, uint32& aLastOffset);

        OSCL_IMPORT_REF void SetStreamFormat(MBDSStreamFormat aStreamFormat);

        OSCL_IMPORT_REF void SetTempCacheCapacity(uint32 aCapacity);

        OSCL_IMPORT_REF MBDSStreamFormat GetStreamFormat();

        OSCL_IMPORT_REF uint32 GetTempCacheCapacity();

    public:
        bool iDownloadComplete;

    private:

        OSCL_IMPORT_REF void ManageReadCapacityNotifications();

    private:

        struct ReadCapacityNotificationStruct
        {
            bool iReadStructValid;

            bool iOutstanding;

            PvmiDataStreamSession iReadSessionID;

            PvmiDataStreamObserver *iReadObserver;

            uint32 iFilePosition;

            uint32 iReadCapacity;

            OsclAny* iContextData;

            PVMFCommandId iCommandID;

            PVMFCommandId iCurrentCommandID;
        };


        struct WriteCapacityNotificationStruct
        {
            bool iOutstanding;

            PvmiDataStreamSession iWriteSessionID;

            PvmiDataStreamObserver *iWriteObserver;

            uint32 iFilePosition;

            uint32 iWriteCapacity;

            OsclAny* iContextData;

            PVMFCommandId iCommandID;

            PVMFCommandId iCurrentCommandID;
        };


        struct RepositionRequestStruct
        {
            bool iOutstanding;

            bool iRequestCompleted;

            PvmiDataStreamStatus iSuccess;

            PvmiDataStreamSession iRepositionSessionID;

            uint32 iNewFilePosition;

            bool iFlushCache;
        };

        struct ReadFilePositionStruct
        {
            bool iReadPositionStructValid;

            uint32 iReadFilePtr;

            bool iInTempCache;

            PVMFMemoryBufferReadDataStreamImpl* iReadDataStream;
        };

        PVMFMemoryBufferDataStreamTempCache* iTempCache;

        PVMFMemoryBufferDataStreamPermCache* iPermCache;

        uint32 iNumReadSessions;

        ReadCapacityNotificationStruct iReadNotifications[PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS];

        RepositionRequestStruct iRepositionRequest;

        ReadFilePositionStruct iReadFilePositions[PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS];

        PvmiDataStreamSession iSessionID;

        int32 iFileNumBytes;

        PVLogger* iLogger;

        uint32 iContentLength;

        PvmiDataStreamRequestObserver* iRequestObserver;

        bool iWriteSessionOpened;

        WriteCapacityNotificationStruct iWriteNotification;

        // Tracks current write file pointer position
        uint32 iFilePtrPos;

        bool iThrowAwayData;

        // Tracks the audio/video/text session read pointers
        PvmiDataStreamSession iAVTSessionID[3];
        uint32 iAVTOffsetDelta;

        bool iMadePersistent;

        MBDSStreamFormat iStreamFormat;

        uint32 iTempCacheCapacity;

        uint32 iTempCacheTrimThreshold;

        uint32 iTempCacheTrimMargin;
};


//////////////////////////////////////////////////////////////////////
// PVMFMemoryBufferDataStream
//////////////////////////////////////////////////////////////////////
class PVMFMemoryBufferDataStream
{
    public:
        // in case we would want to pass the constructor an existing cache
        OSCL_IMPORT_REF PVMFMemoryBufferDataStream(PVMFFormatType& aStreamFormat, uint32 aTempCacheCapacity);

        OSCL_IMPORT_REF ~PVMFMemoryBufferDataStream();

        OSCL_IMPORT_REF PVMFDataStreamFactory* GetReadDataStreamFactoryPtr();

        OSCL_IMPORT_REF PVMFDataStreamFactory* GetWriteDataStreamFactoryPtr();

        OSCL_IMPORT_REF void NotifyDownloadComplete();

    private:
        PVMFMemoryBufferReadDataStreamFactoryImpl* iReadDataStreamFactory;

        PVMFMemoryBufferWriteDataStreamFactoryImpl* iWriteDataStreamFactory;

        PVInterface* iWriteDataStream;

        PVMFMemoryBufferDataStreamTempCache* iTemporaryCache;

        PVMFMemoryBufferDataStreamPermCache* iPermanentCache;

        PVLogger* iLogger;
};

#endif // PVMF_MEMORYBUFFERDATASTREAM_FACTORY_H_INCLUDED


#endif//PVMF_DOWNLOADMANAGER_SUPPORT_PPB

