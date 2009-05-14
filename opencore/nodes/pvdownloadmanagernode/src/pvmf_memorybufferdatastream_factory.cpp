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
 * Memory Buffer Data Stream (MBDS) class will act as the data pipe between the
 * protocol engine node and the file format parsing node.
 * This will write  the data from the protocol engine in a memoery for later reading by the
 * file format parsing node.
 */

#ifndef OSCL_EXCEPTION_H_INCLUDED
#include "oscl_exception.h"
#endif
#ifndef PVMF_MEMORYBUFFERDATASTREAM_FACTORY_H_INCLUDED
#include "pvmf_memorybufferdatastream_factory.h"
#endif
#ifndef OSCL_TICKCOUNT_H_INCLUDED
#include "oscl_tickcount.h"
#endif

// Logging #define
#define LOGDEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_VERBOSE, m);
#define LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, m);
#define LOGTRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, m);

//////////////////////////////////////////////////////////////////////
// PVMFMemoryBufferReadDataStreamFactoryImpl
//////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF
PVMFMemoryBufferReadDataStreamFactoryImpl::PVMFMemoryBufferReadDataStreamFactoryImpl(PVMFMemoryBufferDataStreamTempCache* aTempCache,
        PVMFMemoryBufferDataStreamPermCache* aPermCache)
{
    // store the cache pointer
    iTempCache = aTempCache;
    iPermCache = aPermCache;
    iDownloadComplete = false;
}

OSCL_EXPORT_REF void
PVMFMemoryBufferReadDataStreamFactoryImpl::SetWriteDataStreamPtr(PVInterface* aWriteDataStream)
{
    // Called by PVMFMemoryBufferDataStream constructor
    if (aWriteDataStream)
    {
        iWriteDataStream = OSCL_STATIC_CAST(PVMFMemoryBufferWriteDataStreamImpl*, aWriteDataStream);
    }
}

OSCL_EXPORT_REF PVMFStatus
PVMFMemoryBufferReadDataStreamFactoryImpl::QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids)
{
    aUuids.push_back(PVMIDataStreamSyncInterfaceUuid);
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVInterface*
PVMFMemoryBufferReadDataStreamFactoryImpl::CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid)
{
    // Create a new PVMFMemoryBufferReadDataStreamFactoryImpl for each request
    // Can have up to PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS connections
    if (aUuid == PVMIDataStreamSyncInterfaceUuid)
    {
        PVMFMemoryBufferReadDataStreamImpl* ReadStream = NULL;
        ReadStream = OSCL_NEW(PVMFMemoryBufferReadDataStreamImpl, (iWriteDataStream, iTempCache, iPermCache));
        if (ReadStream == NULL)
        {
            OSCL_LEAVE(OsclErrNoMemory);
        }
        ReadStream->iDownloadComplete = iDownloadComplete;
        // save this connection in list
        iReadStreamVec.push_back(ReadStream);
        return OSCL_STATIC_CAST(PVInterface*, ReadStream);
    }
    return NULL;
}

OSCL_EXPORT_REF void
PVMFMemoryBufferReadDataStreamFactoryImpl::DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid,
        PVInterface* aPtr)
{
    // Destroy the incoming object only if it uses the right UUID and has a vaild pointer
    if ((aUuid == PVMIDataStreamSyncInterfaceUuid) && (aPtr))
    {
        // Cast the incoming ptr to the correct type, then delete
        PVMFMemoryBufferReadDataStreamImpl* iReadStream = NULL;
        iReadStream = OSCL_STATIC_CAST(PVMFMemoryBufferReadDataStreamImpl*, aPtr);
        // Remove from list
        Oscl_Vector<PVMFMemoryBufferReadDataStreamImpl*, OsclMemAllocator>::iterator it;
        it = iReadStreamVec.begin();
        while (it != iReadStreamVec.end())
        {
            if (*it == aPtr)
            {
                iReadStreamVec.erase(it);
                break;
            }
            else
            {
                it++;
            }
        }
        OSCL_DELETE(iReadStream);
    }
}

OSCL_EXPORT_REF void
PVMFMemoryBufferReadDataStreamFactoryImpl::NotifyDownloadComplete()
{
    iDownloadComplete = true;

    Oscl_Vector<PVMFMemoryBufferReadDataStreamImpl*, OsclMemAllocator>::iterator it;
    // Notify all read connections
    for (it = iReadStreamVec.begin(); it != iReadStreamVec.end(); it++)
    {
        (*it)->NotifyDownloadComplete();
    }
}


//////////////////////////////////////////////////////////////////////
// PVMFMemoryBufferWriteDataStreamFactoryImpl
//////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF
PVMFMemoryBufferWriteDataStreamFactoryImpl::PVMFMemoryBufferWriteDataStreamFactoryImpl(PVMFMemoryBufferDataStreamTempCache* aTempCache,
        PVMFMemoryBufferDataStreamPermCache* aPermCache, MBDSStreamFormat aStreamFormat, uint32 aTempCacheCapacity)
{
    // Init to NULL for later creation in CreatePVMFCPMPluginAccessInterface()
    iWriteDataStream = NULL;
    // store the cache pointer
    iTempCache = aTempCache;
    iPermCache = aPermCache;
    iDownloadComplete = false;
    iStreamFormat = aStreamFormat;
    iTempCacheCapacity = aTempCacheCapacity;
}

OSCL_EXPORT_REF
PVMFMemoryBufferWriteDataStreamFactoryImpl::~PVMFMemoryBufferWriteDataStreamFactoryImpl()
{
    if (iWriteDataStream)
    {
        OSCL_DELETE(iWriteDataStream);
    }
}

OSCL_EXPORT_REF PVMFStatus
PVMFMemoryBufferWriteDataStreamFactoryImpl::QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids)
{
    aUuids.push_back(PVMIDataStreamSyncInterfaceUuid);
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVInterface*
PVMFMemoryBufferWriteDataStreamFactoryImpl::CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid)
{
    if (aUuid == PVMIDataStreamSyncInterfaceUuid)
    {
        // iWriteDataStream should have only one instance.
        if (!iWriteDataStream)
        {
            // It does not exist so allocate
            iWriteDataStream = OSCL_NEW(PVMFMemoryBufferWriteDataStreamImpl, (iTempCache, iPermCache, iStreamFormat, iTempCacheCapacity));
            if (iWriteDataStream == NULL)
            {
                OSCL_LEAVE(OsclErrNoMemory);
            }
        }
        // Return the ptr to the iWriteDataStream
        return OSCL_STATIC_CAST(PVInterface*, iWriteDataStream);
    }
    return NULL;
}

OSCL_EXPORT_REF void
PVMFMemoryBufferWriteDataStreamFactoryImpl::DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid,
        PVInterface* aPtr)
{
    // Do nothing
    OSCL_UNUSED_ARG(aUuid);
    OSCL_UNUSED_ARG(aPtr);
}

OSCL_EXPORT_REF void
PVMFMemoryBufferWriteDataStreamFactoryImpl::NotifyDownloadComplete()
{
    iDownloadComplete = true;
    iWriteDataStream->NotifyDownloadComplete();
}

//////////////////////////////////////////////////////////////////////
// PVMFMemoryBufferReadDataStreamImpl
//////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF
PVMFMemoryBufferReadDataStreamImpl::PVMFMemoryBufferReadDataStreamImpl(PVMFMemoryBufferWriteDataStreamImpl* aWriteDataStream,
        PVMFMemoryBufferDataStreamTempCache* aTempCache,
        PVMFMemoryBufferDataStreamPermCache* aPermCache)
{
    iDownloadComplete = false;
    iWriteDataStream = aWriteDataStream;
    iSessionID = 0;
    iFilePtrPos = 0;
    iReadSessionOpened = false;

    // save the pointer to the cache
    iTempCache = aTempCache;
    iPermCache = aPermCache;

    iLogger = PVLogger::GetLoggerObject("PVMFMemoryBufferDataStream");

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::PVMFMemoryBufferReadDataStreamImpl"));
}


OSCL_EXPORT_REF
PVMFMemoryBufferReadDataStreamImpl::~PVMFMemoryBufferReadDataStreamImpl()
{
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::~PVMFMemoryBufferReadDataStreamImpl"));

    if (iReadSessionOpened)
    {
        // need to close the session
        iWriteDataStream->CloseSession(iSessionID);
    }
    iLogger = NULL;
}


OSCL_EXPORT_REF bool
PVMFMemoryBufferReadDataStreamImpl::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::queryInterface"));

    iface = NULL;
    if (uuid == PVMIDataStreamSyncInterfaceUuid)
    {
        PVMIDataStreamSyncInterface* myInterface = OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return true;
    }
    return false;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferReadDataStreamImpl::OpenSession(PvmiDataStreamSession& aSessionID, PvmiDataStreamMode aMode,
        bool nonblocking)
{
    OSCL_UNUSED_ARG(nonblocking);
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::OpenSession"));

    PvmiDataStreamStatus status = PVDS_SUCCESS;
    // Can only open session once
    if (iReadSessionOpened)
    {
        status = PVDS_INVALID_REQUEST;
    }
    else
    {
        // Only support read-only mode
        if (aMode != PVDS_READ_ONLY)
        {
            status = PVDS_UNSUPPORTED_MODE;
        }
        else
        {
            aSessionID = 0;
            iReadSessionOpened = true;
            // Set current file position pointer to the beginning of the file
            iFilePtrPos = 0;
            // Register with the write data stream
            if (iSessionID == 0)
            {
                // open one now
                status = iWriteDataStream->OpenReadSession(iSessionID, PVDS_READ_ONLY, false, this);
            }
        }
    }

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::OpenSession - returning %d session %d", status, iSessionID));
    return status;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferReadDataStreamImpl::CloseSession(PvmiDataStreamSession aSessionID)
{
    OSCL_UNUSED_ARG(aSessionID);
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::CloseSession session %d", iSessionID));

    PvmiDataStreamStatus status = PVDS_SUCCESS;
    if (!iReadSessionOpened)
    {
        status = PVDS_INVALID_REQUEST;
    }
    else
    {
        iReadSessionOpened = false;

        iWriteDataStream->CloseSession(iSessionID);
    }

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::CloseSession - returning %d", status));
    return status;
}


OSCL_EXPORT_REF PvmiDataStreamRandomAccessType
PVMFMemoryBufferReadDataStreamImpl::QueryRandomAccessCapability()
{
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::QueryRandomAccessCapability"));
    return PVDS_FULL_RANDOM_ACCESS;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferReadDataStreamImpl::QueryReadCapacity(PvmiDataStreamSession aSessionID,
        uint32& aCapacity)
{
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::QueryReadCapacity"));

    PvmiDataStreamStatus status = PVDS_SUCCESS;
    // OpenSession needs to be called first
    if (!iReadSessionOpened)
    {
        status = PVDS_INVALID_REQUEST;
    }
    else
    {
        // Return the number of bytes that is available for reading
        // i.e. from the current file pointer position to the last byte that has been written to the file so far

        // Get the current file position
        uint32 currFilePosition = GetCurrentPointerPosition(aSessionID);
        uint32 lastFilePosition = 0;

        // Determine the file size from write datastream.
        status = iWriteDataStream->QueryReadCapacity(iSessionID, lastFilePosition);
        if (PVDS_SUCCESS == status)
        {
            // Calculate the capacity from these two positions.
            aCapacity = (lastFilePosition - currFilePosition) + 1;
        }
        if (iDownloadComplete == true)
        {
            status = PVDS_END_OF_STREAM;
        }
    }

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::QueryReadCapacity returning %d", status));
    return status;
}


OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFMemoryBufferReadDataStreamImpl::RequestReadCapacityNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& aObserver, uint32 aCapacity,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(aSessionID);
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::RequestReadCapacityNotification"));

    // Check for an open session to write data stream
    // iSession contains the session id returned from opening a read session with the wrtie data stream
    if (iSessionID == 0)
    {
        // open one now
        PvmiDataStreamStatus result = iWriteDataStream->OpenSession(iSessionID, PVDS_READ_ONLY, false);
        if (result != PVDS_SUCCESS)
        {
            // No READ sessions left
            OSCL_LEAVE(OsclErrNoResources);
        }
    }
    // Trap the error from the RequestReadCapacityNotification
    PvmiDataStreamCommandId commandID = 0;
    int32 error = 0;
    OSCL_TRY(error, commandID = iWriteDataStream->RequestReadCapacityNotification(iSessionID, aObserver,
                                aCapacity, aContextData));
    if (error)
    {
        OSCL_LEAVE(error);
    }

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::RequestReadCapacityNotification returning %d", commandID));
    return commandID;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferReadDataStreamImpl::QueryWriteCapacity(PvmiDataStreamSession aSessionID, uint32& aCapacity)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(aCapacity);
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::QueryWriteCapacity"));

    // no writing in a read stream
    return PVDS_NOT_SUPPORTED;
}


OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFMemoryBufferReadDataStreamImpl::RequestWriteCapacityNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& aObserver,
        uint32 aCapacity, OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(aObserver);
    OSCL_UNUSED_ARG(aCapacity);
    OSCL_UNUSED_ARG(aContextData);
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::RequestWriteCapacityNotification"));

    // no writing in a read stream
    OSCL_LEAVE(OsclErrNotSupported);
    // to satisfy the compiler
    return 0;
}


OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFMemoryBufferReadDataStreamImpl::CancelNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& aObserver,
        PvmiDataStreamCommandId aID,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(aObserver);
    OSCL_UNUSED_ARG(aID);
    OSCL_UNUSED_ARG(aContextData);
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::CancelNotification"));

    // asynch version is not supported
    OSCL_LEAVE(OsclErrNotSupported);
    // satisfy compiler
    return 0;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferReadDataStreamImpl::CancelNotificationSync(PvmiDataStreamSession aSessionID)
{
    OSCL_UNUSED_ARG(aSessionID);
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::CancelNotificationSync"));

    return iWriteDataStream->CancelNotificationSync(iSessionID);
}

// The perm cache and the temp cache are treated as separate entities
// Always look in the perm cache first, if not in perm cache, then look in the temp cache
// However, if only a part of the data is in one cache, we do not expect to look into the other cache for the rest
OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferReadDataStreamImpl::Read(PvmiDataStreamSession aSessionID, uint8* aBuffer,
        uint32 aSize, uint32& aNumElements)
{
    OSCL_UNUSED_ARG(aSessionID);
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::Read session %d offset %d size %d", iSessionID, iFilePtrPos, aSize * aNumElements));

    // OpenSession needs to be called first
    if (!iReadSessionOpened)
    {
        return PVDS_INVALID_REQUEST;
    }

    // Reading from the current file pointer position
    uint32 bytesToRead = aSize * aNumElements;

    // Check the content range in cache
    uint32 firstByteToRead = iFilePtrPos;
    uint32 lastByteToRead = iFilePtrPos + bytesToRead - 1;

    uint32 numPermEntries = iPermCache->GetNumEntries();
    uint32 firstPermByteOffset = 0;
    uint32 lastPermByteOffset = 0;
    iPermCache->GetFileOffsets(firstPermByteOffset, lastPermByteOffset);

    uint32 numTempEntries = iTempCache->GetNumEntries();
    uint32 firstTempByteOffset = 0;
    uint32 lastTempByteOffset = 0;
    iTempCache->GetFileOffsets(firstTempByteOffset, lastTempByteOffset);

    // for debugging only
    //LOGERROR((0, "PVMFMemoryBufferReadDataStreamImpl::Read session %d offset %d size %d firstTempByteOffset %d lastTempByteOffset %d",
    //	iSessionID, iFilePtrPos, aSize * aNumElements, firstTempByteOffset, lastTempByteOffset));

    uint32 bytesRead = 0;
    uint32 firstEntry = 0;

    bool inTempCache = true;

    // Look at the perm cache first
    if ((0 == numPermEntries) || ((firstByteToRead < firstPermByteOffset) || (firstByteToRead >= lastPermByteOffset)))
    {
        // not in perm cache, look in temp cache
        if ((0 == numTempEntries) || ((firstByteToRead < firstTempByteOffset) || (firstByteToRead > lastTempByteOffset)))
        {
            // First byte not in the temp cache
            // Find out if it is on route to the cache, if so, no need to send reposition request
            // But if the cache is full, we need to send reposition request
            if ((firstByteToRead < firstTempByteOffset) || ((firstByteToRead - lastTempByteOffset) > PV_MBDS_BYTES_TO_WAIT) ||
                    (((firstByteToRead - lastTempByteOffset) <= PV_MBDS_BYTES_TO_WAIT) && ((lastTempByteOffset - firstTempByteOffset + 1) >= iWriteDataStream->GetTempCacheCapacity())))
            {
                LOGDEBUG((0, "PVMFMemoryBufferReadDataStreamImpl::Read Reposition first %d last %d session %d offset %d",
                          firstTempByteOffset, lastTempByteOffset, iSessionID, firstByteToRead));

                // Send a reposition request to the writer
                // Cache should be trimmed to some degree to free buffers for new connection, etc
                // Return a read failure to reader
                PvmiDataStreamStatus status = iWriteDataStream->Reposition(iSessionID, firstByteToRead, MBDS_REPOSITION_WITH_MARGIN);
                if (PVDS_SUCCESS == status)
                {
                    iWriteDataStream->TrimTempCache(MBDS_CACHE_TRIM_HEAD_AND_TAIL);
                }
            }
            // Fail this read
            aNumElements = 0;
            return PVDS_FAILURE;
        }
        // First byte is in temp cache
        if (lastTempByteOffset < lastByteToRead)
        {
            // Not all the bytes are in the temp cache, copy what is there
            bytesRead = iTempCache->ReadBytes(aBuffer, firstByteToRead, lastTempByteOffset, firstEntry);
        }
        else
        {
            // All the bytes are in the cache
            bytesRead = iTempCache->ReadBytes(aBuffer, firstByteToRead, lastByteToRead, firstEntry);
        }
    }
    else
    {
        inTempCache = false;
        // At least part of the data is in the perm cache
        if (lastPermByteOffset < lastByteToRead)
        {
            // Not all the bytes are in the perm cache, copy what is there
            bytesRead = iPermCache->ReadBytes(aBuffer, firstByteToRead, lastPermByteOffset);
        }
        else
        {
            // All the bytes are in the perm cache
            bytesRead = iPermCache->ReadBytes(aBuffer, firstByteToRead, lastByteToRead);
        }
    }

    iFilePtrPos += bytesRead;
    aNumElements = bytesRead / aSize;

    iWriteDataStream->SetReadPointerCacheLocation(iSessionID, inTempCache);
    iWriteDataStream->SetReadPointerPosition(iSessionID, iFilePtrPos);

    if (0 != bytesRead && 0 != firstEntry)
    {
        // there may be some entries in the cache that can be released
        iWriteDataStream->ManageCache();
    }
    if (bytesToRead != bytesRead)
    {
        LOGDEBUG((0, "***** bytesToRead %d != bytesRead %d", bytesToRead, bytesRead));
    }

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::Read session %d returning %d", iSessionID, aNumElements));
    return PVDS_SUCCESS;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferReadDataStreamImpl::Write(PvmiDataStreamSession aSessionID, uint8* aBuffer,
        uint32 aSize, uint32& aNumElements)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(aBuffer);
    OSCL_UNUSED_ARG(aSize);
    OSCL_UNUSED_ARG(aNumElements);
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::Write"));

    // Write not supported in the PVMFMemoryBufferReadDataStreamImpl object
    return PVDS_NOT_SUPPORTED;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferReadDataStreamImpl::Write(PvmiDataStreamSession aSessionID, OsclRefCounterMemFrag* aFrag, uint32& aNumElements)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(aFrag);
    OSCL_UNUSED_ARG(aNumElements);
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::Write"));

    // Write not supported in the PVMFMemoryBufferReadDataStreamImpl object
    return PVDS_NOT_SUPPORTED;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferReadDataStreamImpl::Seek(PvmiDataStreamSession aSessionID, int32 aOffset, PvmiDataStreamSeekType aOrigin)
{
    OSCL_UNUSED_ARG(aSessionID);

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::Seek session %d offset %d origin %d", iSessionID, aOffset, aOrigin));

    PvmiDataStreamStatus status = PVDS_SUCCESS;

    // OpenSession needs to be called first
    if (!iReadSessionOpened)
    {
        status =  PVDS_INVALID_REQUEST;
    }
    else
    {
        // For now, seek changes the read position pointer,
        // but not the content of the cache, i.e. if the read position pointer is outside of the cache
        // repostion request is not issued until a read is done.
        // The downside is that there will be a delay in reading if the pointer is far out.
        // The upside is that if the pointer is slighttly ahead, no reposition request may be needed
        // as more data is coming from the server.
        // If the requested offset is far from the current offset, should trigger a reposition request now.
        // Make sure not seeking beyond end of clip if content length is known

        uint32 contentLength = iWriteDataStream->GetContentLength();
        bool skip = false;
        uint32 skipTo = 0;

        switch (aOrigin)
        {
            case PVDS_SEEK_SET:
                if ((0 != contentLength) && ((uint32)aOffset >= contentLength))
                {
                    status = PVDS_FAILURE;
                }
                else
                {
                    iFilePtrPos = aOffset;
                }
                break;

            case PVDS_SEEK_CUR:
                if ((0 != contentLength) && ((iFilePtrPos + aOffset) >= contentLength))
                {
                    status = PVDS_FAILURE;
                }
                else
                {
                    iFilePtrPos += aOffset;
                }
                break;

            case PVDS_SEEK_END:
                if ((0 == contentLength) || (contentLength <= (uint32)aOffset))
                {
                    // contentLength is not known, fail the seek
                    // if known, do not seek beyond beginning of clip
                    status = PVDS_FAILURE;
                }
                else
                {
                    iFilePtrPos = contentLength - aOffset - 1;
                }
                break;

            case PVDS_SKIP_SET:
                if ((0 != contentLength) && ((uint32)aOffset >= contentLength))
                {
                    status = PVDS_FAILURE;
                }
                else
                {
                    skipTo = aOffset;
                    skip = true;
                }
                break;

            case PVDS_SKIP_CUR:
                if ((0 != contentLength) && ((iFilePtrPos + aOffset) >= contentLength))
                {
                    status = PVDS_FAILURE;
                }
                else
                {
                    skipTo = iFilePtrPos + aOffset;
                    skip = true;
                }
                break;

            case PVDS_SKIP_END:
                if ((0 == contentLength) || (contentLength <= (uint32)aOffset))
                {
                    // contentLength is not known, fail the skip
                    // if known, do not skip beyond beginning of clip
                    status = PVDS_FAILURE;
                }
                else
                {
                    skipTo = contentLength - aOffset - 1;
                    skip = true;
                }
                break;

            default:
                status = PVDS_FAILURE;
                break;
        }

        if (PVDS_SUCCESS == status)
        {
            if (false == skip)
            {
                // seek should not change the cache location
                iWriteDataStream->SetReadPointerPosition(iSessionID, iFilePtrPos);
            }
            else
            {
                // If seeking backwards, data will never come with a reposition request
                uint32 firstTempByteOffset = 0;
                uint32 lastTempByteOffset = 0;
                iTempCache->GetFileOffsets(firstTempByteOffset, lastTempByteOffset);

                if ((skipTo >= firstTempByteOffset) &&
                        (lastTempByteOffset + PV_MBDS_FWD_SEEKING_NO_GET_REQUEST_THRESHOLD >= skipTo))
                {
                    // Seeking forward,, eed to see if the data may be coming shortly before sending request
                    // If the temp cache is full, send the request right away
                    uint32 capacity = 0;
                    iWriteDataStream->QueryWriteCapacity(0, capacity);
                    if (capacity > 64000)
                    {
                        // Check if the data for new offset will eventually come and that there is room
                        // in the cache for it. If there is no room and no more reads are issued by the parser,
                        // no buffers will be released and there will be a deadlock
                        if ((lastTempByteOffset + capacity) > (skipTo + PV_MBDS_BYTES_TO_WAIT))
                        {
                            // data is coming, no need to send request
                            LOGDEBUG((0, "PVMFMemoryBufferReadDataStreamImpl::Seek/Skip data is expected shortly session %d offset %d", iSessionID, skipTo));
                            skip = false;
                        }
                    }
                }
                if (skip)
                {
                    LOGDEBUG((0, "PVMFMemoryBufferReadDataStreamImpl::Seek/Skip Reposition session %d offset %d",
                              iSessionID, skipTo));

                    // Send a reposition request to the writer
                    status = iWriteDataStream->Reposition(iSessionID, skipTo, MBDS_REPOSITION_WITH_MARGIN);
                }
            }
        }
    }

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::Seek returning %d", status));
    return status;
}

OSCL_EXPORT_REF uint32
PVMFMemoryBufferReadDataStreamImpl::GetCurrentPointerPosition(PvmiDataStreamSession aSessionID)
{
    OSCL_UNUSED_ARG(aSessionID);

    uint32 pos = 0;

    // OpenSession needs to be called first
    if (iReadSessionOpened)
    {
        pos = iFilePtrPos;
    }

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::GetCurrentPointerPosition returning %d", pos));
    return pos;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferReadDataStreamImpl::Flush(PvmiDataStreamSession aSessionID)
{
    OSCL_UNUSED_ARG(aSessionID);

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::Flush"));

    // Nothing to do
    return PVDS_SUCCESS;
}


OSCL_EXPORT_REF void
PVMFMemoryBufferReadDataStreamImpl::NotifyDownloadComplete()
{
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::NotifyDownloadComplete"));

    iDownloadComplete = true;
}


OSCL_EXPORT_REF uint32
PVMFMemoryBufferReadDataStreamImpl::GetContentLength()
{
    // returning length of media, if known
    uint32 length = 0;
    if (NULL != iWriteDataStream)
    {
        length = iWriteDataStream->GetContentLength();
    }

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::GetContentLength returning %d", length));
    return length;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferReadDataStreamImpl::SetSourceRequestObserver(PvmiDataStreamRequestObserver& aObserver)
{
    OSCL_UNUSED_ARG(aObserver);

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::SetRequestObserver"));

    // not supoprted for read streams
    return PVDS_NOT_SUPPORTED;
}


OSCL_EXPORT_REF uint32
PVMFMemoryBufferReadDataStreamImpl::QueryBufferingCapacity()
{
    // return size of sliding window
    uint32 capacity = 0;
    if (NULL != iWriteDataStream)
    {
        capacity = iWriteDataStream->QueryBufferingCapacity();
    }

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::QueryBufferingCapacity returning %d", capacity));
    return capacity;
}

// The data to be made persistent may be already in the temp cache.
// If so, copy the data from temp cache into perm cache.
// If not, when the data arrives, it is be written directly in the perm cache
//
// All parsers need to call MakePersistent to get the cache trimming,
// with (offset = 0, size = 0) if nothing is to be copied.
//
// there are possible scenarios:
// all the data to be made persistent is in the perm cache,
//   do nothing, return success
// part of the data be made persistent is in the perm cache,
//   only first part of data can be in perm cache,
//      we can only add to cache in contiguous manner now,
//      malloc a buffer and add to cache,
//      return success
//   if second part of data is already in cache
//      we don't allow filling the gap now
//      return failure
// none of the data be made persistent is in the perm cache,
//   most common usage,
//   malloc a buffer and add to cache,
//   return success
//
// all the data to be made persistent is in the temp cache
//   copy data from temp cache into perm cache, release mem frags
// none of the data to be made persistent is in the temp cache
//   just alloc the buffer but copy nothing
// some of the data to be made persistent is in the temp cache
//   make sure there will not a gap in the perm cache
//   if only second part of data is on temp cache,
//      return failure
//
OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferReadDataStreamImpl::MakePersistent(int32 aOffset, uint32 aSize)
{
    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::MakePersistent offset %d size %d", aOffset, aSize));

    // MakePersistent should not be called multiple times with different offsets
    uint32 firstPersistentOffset = 0;
    uint32 lastPeristentOffset = 0;
    bool bMadePersistent = iWriteDataStream->GetPermCachePersistence(firstPersistentOffset, lastPeristentOffset);

    if (bMadePersistent)
    {
        // has already been called, check the offset + size
        // last byte in perm cache is one byte beyond moov atom
        if ((0 == aSize && 0 == firstPersistentOffset && 0 == lastPeristentOffset) ||
                (aOffset == (int32)firstPersistentOffset && (aOffset + aSize) == lastPeristentOffset))
        {
            // same paramerters, it is ok
            LOGDEBUG((0, "PVMFMemoryBufferReadDataStreamImpl::MakePersistent has already been called with same offset and size"));
            return PVDS_SUCCESS;
        }

        // does not support calling this function again with different parameters
        LOGERROR((0, "PVMFMemoryBufferReadDataStreamImpl::MakePersistent has already been called with first offset %d last offset %d",
                  firstPersistentOffset, lastPeristentOffset));
        return PVDS_NOT_SUPPORTED;
    }

    if (0 == aSize)
    {
        // Just info write stream that cache can now be managed
        iWriteDataStream->MakePersistent(aOffset, aSize);
        return PVDS_SUCCESS;
    }
    // check for upper bound, if any
    if (PV_MBDS_PERM_CACHE_SIZE != NO_LIMIT)
    {
        // find out current perm cache size
        // want to know what has been allocated, not what has been filled
        uint32 cacheSize = iPermCache->GetCacheSize();
        if ((cacheSize + aSize) > PV_MBDS_PERM_CACHE_SIZE)
        {
            LOGERROR((0, "PVMFMemoryBufferReadDataStreamImpl::MakePersistent exceed cache size limit cacheSize %d limit %d", cacheSize, PV_MBDS_PERM_CACHE_SIZE));
            return PVDS_FAILURE;
        }
    }

    // find out what is in the temp cache and perm cache
    // currently, the data in the temp cache and the perm cache are sequential,  there are no gaps in the caches
    // however, the caches can easily modified to be permit gaps

    uint32 firstPermByteOffset = 0;
    uint32 lastPermByteOffset = 0;
    iPermCache->GetPermOffsets(firstPermByteOffset, lastPermByteOffset);

    uint32 firstTempByteOffset = 0;
    uint32 lastTempByteOffset = 0;
    iTempCache->GetFileOffsets(firstTempByteOffset, lastTempByteOffset);

    LOGDEBUG((0, "PVMFMemoryBufferReadDataStreamImpl::MakePersistent perm %d %d temp %d %d",
              firstPermByteOffset, lastPermByteOffset, firstTempByteOffset, lastTempByteOffset));

    // last byte in perm cache is one byte beyond moov atom
    uint32 copyFirstByteOffset = aOffset;
    uint32 copyLastByteOffset = aOffset + aSize;

    // check if some or all data is already in the perm cache, if the perm cache is not empty
    uint32 count = iPermCache->GetNumEntries();
    if (count)
    {
        if (copyFirstByteOffset >= firstPermByteOffset)
        {
            if (copyFirstByteOffset <= lastPermByteOffset)
            {
                // the first byte of data is in perm cache, some or all is in perm cache
                if (copyLastByteOffset <= lastPermByteOffset)
                {
                    // all of it is in the perm cache already, do nothing
                    return PVDS_SUCCESS;
                }
                else
                {
                    // first part is in perm cache
                    // find out how much is left
                    copyFirstByteOffset = lastPermByteOffset + 1;
                }
            }
            else
            {
                // first byte is not the perm cache, check if it is immediately after the last byte of the perm cache
                // if not, there will be a gap, return failure
                if (copyFirstByteOffset != lastPermByteOffset + 1)
                {
                    LOGERROR((0, "PVMFMemoryBufferReadDataStreamImpl::MakePersistent FAILED leaving a gap lastPermOffset %d", lastPermByteOffset));
                    return PVDS_FAILURE;
                }
            }
        }
        else
        {
            // first byte of data is before the perm cache, see if the part of the data is in the perm cache
            if (copyLastByteOffset <= lastPermByteOffset)
            {
                // the second part of the data is already in perm cache
                // add the new buffer in the front of the cache
                copyLastByteOffset = firstPermByteOffset - 1;
            }
            else
            {
                // the middle part of the data is in cache!
                // this done be done, but let's fail that for now
                LOGERROR((0, "PVMFMemoryBufferReadDataStreamImpl::MakePersistent FAILED middle part of data in cache already firstPermByteOffset %d lastPermByteOffset %d", firstPermByteOffset, lastPermByteOffset));
                return PVDS_FAILURE;
            }
        }
    }

    // allocate a memory buffer
    uint32 bufSize = copyLastByteOffset - copyFirstByteOffset + 1;
    uint8* memBuf = (uint8*)oscl_malloc(bufSize);
    if (NULL == memBuf)
    {
        LOGERROR((0, "PVMFMemoryBufferReadDataStreamImpl::MakePersistent out of memory"));
        return PVDS_FAILURE;
    }

    PvmiDataStreamStatus status = PVDS_SUCCESS;
    bool releaseMemFrags = false;
    uint32 bytesRead = 0;
    uint32 firstEntry = 0;

    // the data to be made permanent may not be in the temp cache yet
    if ((copyFirstByteOffset > lastTempByteOffset) || (copyLastByteOffset < firstTempByteOffset))
    {
        // data is not in temp cache, just add the cache entry and when the data arrives,
        // the data will be written to cache
        LOGDEBUG((0, "PVMFMemoryBufferReadDataStreamImpl::MakePersistent data not in temp cache yet"));

        status = iPermCache->AddEntry(memBuf, bufSize, memBuf, copyFirstByteOffset, copyLastByteOffset, copyFirstByteOffset, 0);
    }
    else if ((copyFirstByteOffset >= firstTempByteOffset) && (copyLastByteOffset <= lastTempByteOffset))
    {
        // all the data is in the temp cache, copy the data
        bytesRead = iTempCache->ReadBytes(memBuf, copyFirstByteOffset, copyLastByteOffset, firstEntry);
        if (bytesRead != bufSize)
        {
            // something went wrong
            LOGERROR((0, "PVMFMemoryBufferReadDataStreamImpl::MakePersistent read from temp cache failed %d/%d",
                      bytesRead, bufSize));
            status = PVDS_FAILURE;
        }
        else
        {
            // write out to cache
            status = iPermCache->AddEntry(memBuf, bufSize, memBuf + bufSize, copyFirstByteOffset, copyLastByteOffset, copyLastByteOffset + 1, bufSize);
            if (PVDS_SUCCESS == status)
            {
                releaseMemFrags = true;
            }
        }
    }
    else if ((copyFirstByteOffset >= firstTempByteOffset) && (copyFirstByteOffset <= lastTempByteOffset))
    {
        // first part of data is in the temp cache, copy whatever is there
        uint32 copySize = lastTempByteOffset - copyFirstByteOffset + 1;

        bytesRead = iTempCache->ReadBytes(memBuf, copyFirstByteOffset, lastTempByteOffset, firstEntry);
        if (bytesRead != copySize)
        {
            // something went wrong
            LOGERROR((0, "PVMFMemoryBufferReadDataStreamImpl::MakePersistent read from temp cache failed %d/%d",
                      bytesRead, copySize));
            status = PVDS_FAILURE;
        }
        else
        {
            // write out to cache
            status = iPermCache->AddEntry(memBuf, bufSize, memBuf + copySize, copyFirstByteOffset, copyLastByteOffset, copyFirstByteOffset + copySize, copySize);
            if (PVDS_SUCCESS == status)
            {
                releaseMemFrags = true;
            }
        }
    }
    else
    {
        // first part of data is not in temp cache
        // fail this operation and free the mem buf
        status = PVDS_FAILURE;
    }

    if ((PVDS_FAILURE == status) && (memBuf))
    {
        oscl_free(memBuf);
    }
    else if (PVDS_SUCCESS == status)
    {
        LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::MakePersistent succeeded"));

        if ((releaseMemFrags) && (NULL != iWriteDataStream))
        {
            iWriteDataStream->UpdateReadPointersAfterMakePersistent();

            iWriteDataStream->MakePersistent(aOffset, aSize);
            // give a cache a chance to release mem frags
            iWriteDataStream->ManageCache();
        }
    }

    return status;
}


OSCL_EXPORT_REF void
PVMFMemoryBufferReadDataStreamImpl::GetCurrentByteRange(uint32& aCurrentFirstByteOffset, uint32& aCurrentLastByteOffset)
{
    // if the perm cache and the temp cache was contiguous,
    // report the entire range
    // if not, report only the temp cache range
    uint32 firstTempByteOffset = 0;
    uint32 lastTempByteOffset = 0;
    iTempCache->GetFileOffsets(firstTempByteOffset, lastTempByteOffset);

    uint32 firstPermByteOffset = 0;
    uint32 lastPermByteOffset = 0;
    iPermCache->GetFileOffsets(firstPermByteOffset, lastPermByteOffset);

    if (firstTempByteOffset == (lastPermByteOffset + 1))
    {
        aCurrentFirstByteOffset = firstPermByteOffset;
        aCurrentLastByteOffset = lastTempByteOffset;
    }
    else
    {
        aCurrentFirstByteOffset = firstTempByteOffset;
        aCurrentLastByteOffset = lastTempByteOffset;
    }

    LOGTRACE((0, "PVMFMemoryBufferReadDataStreamImpl::GetCurrentByteRange aCurrentFirstByteOffset %d aCurrentLastByteOffset %d", aCurrentFirstByteOffset, aCurrentLastByteOffset));
}


//////////////////////////////////////////////////////////////////////
// PVMFMemoryBufferWriteDataStreamImpl
//////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF
PVMFMemoryBufferWriteDataStreamImpl::PVMFMemoryBufferWriteDataStreamImpl(PVMFMemoryBufferDataStreamTempCache* aTempCache,
        PVMFMemoryBufferDataStreamPermCache* aPermCache, MBDSStreamFormat aStreamFormat, uint32 aTempCacheCapacity)
{
    iDownloadComplete = false;
    iFileNumBytes = 0;
    iSessionID = 0;
    iContentLength = 0;
    iRequestObserver = NULL;
    iNumReadSessions = 0;
    iWriteSessionOpened = false;
    iThrowAwayData = false;
    iFilePtrPos = 0;
    iAVTSessionID[0] = 0;
    iAVTSessionID[1] = 0;
    iAVTSessionID[2] = 0;
    iAVTOffsetDelta = 0;
    iMadePersistent = false;

    // save pointer to cache
    iTempCache = aTempCache;
    iPermCache = aPermCache;
    iStreamFormat = aStreamFormat;
    iTempCacheCapacity = aTempCacheCapacity;

    for (uint32 i = 0; i < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS; i++)
    {
        iReadNotifications[i].iReadStructValid = false;

        iReadFilePositions[i].iReadPositionStructValid = false;
    }

    iRepositionRequest.iOutstanding = false;
    iWriteNotification.iOutstanding = false;
    iLogger = PVLogger::GetLoggerObject("PVMFMemoryBufferDataStream");

    // put this in the header
    if (MBDS_STREAM_FORMAT_SHOUTCAST == iStreamFormat)
    {
        iTempCacheTrimThreshold = PV_MBDS_TEMP_CACHE_TRIM_THRESHOLD_SC(iTempCacheCapacity);
        iTempCacheTrimMargin = PV_MBDS_TEMP_CACHE_TRIM_MARGIN_SC;
    }
    else
    {
        iTempCacheTrimThreshold = PV_MBDS_TEMP_CACHE_TRIM_THRESHOLD_PS(iTempCacheCapacity);
        iTempCacheTrimMargin = PV_MBDS_TEMP_CACHE_TRIM_MARGIN_PS;
    }

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::PVMFMemoryBufferWriteDataStreamImpl stream format %d temp cache size %d trim threshold %d trim margin %d",
              iStreamFormat, iTempCacheCapacity, iTempCacheTrimThreshold, iTempCacheTrimMargin));
}

OSCL_EXPORT_REF
PVMFMemoryBufferWriteDataStreamImpl::~PVMFMemoryBufferWriteDataStreamImpl()
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::~PVMFMemoryBufferWriteDataStreamImpl"));

    // If there are read notifications outstanding, send them
    // If there are reposition request, signal the semaphores
    for (uint32 i = 0; i < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS; i++)
    {
        if ((iReadNotifications[i].iReadStructValid == true) &&
                (iReadNotifications[i].iOutstanding == true) &&
                (iReadNotifications[i].iReadObserver != NULL))
        {
            PvmiDataStreamObserver* observer = iReadNotifications[i].iReadObserver;
            OsclAny* contextData = iReadNotifications[i].iContextData;
            PvmiDataStreamCommandId cmdID = iReadNotifications[i].iCommandID;

            // Form a command response
            PVMFCmdResp resp(cmdID, contextData, PVMFFailure, NULL, NULL);
            // Make the Command Complete notification
            observer->DataStreamCommandCompleted(resp);
        }
    }
    if (iRepositionRequest.iOutstanding == true)
    {
        iRepositionRequest.iOutstanding = false;
        iRepositionRequest.iSuccess = PVDS_FAILURE;
    }
    // If there is an outstanding write notification, send it
    if (iWriteNotification.iOutstanding && (NULL != iWriteNotification.iWriteObserver))
    {
        PvmiDataStreamObserver* observer2 = iWriteNotification.iWriteObserver;
        OsclAny* contextData = iWriteNotification.iContextData;
        PvmiDataStreamCommandId cmdID = iWriteNotification.iCommandID;

        // Form a command response
        PVMFCmdResp resp2(cmdID, contextData, PVMFFailure, NULL, NULL);
        // Make the Command Complete notification
        observer2->DataStreamCommandCompleted(resp2);
    }
    // Clean up the caches
    while (1)
    {
        OsclRefCounterMemFrag* frag;
        uint8* fragPtr;
        bool found = iTempCache->RemoveFirstEntry(frag, fragPtr);
        if (!found)
        {
            // cache should be empty now
            break;
        }
        // return mem frag to stream writer (e.g. protocol engine)
        iRequestObserver->DataStreamRequestSync(0, PVDS_REQUEST_MEM_FRAG_RELEASED, (OsclAny*)frag);
    }
    while (1)
    {
        uint8* memBuf;
        bool found = iPermCache->RemoveFirstEntry(memBuf);
        if (!found)
        {
            // cache should be empty now
            break;
        }
        // free memory buffer
        if (memBuf)
        {
            oscl_free(memBuf);
        }
    }

    iLogger = NULL;
}


OSCL_EXPORT_REF bool
PVMFMemoryBufferWriteDataStreamImpl::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::queryInterface"));

    iface = NULL;
    if (uuid == PVMIDataStreamSyncInterfaceUuid)
    {
        PVMIDataStreamSyncInterface* myInterface = OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, this);

        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return true;
    }
    return false;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::OpenSession(PvmiDataStreamSession& aSessionID,
        PvmiDataStreamMode aMode, bool nonblocking)
{
    OSCL_UNUSED_ARG(nonblocking);

    // There are 2 kinds of sessions:
    // write session (there can only be 1)
    // read only sessions (at most 4, to support read capacity notifications)
    // read only sessions are opened by hte read data stream

    // This function assumes that the WRITE mode will be requested first!

    LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::OpenSession"));

    PvmiDataStreamStatus status = PVDS_SUCCESS;
    switch (aMode)
    {
        case PVDS_READ_ONLY:
            // Check to see if we have free READ connections before setting one.
            if (iNumReadSessions < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS)
            {
                // at least one of them is free
                bool found = false;
                for (int i = 0; i < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS; i++)
                {
                    if (false == iReadFilePositions[i].iReadPositionStructValid)
                    {
                        found = true;

                        iReadNotifications[i].iReadStructValid = true;
                        iReadNotifications[i].iReadSessionID = i + PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS;
                        iReadNotifications[i].iReadObserver = NULL;
                        iReadNotifications[i].iFilePosition = 0;
                        iReadNotifications[i].iReadCapacity = 0;
                        iReadNotifications[i].iContextData = NULL;
                        iReadNotifications[i].iCommandID = 0;
                        iReadNotifications[i].iCurrentCommandID = 0;

                        aSessionID = iReadNotifications[i].iReadSessionID;

                        iReadFilePositions[i].iReadPositionStructValid = true;
                        iReadFilePositions[i].iReadFilePtr = 0;
                        iReadFilePositions[i].iInTempCache = true;
                        iReadFilePositions[i].iReadDataStream = NULL;

                        iNumReadSessions++;

                        break;
                    }
                }
                if (!found)
                {
                    // something went wrong
                    LOGERROR((0, "PVMFMemoryBufferWriteDataStreamImpl::OpenSession SHOULD NOT GET HERE"));
                    status = PVDS_INVALID_REQUEST;
                }
            }
            else
            {
                status = PVDS_INVALID_REQUEST;
            }
            break;

        case PVDS_WRITE_ONLY:
        case PVDS_APPEND:
            // Can only open a write session once
            if (iWriteSessionOpened)
            {
                status = PVDS_INVALID_REQUEST;
            }
            else
            {
                // write-only starts from beginning of cache, at file offset 0
                // append adds to the last byte in cache

                // id 0 is the write session
                aSessionID = 0;

                iWriteSessionOpened = true;

                iFilePtrPos = 0;

                // reset notification
                iWriteNotification.iOutstanding = false;
                iWriteNotification.iWriteObserver = NULL;
                iWriteNotification.iFilePosition = 0;
                iWriteNotification.iWriteCapacity = 0;
                iWriteNotification.iContextData = NULL;
                iWriteNotification.iCommandID = 0;
                iWriteNotification.iCurrentCommandID = 0;
                // TBD, update file offset and byte count if needed

            }
            break;

        default:
            status = PVDS_UNSUPPORTED_MODE;
            break;

    }

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::OpenSession returning %d", status));
    return status;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::OpenReadSession(PvmiDataStreamSession& aSessionID,
        PvmiDataStreamMode aMode, bool nonblocking,
        PVMFMemoryBufferReadDataStreamImpl* aReadDataStream)
{
    PvmiDataStreamStatus status = OpenSession(aSessionID, aMode, nonblocking);
    if ((PVDS_SUCCESS == status) && (PVDS_READ_ONLY == aMode))
    {
        iReadFilePositions[aSessionID - 1].iReadDataStream = aReadDataStream;
    }

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::OpenReadSession returning %d", status));
    return status;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::CloseSession(PvmiDataStreamSession aSessionID)
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::CloseSession"));

    PvmiDataStreamStatus status = PVDS_SUCCESS;
    // Check for the WRITE session
    if (aSessionID == 0)
    {
        // Check to make sure we don't close the write session twice
        if (!iWriteSessionOpened)
        {
            // Either OpenSession was never called or we already called CloseSession
            status =  PVDS_FAILURE;
        }
        else
        {
            iWriteSessionOpened = false;
            iWriteNotification.iOutstanding = false;
            // empty the cache now
            // release memory buffers back to the writer
            TrimTempCache(MBDS_CACHE_TRIM_EMPTY);
        }
    }
    else
    {
        // Close the READ sessions
        if ((aSessionID > (PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS + PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS)) ||
                (iReadFilePositions[aSessionID - PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS].iReadPositionStructValid != true))
        {
            status =  PVDS_INVALID_SESSION;
        }
        else
        {
            // Have a valid READ session so close it by setting the flag to invalid
            PvmiDataStreamSession sessionId = aSessionID - PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS;

            if (sessionId == iRepositionRequest.iRepositionSessionID)
            {
                iRepositionRequest.iOutstanding = false;
            }

            iReadNotifications[sessionId].iReadStructValid = false;
            iReadNotifications[sessionId].iOutstanding = false;

            iReadFilePositions[sessionId].iReadPositionStructValid = false;

            iNumReadSessions--;

            if (sessionId == iAVTSessionID[0])
            {
                iAVTSessionID[0] = iAVTSessionID[1];
                iAVTSessionID[1] = iAVTSessionID[2];
                iAVTSessionID[2] = 0;
            }
            else if (sessionId == iAVTSessionID[1])
            {
                iAVTSessionID[1] = iAVTSessionID[2];
                iAVTSessionID[2] = 0;
            }
            else if (sessionId == iAVTSessionID[2])
            {
                iAVTSessionID[2] = 0;
            }
        }
    }

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::CloseSession returning %d", status));
    return status;
}


OSCL_EXPORT_REF PvmiDataStreamRandomAccessType
PVMFMemoryBufferWriteDataStreamImpl::QueryRandomAccessCapability()
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::QueryRandomAccessCapability"));

    return PVDS_FULL_RANDOM_ACCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::QueryReadCapacity(PvmiDataStreamSession aSessionID, uint32& aCapacity)
{
    OSCL_UNUSED_ARG(aSessionID);

    // return the offset of the last byte in the cache
    uint32 firstTempByteOffset = 0;
    uint32 lastTempByteOffset = 0;
    iTempCache->GetFileOffsets(firstTempByteOffset, lastTempByteOffset);

    uint32 firstPermByteOffset = 0;
    uint32 lastPermByteOffset = 0;
    iPermCache->GetFileOffsets(firstPermByteOffset, lastPermByteOffset);

    // Return the larger of the two offsets, as the temp cache may be trimmed
    iTempCache->GetFileOffsets(firstTempByteOffset, lastTempByteOffset);

    aCapacity = (lastTempByteOffset > lastPermByteOffset) ? lastTempByteOffset : lastPermByteOffset;

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::QueryReadCapacity returning %d", aCapacity));
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFMemoryBufferWriteDataStreamImpl::RequestReadCapacityNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& aObserver,
        uint32 aCapacity, OsclAny* aContextData)
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::RequestReadCapacityNotification"));

    //  Check that aSessionID is valid and is not the WRITE session
    if ((aSessionID == 0) ||
            (aSessionID > (PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS + PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS)) ||
            (iReadNotifications[aSessionID - PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS].iReadStructValid != true))
    {
        OSCL_LEAVE(OsclErrArgument);
    }
    // Read SessionID index is PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS less than the aSessionID passed in
    PvmiDataStreamSession temp_session = aSessionID - PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS;

    // Save the read notification
    iReadNotifications[temp_session].iOutstanding = true;
    iReadNotifications[temp_session].iReadObserver = &aObserver;
    iReadNotifications[temp_session].iFilePosition = iReadFilePositions[temp_session].iReadFilePtr;
    iReadNotifications[temp_session].iReadCapacity = aCapacity;
    iReadNotifications[temp_session].iContextData = aContextData;
    iReadNotifications[temp_session].iCommandID = iReadNotifications[temp_session].iCurrentCommandID++;

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::RequestReadCapacityNotification returning %d", iReadNotifications[temp_session].iCommandID));
    return iReadNotifications[temp_session].iCommandID;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::QueryWriteCapacity(PvmiDataStreamSession aSessionID, uint32& aCapacity)
{
    OSCL_UNUSED_ARG(aSessionID);

    // return the number of bytes left in the sliding window that can be filled
    // only support writing to the temp cache right now
    // the perm cache is filled by copyig from the temp cache
    aCapacity = (iTempCache->GetTotalBytes() >= iTempCacheCapacity) ? 0 : iTempCacheCapacity - (iTempCache->GetTotalBytes());

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::QueryWriteCapacity returning %d", aCapacity));

    return PVDS_SUCCESS;
}


OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFMemoryBufferWriteDataStreamImpl::RequestWriteCapacityNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& aObserver,
        uint32 aCapacity, OsclAny* aContextData)
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::RequestWriteCapacityNotification"));

    // Check that aSessionID is the WRITE session
    if (aSessionID != 0)
    {
        OSCL_LEAVE(OsclErrArgument);
    }
    // Only one notification request can be made
    if (iWriteNotification.iOutstanding)
    {
        OSCL_LEAVE(OsclErrAlreadyExists);
    }
    // Save the write notification
    iWriteNotification.iOutstanding = true;
    iWriteNotification.iWriteObserver = &aObserver;
    iWriteNotification.iFilePosition = GetCurrentPointerPosition(0);
    iWriteNotification.iWriteCapacity = aCapacity;
    iWriteNotification.iContextData = aContextData;
    iWriteNotification.iCommandID = iWriteNotification.iCurrentCommandID++;

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::RequestWriteCapacityNotification returning %d", iWriteNotification.iCommandID));
    return iWriteNotification.iCommandID;
}


OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFMemoryBufferWriteDataStreamImpl::CancelNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& aObserver,
        PvmiDataStreamCommandId aID,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(aObserver);
    OSCL_UNUSED_ARG(aID);
    OSCL_UNUSED_ARG(aContextData);

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::CancelNotification"));

    // asynch version not supported
    OSCL_LEAVE(OsclErrNotSupported);
    // satisfy compiler
    return 0;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::CancelNotificationSync(PvmiDataStreamSession aSessionID)
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::CancelNotificationSync"));

    PvmiDataStreamStatus status = PVDS_SUCCESS;
    if (aSessionID == 0)
    {
        // Cancel write notification
        if (!iWriteNotification.iOutstanding)
        {
            status = PVDS_INVALID_REQUEST;
        }
        else
        {
            // Clean out notification info
            iWriteNotification.iOutstanding = false;
            iWriteNotification.iWriteObserver = NULL;
            iWriteNotification.iFilePosition = 0;
            iWriteNotification.iWriteCapacity = 0;
            iWriteNotification.iContextData = NULL;
            iWriteNotification.iCommandID = 0;
        }
    }
    else if ((aSessionID > (PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS + PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS)) ||
             (iReadNotifications[aSessionID - PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS].iReadStructValid != true))
    {
        // Cancel read notification
        // Check that aSessionID is valid
        status =  PVDS_INVALID_REQUEST;
    }
    else
    {
        // Zero out notification info
        PvmiDataStreamSession temp_sessionID = aSessionID - PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS;
        iReadNotifications[temp_sessionID].iOutstanding = false;
        iReadNotifications[temp_sessionID].iReadObserver = NULL;
        iReadNotifications[temp_sessionID].iFilePosition = 0;
        iReadNotifications[temp_sessionID].iReadCapacity = 0;
        iReadNotifications[temp_sessionID].iContextData = NULL;
        iReadNotifications[temp_sessionID].iCommandID = 0;
    }

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::CancelNotificationSync returning %d", status));
    return status;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::Read(PvmiDataStreamSession aSessionID, uint8* buffer,
        uint32 size, uint32& numelements)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(buffer);
    OSCL_UNUSED_ARG(size);
    OSCL_UNUSED_ARG(numelements);

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::Read"));

    // Can't read from a write session
    return PVDS_NOT_SUPPORTED;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::Write(PvmiDataStreamSession aSessionID, uint8* buffer,
        uint32 size, uint32& numelements)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(buffer);
    OSCL_UNUSED_ARG(size);
    OSCL_UNUSED_ARG(numelements);

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::Write"));

    // Only support mem frags
    return PVDS_NOT_SUPPORTED;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::Write(PvmiDataStreamSession aSessionID, OsclRefCounterMemFrag* aFrag, uint32& aNumElements)
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::Write iFilePtrPos %d", iFilePtrPos));

    PvmiDataStreamStatus status = PVDS_FAILURE;

    // Make sure this is the write session and that the session is open
    if ((aSessionID == 0) && (iWriteSessionOpened))
    {
        if (iThrowAwayData)
        {
            // Reposition in progress, throw away the data
            LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::Write reposition, throw away frag %x", aFrag));
            return PVDS_SUCCESS;
        }
        // check if this is the start of the new data from a repositioning,
        // if so, repositioning is finally done
        if ((true == iRepositionRequest.iOutstanding) && (true == iRepositionRequest.iRequestCompleted))
        {
            iRepositionRequest.iOutstanding = false;
        }
        bool written = false;

        // check if this data is supposed to written to the perm cache if perm cache is not empty
        uint32 firstPermOffset = 0;
        uint32 lastPermOffset = 0;
        iPermCache->GetPermOffsets(firstPermOffset, lastPermOffset);

        uint32 firstPermReadOffset = 0;
        uint32 lastPermReadOffset = 0;
        iPermCache->GetFileOffsets(firstPermReadOffset, lastPermReadOffset);

        uint32 permEntries = iPermCache->GetNumEntries();

        uint32 fragSize = aFrag->getMemFragSize();
        uint8* fragPtr = (uint8*)aFrag->getMemFragPtr();

        LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::Write fragSize %d", fragSize));

        if (permEntries && ((iFilePtrPos >= firstPermOffset) && (iFilePtrPos <= lastPermOffset)))
        {
            // part or all of this mem frag should be copied to the perm cache
            // the cache entry and the mem buffer have already been allocated during MakePersistent()
            // however, there may be data in this frag that has not been made persistent
            // if there is data left in this frag that does not belong in the perm cache,
            // add the entire fragment to the temp cache
            if ((iFilePtrPos + fragSize - 1) > lastPermOffset)
            {
                LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::Write partially to perm cache"));

                // part of frag has been made persistent
                uint32 firstEntrySize = lastPermOffset - iFilePtrPos + 1;
                // copy the first part
                status = iPermCache->WriteBytes(fragPtr, firstEntrySize, iFilePtrPos);
                if (PVDS_SUCCESS != status)
                {
                    LOGERROR((0, "PVMFMemoryBufferWriteDataStreamImpl::Write WriteBytes FAILED"));
                }
                else
                {
                    // this fragment should be the first entry in the temp cache
                    // if not, something went wrong
                    uint32 count = iTempCache->GetNumEntries();
                    if (count)
                    {
                        LOGERROR((0, "PVMFMemoryBufferWriteDataStreamImpl::Write FAILED Temp Cache not empty"));
                        status = PVDS_FAILURE;
                    }
                    else
                    {
                        status = iTempCache->AddEntry(aFrag, fragPtr, fragSize, iFilePtrPos);
                        if (PVDS_SUCCESS == status)
                        {
                            // notify the writer that the buffer should not be freed
                            // as it has become a part of the cache
                            status = PVDS_PENDING;
                        }
                    }
                }
            }
            else
            {
                LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::Write entirely to perm cache"));

                // all of the frag has been made persistent
                status = iPermCache->WriteBytes(fragPtr, fragSize, iFilePtrPos);
            }
            if ((PVDS_SUCCESS == status) || (PVDS_PENDING == status))
            {
                // see if any read notifications can be satisified
                written = true;
            }
        }
        else
        {
            // this mem frag should be added to temp cache
            // If this mem frag does not fit sequentially in the cache, i.e. a reposition has happened
            // The cache needs to be flushed and all the existing entries returned
            uint32 firstByteOffset = 0;
            uint32 lastByteOffset = 0;
            iTempCache->GetFileOffsets(firstByteOffset, lastByteOffset);
            if (iFilePtrPos != (lastByteOffset + 1))
            {
                while (1)
                {
                    OsclRefCounterMemFrag* frag;
                    uint8* ptr;
                    bool found = iTempCache->RemoveFirstEntry(frag, ptr);
                    if (!found)
                    {
                        // cache should be empty now
                        break;
                    }
                    // return mem frag to stream writer (e.g. protocol engine)
                    iRequestObserver->DataStreamRequestSync(0, PVDS_REQUEST_MEM_FRAG_RELEASED, (OsclAny*)frag);
                }
            }
            status = iTempCache->AddEntry(aFrag, fragPtr, fragSize, iFilePtrPos);
            if (PVDS_SUCCESS == status)
            {
                written = true;
                // notify the writer that the buffer should not be freed
                // as it has become a part of the cache
                status = PVDS_PENDING;
            }
            // Check if there are frags that are not being read from
            // at the beginning of the cache that can be returned to the writer (protocol engine)
            ManageCache();
        }
        if (written)
        {
            // advance the write pointer
            iFilePtrPos += fragSize;
            aNumElements = fragSize;

            // Send notification if requested read capacity is available
            ManageReadCapacityNotifications();
        }
    }

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::Write returning %d", status));
    return status;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::Seek(PvmiDataStreamSession aSessionID, int32 aOffset, PvmiDataStreamSeekType aOrigin)
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::Seek offset %d origin %d", aOffset, aOrigin));

    PvmiDataStreamStatus status = PVDS_SUCCESS;
    // Make sure this is the write session and that the session is open
    if ((aSessionID == 0) && (iWriteSessionOpened))
    {
        // Seek just advances the write pointer
        // The content in the cache does not change until the next Write
        // Seek is used to discard the data in the stream
        switch (aOrigin)
        {
            case PVDS_SEEK_SET:
                iFilePtrPos = aOffset;
                break;

            case PVDS_SEEK_CUR:
                iFilePtrPos += aOffset;
                break;

            case PVDS_SEEK_END:
                if (0 == iContentLength)
                {
                    // content length not known, fail the seek
                    status = PVDS_FAILURE;
                }
                else
                {
                    iFilePtrPos = iContentLength - aOffset - 1;
                }
                break;

            default:
                status = PVDS_FAILURE;
                break;
        }
    }

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::Seek returning %d", status));
    return status;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::Reposition(PvmiDataStreamSession aSessionID, uint32 aOffset,
        MBDSRepositionMode aMode)
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::Reposition"));

    uint32 writeCap = 0;
    QueryWriteCapacity(0, writeCap);
    PvmiDataStreamStatus status = PVDS_SUCCESS;

    //  Check that aSessionID is valid and is not the WRITE session
    if ((aSessionID == 0) ||
            (aSessionID > (PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS + PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS)))
    {
        status = PVDS_FAILURE;
        return status;
    }
    else if (NULL == iRequestObserver)
    {
        // Protocol engine is not listening
        status = PVDS_FAILURE;
        return status;
    }
    else if ((aOffset >= iFilePtrPos)
             && ((writeCap != 0) && ((writeCap + iFilePtrPos) > aOffset)))
    {
        // data is on route. Now check whether we should send new GET and flush the old data or just wait
        LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::Reposition data is en route, GET request not sent"));
        // Check for read position pointers in temp cache
        bool found = false;
        uint32 smallest = 0xFFFFFFFF;
        {
            uint32 firstPersistentOffset = 0;
            uint32 lastPersistentOffset = 0;
            iPermCache->GetPermOffsets(firstPersistentOffset, lastPersistentOffset);
            for (int32 i = 0; i < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS; i++)
            {
                if ((iReadFilePositions[i].iReadPositionStructValid == true) && (iReadFilePositions[i].iInTempCache == true))
                {
                    if ((0 == i) && (0 == firstPersistentOffset) && (0 == lastPersistentOffset) && (0 == iReadFilePositions[i].iReadFilePtr))
                    {
                        // nothing made persistent, ignore session 0 ptr at offset 0
                        continue;
                    }
                    found = true;
                    //LOGE("Ln %d smalltest %d iReadFilePositions[%d].iReadFilePtr %d ", __LINE__, smallest , i, iReadFilePositions[i].iReadFilePtr );
                    if (iReadFilePositions[i].iReadFilePtr < smallest)
                    {
                        smallest = iReadFilePositions[i].iReadFilePtr;
                    }
                }
            }
        }
        //LOGE("Ln %d found %d smalltest %d iFilePtrPos %d aOffset %d", __LINE__, found, smallest , iFilePtrPos, aOffset );
        //LOGE("Ln %d tmpCache [%d %d] iFilePtrPos %d", __LINE__, firstTempByteOffset, lastTempByteOffset , iFilePtrPos);
        if (found)
        {
            if (smallest < iFilePtrPos)
            {
                //LOGE("Ln %d Do nothing. found %d smalltest %d", __LINE__, found, smallest );
                return status;
            }

            if ((smallest >= iFilePtrPos) &&
                    (iFilePtrPos + PV_MBDS_FWD_SEEKING_NO_GET_REQUEST_THRESHOLD > smallest) &&
                    ((writeCap != 0) && ((smallest - iFilePtrPos) < writeCap)) &&
                    (iAVTOffsetDelta < iTempCacheCapacity))
            {
                //LOGE("Ln %d Do nothing. found %d smalltest %d", __LINE__, found, smallest );
                return status;
            }
        }
    }

    {
        // Only support one outstanding reposition request, do not send another
        // If this is requesting the same offset as the outstanding request, return success, otherwise failure
        if (true == iRepositionRequest.iOutstanding)
        {
            if (aOffset < iRepositionRequest.iNewFilePosition)
            {
                // Definitely not getting the data
                status = PVDS_FAILURE;
            }
            else
            {
                // TBD
                // may have to wait for a long time
            }
        }
        else
        {
            // This is a non-blocking call, store the request info
            // If mode is MBDS_REPOSITION_EXACT, request the exact offset (e.g. connection to data source
            // was disrupted, want to continue exactly where the last write left off)
            // If mode is MBDS_REPOSITION_WITH_MARGIN, subtract a 64000 bytes from the offset,
            // this creates a comfortable margin (e.g. video seeks to 100000, if reposition exactly to 100000
            // and then audio then seeks to 90000, another reposition will be needed, not good)
            // However, if MakePersistent has been called and the perm cache is not empty,
            // do not try to re-fill the perm cache.

            if (MBDS_REPOSITION_EXACT == aMode)
            {
                // Save the requested offset
                iRepositionRequest.iNewFilePosition = aOffset;

                // When new data comes, only trim from the top
                iRepositionRequest.iFlushCache = false;
            }
            else if (MBDS_REPOSITION_WITH_MARGIN == aMode)
            {
                // check perm cache status
                bool hasPerm = false;
                if (iMadePersistent)
                {
                    if (0 != iPermCache->GetNumEntries())
                    {
                        hasPerm = true;
                        uint32 firstPermByteOffset = 0;
                        uint32 lastPermByteOffset = 0;
                        iPermCache->GetFileOffsets(firstPermByteOffset, lastPermByteOffset);

                        // should never reposition outside of the temp cache
                        if (aOffset >= lastPermByteOffset + PV_MBDS_TEMP_CACHE_TRIM_MARGIN_PS)
                        {
                            iRepositionRequest.iNewFilePosition = aOffset - PV_MBDS_TEMP_CACHE_TRIM_MARGIN_PS;
                        }
                        else
                        {
                            iRepositionRequest.iNewFilePosition = lastPermByteOffset;
                        }
                    }
                }

                if (!hasPerm)
                {
                    // add the margin
                    if (aOffset > PV_MBDS_TEMP_CACHE_TRIM_MARGIN_PS)
                    {
                        iRepositionRequest.iNewFilePosition = aOffset - PV_MBDS_TEMP_CACHE_TRIM_MARGIN_PS;
                    }
                    else
                    {
                        iRepositionRequest.iNewFilePosition = 0;
                    }
                }

                // Set the read pointer to the request offset,
                // so that the new data will be kept in the cache
                // and not get thrown out because there is no pointer to it
                // Reposition should never change the cache location
                SetReadPointerPosition(aSessionID, iRepositionRequest.iNewFilePosition);

                // When new data comes, the cache can be flushed
                iRepositionRequest.iFlushCache = true;
            }
            else
            {
                status = PVDS_UNSUPPORTED_MODE;

                LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::Reposition failed returning %d", status));
                return status;
            }

            // Send reposition request to the Protocol Engine, which should just queue the request
            // Write session id is 0
            iRepositionRequest.iOutstanding = true;
            iRepositionRequest.iRequestCompleted = false;
            iRepositionRequest.iSuccess = PVDS_PENDING;

            // Read SessionID index is PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS less than the aSessionID passed in
            iRepositionRequest.iRepositionSessionID = aSessionID - PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS;

            PvmiDataStreamCommandId cmdId = 0;
            int32 error = 0;
            OSCL_TRY(error, cmdId = iRequestObserver->DataStreamRequest(0, PVDS_REQUEST_REPOSITION, (OsclAny*)iRepositionRequest.iNewFilePosition, (OsclAny*) & iRepositionRequest));
            if (error)
            {
                status = PVDS_FAILURE;
            }
            if (PVDS_SUCCESS == status)
            {
                // set a flag to throw away any write data
                // and trim the cache, to release buffers back to the writer
                iThrowAwayData = true;

                // new download session is commencing
                iDownloadComplete = false;
            }
        }
    }

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::Reposition returning %d", status));
    return status;
}


OSCL_EXPORT_REF uint32
PVMFMemoryBufferWriteDataStreamImpl::GetCurrentPointerPosition(PvmiDataStreamSession aSessionID)
{
    uint32 pos = 0;
    // Return the write position pointer
    // Make sure this is the write session and that the session is open
    if ((aSessionID == 0) && (iWriteSessionOpened))
    {
        pos = iFilePtrPos;
    }

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::GetCurrentPointerPosition returning %d", pos));
    return pos;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::Flush(PvmiDataStreamSession aSessionID)
{
    PvmiDataStreamStatus status = PVDS_SUCCESS;
    // Make sure this is the write session and that the session is open
    if ((aSessionID != 0) || (!iWriteSessionOpened))
    {
        status = PVDS_FAILURE;
    }
    else
    {
        // Empty cache and return mem buffers to writer
        // It doesn't matter if there are read sessions still open
        // subsequent reads will fail
        for (int32 i = 0; i < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS; i++)
        {
            if (true == iReadFilePositions[i].iReadPositionStructValid)
            {
                // write session id is 0 and read session id's are 1 and up
                LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::Flush read session %d still open!!", i + 1));
            }
        }
        TrimTempCache(MBDS_CACHE_TRIM_EMPTY);
    }

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::Flush returning %d", status));
    return status;
}

OSCL_EXPORT_REF void
PVMFMemoryBufferWriteDataStreamImpl::NotifyDownloadComplete()
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::NotifyDownloadComplete"));

    iDownloadComplete = true;

    ManageReadCapacityNotifications();
}


OSCL_EXPORT_REF void
PVMFMemoryBufferWriteDataStreamImpl::SetContentLength(uint32 aContentLength)
{
    iContentLength = aContentLength;

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::SetContentLength %d", iContentLength));
}


OSCL_EXPORT_REF uint32
PVMFMemoryBufferWriteDataStreamImpl::GetContentLength()
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::GetContentLength returning %d", iContentLength));

    return iContentLength;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::SetSourceRequestObserver(PvmiDataStreamRequestObserver& aObserver)
{
    PvmiDataStreamStatus status = PVDS_SUCCESS;
    // only one observer per write stream
    if (NULL != iRequestObserver)
    {
        status = PVDS_FAILURE;
    }
    else
    {
        iRequestObserver = &aObserver;
    }
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::SetSourceRequestObserver returning %d", status));

    return status;
}


OSCL_EXPORT_REF void
PVMFMemoryBufferWriteDataStreamImpl::SourceRequestCompleted(const PVMFCmdResp& aResponse)
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::SourceRequestCompleted"));

    // Currently, we only have reposition requests, if we have other kinds later on, CmdId will become important

    // request completely, but still needs to wait for the data to show up
    RepositionRequestStruct* reqStruct = (RepositionRequestStruct*)aResponse.GetContext();
    reqStruct->iRequestCompleted = true;
    if (aResponse.GetCmdStatus() != PVMFSuccess)
    {
        // do nothing now, log error
        reqStruct->iSuccess = PVDS_FAILURE;
        LOGERROR((0, "PVMFMemoryBufferWriteDataStreamImpl::SourceRequestCompleted Reposition failed"));
    }
    else
    {
        // Success does not mean that all the data is in the cache,
        // it only means that the networks has responded with the data
        reqStruct->iSuccess = PVDS_SUCCESS;
        iThrowAwayData = false;
        iFilePtrPos = reqStruct->iNewFilePosition;

        if (reqStruct->iFlushCache)
        {
            TrimTempCache(MBDS_CACHE_TRIM_EMPTY);
        }
    }

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::SourceRequestCompleted cmdId %d status %d", aResponse.GetCmdId(), aResponse.GetCmdStatus()));
}


OSCL_EXPORT_REF uint32
PVMFMemoryBufferWriteDataStreamImpl::QueryBufferingCapacity()
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::QueryBufferingCapacity returning %d", iTempCacheCapacity));

    // return the minimum size of the cache/sliding window
    return iTempCacheCapacity;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::MakePersistent(int32 aOffset, uint32 aSize)
{
    OSCL_UNUSED_ARG(aOffset);
    OSCL_UNUSED_ARG(aSize);

    iMadePersistent = true;
    return PVDS_SUCCESS;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::SetReadPointerPosition(PvmiDataStreamSession aSessionID, uint32 aFilePosition)
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::SetReadPointerPosition session %d pos %d", aSessionID, aFilePosition));

    if (iMadePersistent && ((0 == iAVTSessionID[0]) || (0 == iAVTSessionID[1]) || (0 == iAVTSessionID[2])))
    {
        // go through the temp cache looking for the id's of the audio/video/text sessions
        PvmiDataStreamSession avtFirst = 0, avtSecond = 0, avtThird = 0;
        for (int32 i = 0; i < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS; i++)
        {
            if ((iReadFilePositions[i].iReadPositionStructValid == true) && (iReadFilePositions[i].iInTempCache == true))
            {
                if (0 == avtFirst)
                {
                    avtFirst = i;
                }
                else if (0 == avtSecond)
                {
                    avtSecond = i;
                }
                else  if (0 == avtThird)
                {
                    avtThird = i;
                }
            }
        }
        if ((0 != avtFirst) && (0 != avtSecond))
        {
            iAVTSessionID[0] = avtFirst;
            iAVTSessionID[1] = avtSecond;
            if (0 != avtThird)
            {
                iAVTSessionID[2] = avtThird;
            }
        }
    }

    PvmiDataStreamStatus status = PVDS_SUCCESS;
    if ((aSessionID == 0) ||
            (aSessionID > (PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS + PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS)) ||
            (iReadFilePositions[aSessionID - PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS].iReadPositionStructValid != true))
    {
        LOGERROR((0, "PVMFMemoryBufferWriteDataStreamImpl::SetReadPointerPosition invalid session %d", aSessionID));
        status = PVDS_FAILURE;
    }
    else
    {
        PvmiDataStreamSession index = aSessionID - PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS;
        iReadFilePositions[index].iReadFilePtr = aFilePosition;

        // Once MakePersistent has been called
        // need to keep track of the av sample offsets in the temp cache, only worry if we have more than 1 avt sessions
        if ((true == iReadFilePositions[index].iInTempCache) && (0 != iAVTSessionID[0] && (0 != iAVTSessionID[1])))
        {
            if ((index == iAVTSessionID[0]) || (index == iAVTSessionID[1]) || (index == iAVTSessionID[2]))
            {
                // one of the avt file pointers have changed, need to find the largest delta
                if (iReadFilePositions[iAVTSessionID[0]].iReadFilePtr > iReadFilePositions[iAVTSessionID[1]].iReadFilePtr)
                {
                    iAVTOffsetDelta = iReadFilePositions[iAVTSessionID[0]].iReadFilePtr - iReadFilePositions[iAVTSessionID[1]].iReadFilePtr;
                }
                else
                {
                    iAVTOffsetDelta = iReadFilePositions[iAVTSessionID[1]].iReadFilePtr - iReadFilePositions[iAVTSessionID[0]].iReadFilePtr;
                }
                if (0 != iAVTSessionID[2])
                {
                    uint32 tempDelta = 0;
                    if (iReadFilePositions[iAVTSessionID[1]].iReadFilePtr > iReadFilePositions[iAVTSessionID[2]].iReadFilePtr)
                    {
                        tempDelta = iReadFilePositions[iAVTSessionID[1]].iReadFilePtr > iReadFilePositions[iAVTSessionID[2]].iReadFilePtr;
                    }
                    else
                    {
                        tempDelta = iReadFilePositions[iAVTSessionID[2]].iReadFilePtr > iReadFilePositions[iAVTSessionID[1]].iReadFilePtr;
                    }
                    if (tempDelta > iAVTOffsetDelta)
                    {
                        iAVTOffsetDelta = tempDelta;
                    }
                    if (iReadFilePositions[iAVTSessionID[0]].iReadFilePtr > iReadFilePositions[iAVTSessionID[2]].iReadFilePtr)
                    {
                        tempDelta = iReadFilePositions[iAVTSessionID[0]].iReadFilePtr > iReadFilePositions[iAVTSessionID[2]].iReadFilePtr;
                    }
                    else
                    {
                        tempDelta = iReadFilePositions[iAVTSessionID[2]].iReadFilePtr > iReadFilePositions[iAVTSessionID[0]].iReadFilePtr;
                    }
                    if (tempDelta > iAVTOffsetDelta)
                    {
                        iAVTOffsetDelta = tempDelta;
                    }
                }
            }
        }
    }

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::SetReadPointerPosition returning %d", status));
    return status;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFMemoryBufferWriteDataStreamImpl::SetReadPointerCacheLocation(PvmiDataStreamSession aSessionID, bool aInTempCache)
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::SetReadPointerCacheLocation session %d temp cache %d", aSessionID, aInTempCache));

    PvmiDataStreamStatus status = PVDS_SUCCESS;
    if ((aSessionID == 0) ||
            (aSessionID > (PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS + PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS)) ||
            (iReadFilePositions[aSessionID - PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS].iReadPositionStructValid != true))
    {
        LOGERROR((0, "PVMFMemoryBufferWriteDataStreamImpl::SetReadPointerCacheLocation invalid session %d", aSessionID));
        status = PVDS_FAILURE;
    }
    else
    {
        PvmiDataStreamSession index = aSessionID - PV_MBDS_MAX_NUMBER_OF_WRITE_CONNECTIONS;
        iReadFilePositions[index].iInTempCache = aInTempCache;
    }
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::SetReadPointerCacheLocation returning %d", status));
    return status;

}


OSCL_EXPORT_REF void
PVMFMemoryBufferWriteDataStreamImpl::ManageReadCapacityNotifications()
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::ManageReadCapacityNotifications"));

    // Loop through iReadNotifications for a Read Notification
    // Send notification if write pointer has advanced beyond requested read capacity, or
    // download has completed

    uint32 currFilePosition = iFilePtrPos;
    for (uint32 i = 0; i < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS; i++)
    {
        // Make sure it a valid iReadNotifications element
        //  AND the iReadObserver != NULL
        if ((iReadNotifications[i].iReadStructValid == true) &&
                (iReadNotifications[i].iOutstanding == true) &&
                (iReadNotifications[i].iReadObserver != NULL))
        {
            bool bSend = false;
            PVMFStatus status = PVMFFailure;

            if ((currFilePosition - iReadNotifications[i].iFilePosition) >
                    iReadNotifications[i].iReadCapacity)
            {
                bSend = true;
                status = PVMFSuccess;

            }
            else if (iDownloadComplete)
            {
                // no more data is coming
                // return failure
                bSend = true;
            }
            if (bSend)
            {
                // Retrieve notification info
                PvmiDataStreamObserver* observer = iReadNotifications[i].iReadObserver;
                OsclAny* contextData = iReadNotifications[i].iContextData;
                PvmiDataStreamCommandId cmdID = iReadNotifications[i].iCommandID;
                // Reset the iReadNotifications for the next notification
                iReadNotifications[i].iOutstanding = false;
                iReadNotifications[i].iReadObserver = NULL;
                iReadNotifications[i].iReadCapacity = 0;
                iReadNotifications[i].iFilePosition = 0;
                iReadNotifications[i].iCommandID = 0;
                iReadNotifications[i].iContextData = NULL;
                // Form a command response.
                PVMFCmdResp resp(cmdID, contextData, status, NULL, NULL);
                // Make the Command Complete notification.
                observer->DataStreamCommandCompleted(resp);
            }
        }
    }
}


OSCL_EXPORT_REF void
PVMFMemoryBufferWriteDataStreamImpl::ManageCache()
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::ManageCache entry"));

    // Only need to manage temporary cache,
    // leave a 64000 buffer zone at the beginning of cache
    // only release the buffers when the cache has reached a certain capacity,
    // except in the case of when MakePersistent has just been called,
    // where the data may be duplicated in both caches
    // The mem frags should be released in the order they sent to MBDS
    if (0 == iTempCache->GetNumEntries())
    {
        // temp cache is empty, nothing to do
        LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::ManageCache temp cache is empty, do nothing"));
        return;
    }

    // for debug only
    for (int32 j = 0; j < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS; j++)
    {
        LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::ManageCache session %d valid %d cache %d ptr %d",
                  j + 1, iReadFilePositions[j].iReadPositionStructValid, iReadFilePositions[j].iInTempCache, iReadFilePositions[j].iReadFilePtr));
    }

    // All parsers have to call MakePersistent to get the cache moving
    if (!iMadePersistent)
    {
        LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::ManageCache MakePersistent not yet called"));
        return;
    }

    // If there are no read position pointers in the temp cache, leave the cache alone,
    // except in the case where MakePersistent has just been called,
    // the data in the perm cache will also be in the temp cache,
    // Currently MakePersistent is called with (offset = 0, num of bytes)

    // If there are read position pointers in the temp cache,
    // find the smallest pointer position,
    // subtract 64000 from that and release old mem frags up to that point

    uint32 firstPermByteOffset = 0;
    uint32 lastPermByteOffset = 0;
    iPermCache->GetFileOffsets(firstPermByteOffset, lastPermByteOffset);

    uint32 firstPersistentOffset = 0;
    uint32 lastPersistentOffset = 0;
    iPermCache->GetPermOffsets(firstPersistentOffset, lastPersistentOffset);

    uint32 firstTempByteOffset = 0;
    uint32 lastTempByteOffset = 0;
    iTempCache->GetFileOffsets(firstTempByteOffset, lastTempByteOffset);

    LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::ManageCache firstPermByteOffset %d lastPermByteOffset %d firstTempByteOffset %d lastTempByteOffset %d",
              firstPermByteOffset, lastPermByteOffset, firstTempByteOffset, lastTempByteOffset));

    // Check for read position pointers in temp cache
    bool found = false;
    bool trim = false;
    uint32 smallest = 0xFFFFFFFF;
    for (int32 i = 0; i < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS; i++)
    {
        if ((iReadFilePositions[i].iReadPositionStructValid == true) && (iReadFilePositions[i].iInTempCache == true))
        {
            if ((0 == i) && (0 == firstPersistentOffset) && (0 == lastPersistentOffset) && (0 == iReadFilePositions[i].iReadFilePtr))
            {
                // nothing made persistent, ignore session 0 ptr at offset 0
                continue;
            }
            found = true;
            if (iReadFilePositions[i].iReadFilePtr < smallest)
            {
                smallest = iReadFilePositions[i].iReadFilePtr;
            }
        }
    }

    if (!found && (0 != iPermCache->GetNumEntries()))
    {
        // No read position pointers in temp cache and perm cache is not empty
        // Check if data in perm cache is also in temp cache (right after MakePersistent)
        // If not, don't touch the temp cache
        // TBD - what if the bytes are not at the beginning of the clip?
        if (!((firstPermByteOffset == firstTempByteOffset) && (lastPermByteOffset <= lastTempByteOffset)))
        {
            // nothing to do
            LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::ManageCache no read ptrs in temp cache, do nothing"));
            return;
        }
        else
        {
            // This is the case right after MakePersistent
            // Need to release the mem frags containing duplicated data in the temp cache
            trim = true;
            smallest = lastPermByteOffset + 1;
        }
    }

    if ((0xFFFFFFFF != smallest) && !trim)
    {
        // Put in a buffer zone at the beginning of cache (64000 for PS and 4096 for Shoutcast)
        // If there is less than 64000 at the beginning, don't touch the cache
        // This is important in case there are other read sessions (audio, video, text) that have not yet been opened,
        // don't want to throw away any media data that may be needed later
        if ((smallest - firstTempByteOffset) <= iTempCacheTrimMargin)
        {
            LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::ManageCache leaving a %d zone, do nothing", iTempCacheTrimMargin));
            return;
        }
        else
        {
            smallest -= iTempCacheTrimMargin;
        }
    }

    while ((0 != iTempCache->GetNumEntries()) && (trim || (iTempCache->GetTotalBytes() > (iTempCacheTrimThreshold))))
    {
        // Check if any read pointers are pointing to this frag
        uint32 size = 0;
        uint32 offset = 0;
        iTempCache->GetFirstEntryInfo(offset, size);

        if ((offset + size) <= smallest)
        {
            // this entire fragment is below the zone and can be released if no read pointers are in it
            found = true;
            for (int32 i = 0; i < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS; i++)
            {
                if ((iReadFilePositions[i].iReadPositionStructValid == true) && (iReadFilePositions[i].iInTempCache == true))
                {
                    LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::ManageCache ptr %d session %d", iReadFilePositions[i].iReadFilePtr, i + 1));

                    if ((0 == i) && (0 == firstPersistentOffset) && (0 == lastPersistentOffset) && (0 == iReadFilePositions[i].iReadFilePtr))
                    {
                        // nothing made persistent, ignore session 0 ptr at offset 0
                        continue;
                    }
                    if (iReadFilePositions[i].iReadFilePtr < (offset + size))
                    {
                        // this pointer is in cache entry, don't release
                        LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::ManageCache found ptr %d session %d in temp cache, done", iReadFilePositions[i].iReadFilePtr, i + 1));
                        found = false;
                        break;
                    }
                }
            }

            if (found)
            {
                OsclRefCounterMemFrag* frag = NULL;
                uint8* fragPtr = NULL;
                if (iTempCache->RemoveFirstEntry(frag, fragPtr))
                {
                    // return mem frag to stream writer (e.g. protocol engine)
                    iRequestObserver->DataStreamRequestSync(0, PVDS_REQUEST_MEM_FRAG_RELEASED, (OsclAny*)frag);
                }
            }
            else
            {
                // done
                break;
            }
        }
        else
        {
            // done
            break;
        }
    }
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::ManageCache exit"));
}


OSCL_EXPORT_REF void
PVMFMemoryBufferWriteDataStreamImpl::TrimTempCache(MBDSCacheTrimMode aTrimMode)
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::TrimTempCache mode %d", aTrimMode));

    if (MBDS_CACHE_TRIM_EMPTY == aTrimMode)
    {
        // empty the cache
        while (iTempCache->GetNumEntries() > 0)
        {
            bool found = false;
            OsclRefCounterMemFrag* frag = NULL;
            uint8* fragPtr = NULL;
            found = iTempCache->RemoveFirstEntry(frag, fragPtr);
            if (found)
            {
                // return mem frag to stream writer (e.g. protocol engine)
                iRequestObserver->DataStreamRequestSync(0, PVDS_REQUEST_MEM_FRAG_RELEASED, (OsclAny*)frag);
            }
            else
            {
                // should never get here
                LOGDEBUG((0, "PVMFMemoryBufferWriteDataStreamImpl::TrimTempCache cache corruption"));
                break;
            }
        }
    }
    // This is called after a reposition request has been issued to the server
    // Need to release as many buffers as possible to faciliate cache refill with new data
    // For seek (max), trim from both ends and keep the middle intact
    // For source reconnect (half), trim from bottom end until 1/2 of the buffers are returned.

    if ((MBDS_CACHE_TRIM_HEAD_AND_TAIL == aTrimMode) || (MBDS_CACHE_TRIM_HEAD_ONLY == aTrimMode))
    {
        // Trim from the beginning
        while (iTempCache->GetNumEntries() > 0)
        {
            bool releaseBuf = true;
            uint32 size = 0;
            uint32 offset = 0;
            iTempCache->GetFirstEntryInfo(offset, size);

            for (int i = 0; i < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS; i++)
            {
                if ((true == iReadFilePositions[i].iReadPositionStructValid) &&
                        (iReadFilePositions[i].iInTempCache == true) &&
                        (iReadFilePositions[i].iReadFilePtr >= offset) &&
                        (iReadFilePositions[i].iReadFilePtr < (offset + size)))
                {
                    // don't release this buffer
                    releaseBuf = false;
                    break;
                }
            }
            if (releaseBuf)
            {
                OsclRefCounterMemFrag* frag;
                uint8* fragPtr;
                bool found = iTempCache->RemoveFirstEntry(frag, fragPtr);
                if (found)
                {
                    // return mem frag to stream writer (e.g. protocol engine)
                    iRequestObserver->DataStreamRequestSync(0, PVDS_REQUEST_MEM_FRAG_RELEASED, (OsclAny*)frag);
                }
            }
            else
            {
                // done trimming from the beginning
                break;
            }
        }
    }

    if ((MBDS_CACHE_TRIM_HEAD_AND_TAIL == aTrimMode) || (MBDS_CACHE_TRIM_TAIL_ONLY == aTrimMode))
    {
        // Trim from the end, up to 1/2 of max cache size
        while (((MBDS_CACHE_TRIM_HEAD_AND_TAIL == aTrimMode) && (iTempCache->GetNumEntries() > 0)) ||
                ((MBDS_CACHE_TRIM_TAIL_ONLY == aTrimMode) && (iTempCache->GetTotalBytes() > (iTempCacheCapacity >> 1))))
        {
            bool releaseBuf = true;
            uint32 size = 0;
            uint32 offset = 0;
            iTempCache->GetLastEntryInfo(offset, size);

            for (int i = 0; i < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS; i++)
            {
                if ((true == iReadFilePositions[i].iReadPositionStructValid) &&
                        (iReadFilePositions[i].iInTempCache == true) &&
                        (iReadFilePositions[i].iReadFilePtr >= offset) &&
                        (iReadFilePositions[i].iReadFilePtr < (offset + size)))
                {
                    // don't release this buffer
                    releaseBuf = false;
                    break;
                }
            }
            if (releaseBuf)
            {
                OsclRefCounterMemFrag* frag;
                uint8* fragPtr;
                bool found = iTempCache->RemoveLastEntry(frag, fragPtr);
                if (found)
                {
                    // return mem frag to stream writer (e.g. protocol engine)
                    iRequestObserver->DataStreamRequestSync(0, PVDS_REQUEST_MEM_FRAG_RELEASED, (OsclAny*)frag);
                }
            }
            else
            {
                // done trimming from the end
                break;
            }
        }
    }

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::TrimTempCache exit"));
}


OSCL_EXPORT_REF void
PVMFMemoryBufferWriteDataStreamImpl::UpdateReadPointersAfterMakePersistent()
{
    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::UpdateReadPointersAfterMakePersistent"));

    uint32 firstOffset = 0;
    uint32 lastOffset = 0;
    iPermCache->GetFileOffsets(firstOffset, lastOffset);

    for (int32 i = 0; i < PV_MBDS_MAX_NUMBER_OF_READ_CONNECTIONS; i++)
    {
        if ((true == iReadFilePositions[i].iReadPositionStructValid) && (true == iReadFilePositions[i].iInTempCache))
        {
            if ((iReadFilePositions[i].iReadFilePtr >= firstOffset) &&
                    (iReadFilePositions[i].iReadFilePtr <= lastOffset))
            {
                // data is in perm cache after MakePersistent
                iReadFilePositions[i].iInTempCache = false;
            }
        }
    }
}


OSCL_EXPORT_REF bool
PVMFMemoryBufferWriteDataStreamImpl::GetPermCachePersistence(uint32& aFirstOffset, uint32& aLastOffset)
{
    uint32 firstPersistentOffset = 0;
    uint32 lastPersistentOffset = 0;
    iPermCache->GetPermOffsets(firstPersistentOffset, lastPersistentOffset);

    LOGTRACE((0, "PVMFMemoryBufferWriteDataStreamImpl::GetPermCachePersistence MakePersistent called %d first offset %d last offset %d",
              iMadePersistent, firstPersistentOffset, lastPersistentOffset));

    if (iMadePersistent)
    {
        aFirstOffset = firstPersistentOffset;
        aLastOffset = lastPersistentOffset;
    }
    return iMadePersistent;
}

OSCL_EXPORT_REF void
PVMFMemoryBufferWriteDataStreamImpl::SetStreamFormat(MBDSStreamFormat aStreamFormat)
{
    iStreamFormat = aStreamFormat;
}

OSCL_EXPORT_REF void
PVMFMemoryBufferWriteDataStreamImpl::SetTempCacheCapacity(uint32 aCapacity)
{
    iTempCacheCapacity = aCapacity;
}

OSCL_EXPORT_REF MBDSStreamFormat
PVMFMemoryBufferWriteDataStreamImpl::GetStreamFormat()
{
    return iStreamFormat;
}

OSCL_EXPORT_REF uint32
PVMFMemoryBufferWriteDataStreamImpl::GetTempCacheCapacity()
{
    return iTempCacheCapacity;
}

//////////////////////////////////////////////////////////////////////
// PVMFMemoryBufferDataStream
//////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF
PVMFMemoryBufferDataStream::PVMFMemoryBufferDataStream(PVMFFormatType& aStreamFormat, uint32 aTempCacheCapacity)
{
    // Create a temporary cache and a permanent cache
    iTemporaryCache = OSCL_NEW(PVMFMemoryBufferDataStreamTempCache, ());
    iPermanentCache = OSCL_NEW(PVMFMemoryBufferDataStreamPermCache, ());

    // set the stream format and the temp cache size
    MBDSStreamFormat streamFormat = MBDS_STREAM_FORMAT_PROGRESSIVE_PLAYBACK;
    if (aStreamFormat == PVMF_MIME_DATA_SOURCE_SHOUTCAST_URL)
    {
        streamFormat = MBDS_STREAM_FORMAT_SHOUTCAST;
    }

    // Create the two factories
    iWriteDataStreamFactory = OSCL_NEW(PVMFMemoryBufferWriteDataStreamFactoryImpl, (iTemporaryCache, iPermanentCache, streamFormat, aTempCacheCapacity));
    iReadDataStreamFactory = OSCL_NEW(PVMFMemoryBufferReadDataStreamFactoryImpl, (iTemporaryCache, iPermanentCache));

    // Now create a iWriteDataStream
    PVUuid uuid = PVMIDataStreamSyncInterfaceUuid;
    iWriteDataStream = iWriteDataStreamFactory->CreatePVMFCPMPluginAccessInterface(uuid);

    // Set the pointer in the iReadDataStreamFactory
    iReadDataStreamFactory->SetWriteDataStreamPtr(iWriteDataStream);

    iLogger = PVLogger::GetLoggerObject("PVMFMemoryBufferDataStream");
    LOGTRACE((0, "PVMFMemoryBufferDataStream::PVMFMemoryBufferDataStream"));
}

OSCL_EXPORT_REF
PVMFMemoryBufferDataStream::~PVMFMemoryBufferDataStream()
{
    LOGTRACE((0, "PVMFMemoryBufferDataStream::~PVMFMemoryBufferDataStream"));

    // Delete the two DataStreamFactories
    OSCL_DELETE(iWriteDataStreamFactory);
    OSCL_DELETE(iReadDataStreamFactory);

    // Delete the caches
    OSCL_DELETE(iTemporaryCache);
    OSCL_DELETE(iPermanentCache);

    iLogger = NULL;
}

OSCL_EXPORT_REF PVMFDataStreamFactory*
PVMFMemoryBufferDataStream::GetReadDataStreamFactoryPtr()
{
    LOGTRACE((0, "PVMFMemoryBufferDataStream::GetReadDataStreamFactoryPtr"));

    return OSCL_STATIC_CAST(PVMFDataStreamFactory*, iReadDataStreamFactory);
}

OSCL_EXPORT_REF PVMFDataStreamFactory*
PVMFMemoryBufferDataStream::GetWriteDataStreamFactoryPtr()
{
    LOGTRACE((0, "PVMFMemoryBufferDataStream::GetWriteDataStreamFactoryPtr"));

    return OSCL_STATIC_CAST(PVMFDataStreamFactory*, iWriteDataStreamFactory);
}


// Called by download manager when buffering is complete
OSCL_EXPORT_REF void
PVMFMemoryBufferDataStream::NotifyDownloadComplete()
{
    LOGTRACE((0, "PVMFMemoryBufferDataStream::NotifyDownloadComplete"));

    if (iReadDataStreamFactory != NULL)
    {
        iReadDataStreamFactory->NotifyDownloadComplete();
    }
    if (iWriteDataStreamFactory != NULL)
    {
        iWriteDataStreamFactory->NotifyDownloadComplete();
    }
}


//////////////////////////////////////////////////////////////////////
// PVMFMemoryBufferDataStreamTempCache
//////////////////////////////////////////////////////////////////////
PVMFMemoryBufferDataStreamTempCache::PVMFMemoryBufferDataStreamTempCache()
{
    iTotalBytes = 0;
    iFirstByteFileOffset = 0;
    iLastByteFileOffset = 0;

    iLogger = PVLogger::GetLoggerObject("PVMFMemoryBufferDataStream");
    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::PVMFMemoryBufferDataStreamTempCache %x", this));
}


PVMFMemoryBufferDataStreamTempCache::~PVMFMemoryBufferDataStreamTempCache()
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::~PVMFMemoryBufferDataStreamTempCache %x", this));

    // clean out the cache, remove reference to mem frags is good enough
    if (!iEntries.empty())
    {
        iEntries.clear();
    }

    iLogger = NULL;
}


uint32
PVMFMemoryBufferDataStreamTempCache::GetTotalBytes()
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::GetTotalBytes returning %d", iTotalBytes));
    return iTotalBytes;
}


void
PVMFMemoryBufferDataStreamTempCache::GetFileOffsets(uint32& aFirstByte, uint32& aLastByte)
{

    aFirstByte = iFirstByteFileOffset;
    aLastByte = iLastByteFileOffset;

    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::GetFileOffsets returning first %d last %d", iFirstByteFileOffset, iLastByteFileOffset));
}


PvmiDataStreamStatus
PVMFMemoryBufferDataStreamTempCache::AddEntry(OsclRefCounterMemFrag* aFrag, uint8* aFragPtr, uint32 aFragSize, uint32 aFileOffset)
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::AddEntry ptr %x size %d offset %d", aFragPtr, aFragSize, aFileOffset));

    PvmiDataStreamStatus status = PVDS_SUCCESS;
    // Create an entry for the cache
    // Caller is write data stream,
    // it has checked for contiguous write
    // May want to double check, just in case
    if (!iEntries.empty() && (aFileOffset != (iLastByteFileOffset + 1)))
    {
        status = PVDS_INVALID_REQUEST;
    }
    else
    {
        MBDSTempCacheEntry* entry = (MBDSTempCacheEntry*)OSCL_MALLOC(sizeof(struct MBDSTempCacheEntry));
        if (entry)
        {
            entry->frag = aFrag;
            entry->fragPtr = aFragPtr;
            entry->fragSize = aFragSize;
            entry->fileOffset = aFileOffset;

            iEntries.push_back(entry);

            if (1 == iEntries.size())
            {
                // If there is only one entry,
                // set the first byte offset
                iFirstByteFileOffset = aFileOffset;
                iLastByteFileOffset = iFirstByteFileOffset + aFragSize - 1;
            }
            else
            {
                iLastByteFileOffset += aFragSize;
            }
            iTotalBytes += aFragSize;

            LOGDEBUG((0, "PVMFMemoryBufferDataStreamTempCache::AddEntry %x size %d first %d last %d total %d",
                      entry, aFragSize, iFirstByteFileOffset, iLastByteFileOffset, iTotalBytes));
            // for debugging
            uint8 data[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            int32 count = 16;
            if (aFragSize < 16)
                count = aFragSize;
            for (int32 i = 0; i < count; i++)
            {
                data[i] = *(aFragPtr + i);
            }
            LOGDEBUG((0, "%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x",
                      data[0], data[1], data[2], data[3], data[4], data[5], data[6],
                      data[7], data[8], data[9], data[10], data[11], data[12], data[13],
                      data[14], data[15]));
        }
        else
        {
            status = PVDS_FAILURE;
        }
    }

    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::AddEntry returning %d", status));
    return status;
}



bool
PVMFMemoryBufferDataStreamTempCache::RemoveFirstEntry(OsclRefCounterMemFrag*& aFrag, uint8*& aFragPtr)
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::RemoveFirstEntry first %d last %d total %d", iFirstByteFileOffset, iLastByteFileOffset, iTotalBytes));

    bool found = false;

    if (!iEntries.empty())
    {
        // Remove and return the first/oldest entry in the cache and free the memory
        MBDSTempCacheEntry* entry = iEntries.front();

        found = true;

#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
        uint32 offset = entry->fileOffset;
#endif
        uint32 size = entry->fragSize;

        aFrag = entry->frag;
        aFragPtr = entry->fragPtr;

        iEntries.erase(iEntries.begin());

        OSCL_FREE(entry);

        if (!iEntries.empty())
        {
            iTotalBytes -= size;

            // Next entry becomes first entry
            entry = iEntries.front();
            iFirstByteFileOffset = entry->fileOffset;
        }
        else
        {
            // no more entries
            iFirstByteFileOffset = 0;
            iLastByteFileOffset = 0;
            iTotalBytes = 0;
        }

#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
        LOGDEBUG((0, "PVMFMemoryBufferDataStreamTempCache::RemoveFirstEntry %x offset %d size %d first %d last %d total %d",
                  entry, offset, size, iFirstByteFileOffset, iLastByteFileOffset, iTotalBytes));
#endif

    }

    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::RemoveFirstEntry returning %d", (int)found));
    return found;
}


bool
PVMFMemoryBufferDataStreamTempCache::RemoveLastEntry(OsclRefCounterMemFrag*& aFrag, uint8*& aFragPtr)
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::RemoveLastEntry first %d last %d total %d", iFirstByteFileOffset, iLastByteFileOffset, iTotalBytes));

    bool found = false;

    if (!iEntries.empty())
    {
        // Remove and return the last/newest entry in the cache
        // Free the memory
        MBDSTempCacheEntry* entry = iEntries.back();

        found = true;

        uint32 offset = entry->fileOffset;
        uint32 size = entry->fragSize;

        aFrag = entry->frag;
        aFragPtr = entry->fragPtr;

        iEntries.pop_back();

        OSCL_FREE(entry);

        if (!iEntries.empty())
        {
            // Second last entry becomes last entry
            iTotalBytes -= size;
            iLastByteFileOffset = offset - 1;
        }
        else
        {
            // no more entries
            iFirstByteFileOffset = 0;
            iLastByteFileOffset = 0;
            iTotalBytes = 0;
        }

        LOGDEBUG((0, "PVMFMemoryBufferDataStreamTempCache::RemoveLastEntry  %x offset %d size %d first %d last %d total %d",
                  entry, offset, size, iFirstByteFileOffset, iLastByteFileOffset, iTotalBytes));
    }

    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::RemoveLastEntry returning %d", (int)found));
    return found;
}


uint32
PVMFMemoryBufferDataStreamTempCache::ReadBytes(uint8* aBuffer, uint32 aFirstByte, uint32 aLastByte, uint32& firstEntry)
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::ReadBytes buf %x first %d last %d", aBuffer, aFirstByte, aLastByte));

    // Caller is the read data stream, it limits the read to what is in the cache
    // the first byte should be in the cache, need to go through the cache entries for the file offset
    bool found = false;
    uint32 count = iEntries.size();
    uint32 bytesRead = 0;
    uint32 bytesToRead = 0;
    uint8* dataPtr = NULL;
    uint8* bufferPtr = aBuffer;

    firstEntry = 0;

    for (uint32 i = 0; i < count; i++)
    {
        if (!found)
        {
            // haven't found the first offset yet
            if ((aFirstByte >= iEntries[i]->fileOffset) && (aFirstByte < (iEntries[i]->fileOffset + iEntries[i]->fragSize)))
            {
                // the first byte to be read is in this frag
                found = true;
                if (aLastByte < (iEntries[i]->fileOffset + iEntries[i]->fragSize))
                {
                    //LOGDEBUG((0, "TC1 entry %d offset %d size %d", i, iEntries[i]->fileOffset, iEntries[i]->fragSize));
                    // every byte to be read is in this one frag
                    dataPtr = iEntries[i]->fragPtr + (aFirstByte - iEntries[i]->fileOffset);
                    bytesToRead = aLastByte - aFirstByte + 1;
                    // copy the data to the buffer
                    oscl_memcpy(bufferPtr, dataPtr, bytesToRead);
                    bytesRead += bytesToRead;
                    firstEntry = i;
                    // done
                    break;
                }
                else
                {
                    //LOGDEBUG((0, "TC2 entry %d offset %d size %d", i, iEntries[i]->fileOffset, iEntries[i]->fragSize));
                    // only a portion is in this frag
                    dataPtr = iEntries[i]->fragPtr + (aFirstByte - iEntries[i]->fileOffset);
                    bytesToRead = iEntries[i]->fragSize - (aFirstByte - iEntries[i]->fileOffset);
                    // copy the data, the rest of the data should be in the next frag or frags
                    oscl_memcpy(bufferPtr, dataPtr, bytesToRead);
                    bytesRead += bytesToRead;
                    bufferPtr += bytesToRead;
                    firstEntry = i;
                }
            }
        }
        else
        {
            // the first byte was in the previous frag, more data to copy from this frag
            if (aLastByte < (iEntries[i]->fileOffset + iEntries[i]->fragSize))
            {
                //LOGDEBUG((0, "TC3 entry %d offset %d size %d", i, iEntries[i]->fileOffset, iEntries[i]->fragSize));
                // what is left to be read is in this one frag
                dataPtr = iEntries[i]->fragPtr;
                bytesToRead = aLastByte - iEntries[i]->fileOffset + 1;
                // copy the data to the buffer
                oscl_memcpy(bufferPtr, dataPtr, bytesToRead);
                bytesRead += bytesToRead;
                // done
                break;
            }
            else
            {
                //LOGDEBUG((0, "TC4 entry %d offset %d size %d", i, iEntries[i]->fileOffset, iEntries[i]->fragSize));
                // all of this frag is needed
                dataPtr = iEntries[i]->fragPtr;
                bytesToRead = iEntries[i]->fragSize;
                // copy the data, the rest of the data should be in the next frag or frags
                oscl_memcpy(bufferPtr, dataPtr, bytesToRead);
                bytesRead += bytesToRead;
                bufferPtr += bytesToRead;
            }
        }
    }

    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::ReadBytes returning %d firstEntry %d", bytesRead, firstEntry));
    return bytesRead;
}


void
PVMFMemoryBufferDataStreamTempCache::GetFirstEntryInfo(uint32& entryOffset, uint32& entrySize)
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::GetFirstEntryInfo"));

    // Return the frag size of the first/oldest entry in cache
    if (!iEntries.empty())
    {
        MBDSTempCacheEntry* entry = iEntries.front();

        entrySize = entry->fragSize;
        entryOffset = entry->fileOffset;
    }

    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::GetFirstEntryInfo returning offset %d size %d", entryOffset, entrySize));
}


void
PVMFMemoryBufferDataStreamTempCache::GetLastEntryInfo(uint32& entryOffset, uint32& entrySize)
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamCache::GetLastEntryInfo"));

    // Return the frag size of the last/newest entry in cache
    if (!iEntries.empty())
    {
        MBDSTempCacheEntry* entry = iEntries.back();

        entrySize = entry->fragSize;
        entryOffset = entry->fileOffset;
    }

    LOGTRACE((0, "PVMFMemoryBufferDataStreamCache::GetFirstEntryInfo returning offset %d size %d", entryOffset, entrySize));
}


uint32
PVMFMemoryBufferDataStreamTempCache::GetNumEntries()
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamTempCache::GetNumEntries returning %d", iEntries.size()));
    // return number of entries in cache
    return iEntries.size();
}


//////////////////////////////////////////////////////////////////////
// PVMFMemoryBufferDataStreamPermCache
//////////////////////////////////////////////////////////////////////
PVMFMemoryBufferDataStreamPermCache::PVMFMemoryBufferDataStreamPermCache()
{
    iTotalBytes = 0;
    iTotalBufferAlloc = 0;
    iFirstByteFileOffset = 0;
    iLastByteFileOffset = 0;
    iFirstPermByteOffset = 0;
    iLastPermByteOffset = 0;

    iLogger = PVLogger::GetLoggerObject("PVMFMemoryBufferDataStream");
    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::PVMFMemoryBufferDataStreamPermCache %x", this));
}



PVMFMemoryBufferDataStreamPermCache::~PVMFMemoryBufferDataStreamPermCache()
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::~PVMFMemoryBufferDataStreamPermCache %x", this));

    // clean out the cache, need to free the memory
    if (!iEntries.empty())
    {
        while (!iEntries.empty())
        {
            uint8* memPtr = iEntries[0]->bufPtr;
            if (memPtr)
            {
                oscl_free(memPtr);
            }
            iEntries.erase(iEntries.begin());
        }
    }

    iLogger = NULL;
}


uint32
PVMFMemoryBufferDataStreamPermCache::GetTotalBytes()
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::GetTotalBytes returning %d", iTotalBytes));
    return iTotalBytes;
}


void
PVMFMemoryBufferDataStreamPermCache::GetFileOffsets(uint32& aFirstByte, uint32& aLastByte)
{
    aFirstByte = iFirstByteFileOffset;
    aLastByte = iLastByteFileOffset;

    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::GetFileOffsets returning first %d last %d", aFirstByte, aLastByte));
}


void
PVMFMemoryBufferDataStreamPermCache::GetPermOffsets(uint32& aFirstByte, uint32& aLastByte)
{
    aFirstByte = iFirstPermByteOffset;
    aLastByte = iLastPermByteOffset;

    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::GetPermOffsets returning first %d last %d", aFirstByte, aLastByte));
}

PvmiDataStreamStatus
PVMFMemoryBufferDataStreamPermCache::AddEntry(uint8* aBufPtr, uint32 aBufSize, uint8* aFillPtr, uint32 aFirstOffset,
        uint32 aLastOffset, uint32 aFillOffset, uint32 aFillSize)
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::AddEntry bufPtr %x bufSize %d fillPtr %x firstOffset %d lastOffset %d fillOffset %d fillSize %d",
              aBufPtr, aBufSize, aFillPtr, aFirstOffset, aLastOffset, aFillOffset, aFillSize));

    PvmiDataStreamStatus status = PVDS_SUCCESS;
    // caller should have checked for contiguous write
    // may want to double check, just in case
    if (!iEntries.empty() && ((aFirstOffset != (iLastByteFileOffset + 1)) && (aLastOffset != iFirstByteFileOffset - 1)))
    {
        status = PVDS_INVALID_REQUEST;
        LOGERROR((0, "PVMFMemoryBufferDataStreamPermCache::AddEntry FAILED not contiguous entry"));
    }
    else
    {
        MBDSPermCacheEntry* entry = (MBDSPermCacheEntry*)OSCL_MALLOC(sizeof(struct MBDSPermCacheEntry));
        if (entry)
        {
            entry->bufPtr = aBufPtr;
            entry->bufSize = aBufSize;
            entry->fillBufPtr = aFillPtr;
            entry->firstFileOffset = aFirstOffset;
            entry->lastFileOffset = aLastOffset;
            entry->fillFileOffset = aFillOffset;
            entry->fillSize = aFillSize;

            // the entry can be added to the beginning or the end of the cache
            if (!iEntries.empty() && (aLastOffset == iFirstByteFileOffset - 1))
            {
                // adding to the beginning
                iEntries.push_front(entry);
                // first byte made persistent
                iFirstPermByteOffset = aFirstOffset;
                // data may not be here yet
                if (0 != aFillSize)
                {
                    iFirstByteFileOffset = aFirstOffset;
                }
            }
            else
            {
                // adding to the end
                iEntries.push_back(entry);
                // last byte to be made persistent
                iLastPermByteOffset = aLastOffset;
                // data may not be here yet
                if (0 != aFillSize)
                {
                    if (aFillSize == aBufSize)
                    {
                        iLastByteFileOffset = aLastOffset;
                    }
                    else
                    {
                        iLastByteFileOffset = aFillOffset - 1;
                    }
                }
            }
            iTotalBytes += aFillSize;
            iTotalBufferAlloc += aBufSize;
            // for debugging use
            LOGDEBUG((0, "PVMFMemoryBufferDataStreamPermCache::AddEntry %x bufsize %d fillsize %d first %d last %d total %d",
                      entry, aBufSize, aFillSize, iFirstByteFileOffset, iLastByteFileOffset, iTotalBytes));
            uint8 data[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            int32 count = 16;
            if (aBufSize < 16)
                count = aBufSize;
            for (int32 i = 0; i < count; i++)
            {
                data[i] = *(aBufPtr + i);
            }
            LOGDEBUG((0, "%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x",
                      data[0], data[1], data[2], data[3], data[4], data[5], data[6],
                      data[7], data[8], data[9], data[10], data[11], data[12], data[13],
                      data[14], data[15]));
        }
        else
        {
            status = PVDS_FAILURE;
        }
    }

    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::AddEntry returning %d", status));
    return status;
}


PvmiDataStreamStatus
PVMFMemoryBufferDataStreamPermCache::WriteBytes(uint8* aFragPtr, uint32 aFragSize, uint32 aFileOffset)
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::WriteBytes aFragPtr %x aFragSize %d aFileOffset %d", aFragPtr, aFragSize, aFileOffset));

    // make sure cache is not empty
    if (iEntries.empty())
    {
        LOGDEBUG((0, "PVMFMemoryBufferDataStreamPermCache::WriteBytes Failed invalid request"));
        return PVDS_INVALID_REQUEST;
    }
    // the cache entry/entries has already been allocated and added to the cache
    // find the cache entry that matches the file offset
    // copy the data into the cache and update the data structure
    // caller is responsible to make sure that the cache entries are in the cache
    bool found = false;
    uint32 entry = 0;
    for (uint32 i = 0; i < iEntries.size(); i++)
    {
        // find the first cache entry in which the file offset resides
        if ((aFileOffset >= iEntries[i]->firstFileOffset) && (aFileOffset <= iEntries[i]->lastFileOffset))
        {
            // allow writing over existing data,
            // but do not allow a gap in the cache entry
            if (aFileOffset > iEntries[i]->fillFileOffset)
            {
                // gap
                LOGERROR((0, "PVMFMemoryBufferDataStreamPermCache::WriteBytes Failed gap found fillFileOffset %d aFileOffset %d", iEntries[i]->fillFileOffset, aFileOffset));
                return PVDS_INVALID_REQUEST;
            }

            found = true;
            entry = i;
            break;
        }
    }
    if (!found)
    {
        LOGERROR((0, "PVMFMemoryBufferDataStreamPermCache::WriteBytes Failed cache entry not found"));
        return PVDS_INVALID_REQUEST;
    }
    // copy data into cache
    uint8* dstPtr = iEntries[entry]->bufPtr + (aFileOffset - iEntries[entry]->firstFileOffset);
    uint8* srcPtr = aFragPtr;
    uint32 bytesToCopy = aFragSize;
    uint32 bufAvail = iEntries[entry]->bufSize - (aFileOffset - iEntries[entry]->firstFileOffset);
    uint32 copySize = bytesToCopy;
    while (bytesToCopy)
    {
        copySize = bytesToCopy;
        if (bytesToCopy > bufAvail)
        {
            copySize = bufAvail;
        }
        oscl_memcpy(dstPtr, srcPtr, copySize);

        iEntries[entry]->fillFileOffset += copySize;
        iEntries[entry]->fillSize += copySize;

        bytesToCopy -= copySize;
        if (++entry >= iEntries.size())
        {
            break;
        }
        dstPtr = iEntries[entry]->bufPtr;
        srcPtr += copySize;
        bufAvail = iEntries[entry]->bufSize;
    }

    if ((aFileOffset + aFragSize - 1) > iLastByteFileOffset)
    {
        iLastByteFileOffset = aFileOffset + aFragSize - 1;
    }
    iTotalBytes += aFragSize;

    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::WriteBytes success iLastByteFileOffset % d", iLastByteFileOffset));
    return PVDS_SUCCESS;
}


uint32
PVMFMemoryBufferDataStreamPermCache::ReadBytes(uint8* aBuffer, uint32 aFirstByte, uint32 aLastByte)
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::ReadBytes buf %x first %d last %d", aBuffer, aFirstByte, aLastByte));

    // Caller is the read data stream, it limits the read to what is in the cache
    // the first byte should be in the cache, go through the cache entires looking for the file offset
    bool found = false;
    uint32 count = iEntries.size();
    uint32 bytesRead = 0;
    uint32 bytesToRead = 0;
    uint8* dataPtr = NULL;
    uint8* bufferPtr = aBuffer;

    for (uint32 i = 0; i < count; i++)
    {
        // there should not be holes in the cache right now
        // once an empty cache entry is found, break out
        if (0 == iEntries[i]->fillSize)
        {
            break;
        }
        if (!found)
        {
            // haven't found the first offset yet
            if ((aFirstByte >= iEntries[i]->firstFileOffset) && (aFirstByte < iEntries[i]->fillFileOffset))
            {
                // the first byte to be read is in this entry
                found = true;
                if (aLastByte < iEntries[i]->fillFileOffset)
                {
                    //LOGDEBUG((0, "PC1 entry %d offset %d size %d", i, iEntries[i]->firstFileOffset, iEntries[i]->fillSize));
                    // every byte to be read is in this entry
                    dataPtr = iEntries[i]->bufPtr + (aFirstByte - iEntries[i]->firstFileOffset);
                    bytesToRead = aLastByte - aFirstByte + 1;
                    // copy the data to the buffer
                    oscl_memcpy(bufferPtr, dataPtr, bytesToRead);
                    bytesRead += bytesToRead;
                    // done
                    break;
                }
                else
                {
                    //LOGDEBUG((0, "PC2 entry %d offset %d size %d", i, iEntries[i]->firstFileOffset, iEntries[i]->fillSize));
                    // only a portion is in this entry
                    dataPtr = iEntries[i]->bufPtr + (aFirstByte - iEntries[i]->firstFileOffset);
                    bytesToRead = iEntries[i]->fillSize - (aFirstByte - iEntries[i]->firstFileOffset);
                    // copy the data, the rest of the data should be in the next entry/entries
                    oscl_memcpy(bufferPtr, dataPtr, bytesToRead);
                    bytesRead += bytesToRead;
                    bufferPtr += bytesToRead;
                }
            }
        }
        else
        {
            // the first byte was in the previous entry, more data to copy from this entry
            if (aLastByte < (iEntries[i]->firstFileOffset + iEntries[i]->fillSize))
            {
                //LOGDEBUG((0, "PC3 entry %d offset %d size %d", i, iEntries[i]->firstFileOffset, iEntries[i]->fillSize));
                // what is left to be read is in this entry
                dataPtr = iEntries[i]->bufPtr;
                bytesToRead = aLastByte - iEntries[i]->firstFileOffset + 1;
                // copy the data to the buffer
                oscl_memcpy(bufferPtr, dataPtr, bytesToRead);
                bytesRead += bytesToRead;
                // done
                break;
            }
            else
            {
                //LOGDEBUG((0, "PC4 entry %d offset %d size %d", i, iEntries[i]->firstFileOffset, iEntries[i]->fillSize));
                // all of this entry is needed
                dataPtr = iEntries[i]->bufPtr;
                bytesToRead = iEntries[i]->fillSize;
                // copy the data, the rest of the data should be in the next entry/entries
                oscl_memcpy(bufferPtr, dataPtr, bytesToRead);
                bytesRead += bytesToRead;
                bufferPtr += bytesToRead;
            }
        }
    }
    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::ReadBytes returning %d", bytesRead));
    return bytesRead;
}


bool
PVMFMemoryBufferDataStreamPermCache::RemoveFirstEntry(uint8*& aFragPtr)
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::RemoveFirstEntry first %d last %d total %d", iFirstByteFileOffset, iLastByteFileOffset, iTotalBytes));

    bool found = false;

    if (!iEntries.empty())
    {
        // Remove and return the first/oldest entry in the cache and free the memory
        MBDSPermCacheEntry* entry = iEntries.front();

        found = true;
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
        uint32 offset = entry->firstFileOffset;
#endif
        uint32 size = entry->fillSize;

        aFragPtr = entry->bufPtr;

        iEntries.erase(iEntries.begin());

        OSCL_FREE(entry);

        // House keeping
        if (!iEntries.empty())
        {
            iTotalBytes -= size;

            // Next entry becomes first entry
            entry = iEntries.front();
            iFirstByteFileOffset = entry->firstFileOffset;
        }
        else
        {
            // no more entries
            iFirstByteFileOffset = 0;
            iLastByteFileOffset = 0;
            iTotalBytes = 0;
        }
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
        LOGDEBUG((0, "PVMFMemoryBufferDataStreamTempCache::RemoveFirstEntry %x offset %d size %d first %d last %d total %d",
                  entry, offset, size, iFirstByteFileOffset, iLastByteFileOffset, iTotalBytes));
#endif

    }

    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::RemoveFirstEntry returning %d", (int)found));
    return found;
}


uint32
PVMFMemoryBufferDataStreamPermCache::GetNumEntries()
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::GetNumEntries returning %d", iEntries.size()));
    // return number of entries in cache
    return iEntries.size();
}


uint32
PVMFMemoryBufferDataStreamPermCache::GetCacheSize()
{
    LOGTRACE((0, "PVMFMemoryBufferDataStreamPermCache::GetCacheSize %d", iTotalBufferAlloc));
    // return the size of all buffers allocated
    return iTotalBufferAlloc;
}

