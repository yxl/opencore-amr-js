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
 * File Buffer Data Stream (FBDS) class will act as the data pipe between the
 * protocol engine node and the file format parsing node.
 * This will write  the data from the protocol engine in a file for later reading by the
 * file format parsing node.
 */

#ifndef OSCL_EXCEPTION_H_INCLUDED
#include "oscl_exception.h"
#endif
#ifndef PVMF_FILEBUFFERDATASTREAM_FACTORY_H_INCLUDED
#include "pvmf_filebufferdatastream_factory.h"
#endif

// Logging #define
#define LOGDEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_VERBOSE,m);

//////////////////////////////////////////////////////////////////////
// PVMFFileBufferDataStreamReadDataStreamFactoryImpl
//////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF
PVMFFileBufferDataStreamReadDataStreamFactoryImpl::PVMFFileBufferDataStreamReadDataStreamFactoryImpl(OSCL_wString& aFileName)
{
    iFileName = aFileName;
    iDownloadComplete = false;
}

OSCL_EXPORT_REF void
PVMFFileBufferDataStreamReadDataStreamFactoryImpl::SetWriteDataStreamPtr(PVInterface* aWriteDataStream)
{
    if (aWriteDataStream)
    {
        iWriteDataStream = OSCL_STATIC_CAST(PVMFFileBufferWriteDataStreamImpl*, aWriteDataStream);
    }
}

OSCL_EXPORT_REF PVMFStatus
PVMFFileBufferDataStreamReadDataStreamFactoryImpl::QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids)
{
    aUuids.push_back(PVMIDataStreamSyncInterfaceUuid);
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVInterface*
PVMFFileBufferDataStreamReadDataStreamFactoryImpl::CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid)
{
    // Create a new PVMFFileBufferDataStreamReadDataStreamFactoryImpl for each request.
    if (aUuid == PVMIDataStreamSyncInterfaceUuid)
    {
        PVMFFileBufferReadDataStreamImpl* ReadStream = NULL;
        ReadStream = OSCL_NEW(PVMFFileBufferReadDataStreamImpl, (iWriteDataStream, iFileName));
        if (ReadStream == NULL)
        {
            OSCL_LEAVE(OsclErrNoMemory);
        }
        ReadStream->iDownloadComplete = iDownloadComplete;
        iReadStreamVec.push_back(ReadStream);
        return OSCL_STATIC_CAST(PVInterface*, ReadStream);
    }
    return NULL;
}

OSCL_EXPORT_REF void
PVMFFileBufferDataStreamReadDataStreamFactoryImpl::DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid,
        PVInterface* aPtr)
{
    // Destroy the incoming object only if it uses the right UUID and has a vaild pointer
    if ((aUuid == PVMIDataStreamSyncInterfaceUuid) && (aPtr))
    {
        // Cast the incoming ptr to the correct type, then delete
        PVMFFileBufferReadDataStreamImpl* iReadStream = NULL;
        iReadStream = OSCL_STATIC_CAST(PVMFFileBufferReadDataStreamImpl*, aPtr);

        Oscl_Vector<PVMFFileBufferReadDataStreamImpl*, OsclMemAllocator>::iterator it;
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
PVMFFileBufferDataStreamReadDataStreamFactoryImpl::NotifyDownloadComplete()
{
    iDownloadComplete = true;
    Oscl_Vector<PVMFFileBufferReadDataStreamImpl*, OsclMemAllocator>::iterator it;
    for (it = iReadStreamVec.begin();  it != iReadStreamVec.end(); it++)
    {
        (*it)->NotifyDownloadComplete();
    }
}


//////////////////////////////////////////////////////////////////////
// PVMFFileBufferDataStreamWriteDataStreamFactoryImpl
//////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF
PVMFFileBufferDataStreamWriteDataStreamFactoryImpl::PVMFFileBufferDataStreamWriteDataStreamFactoryImpl(OSCL_wString& aFileName)
{
    // Init to NULL for later creation
    iWriteDataStream = NULL;
    // Set the Filename to pass
    iFileName = aFileName;
    iDownloadComplete = false;
}

OSCL_EXPORT_REF
PVMFFileBufferDataStreamWriteDataStreamFactoryImpl::~PVMFFileBufferDataStreamWriteDataStreamFactoryImpl()
{
    if (iWriteDataStream)
    {
        OSCL_DELETE(iWriteDataStream);
    }
}

OSCL_EXPORT_REF PVMFStatus
PVMFFileBufferDataStreamWriteDataStreamFactoryImpl::QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids)
{
    aUuids.push_back(PVMIDataStreamSyncInterfaceUuid);
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVInterface*
PVMFFileBufferDataStreamWriteDataStreamFactoryImpl::CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid)
{
    if (aUuid == PVMIDataStreamSyncInterfaceUuid)
    {
        // iWriteDataStream should have only one instance.
        if (!iWriteDataStream)
        {
            // It does not exist so allocate
            iWriteDataStream = OSCL_NEW(PVMFFileBufferWriteDataStreamImpl, (iFileName));
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
PVMFFileBufferDataStreamWriteDataStreamFactoryImpl::DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid,
        PVInterface* aPtr)
{
    // Do nothing
    OSCL_UNUSED_ARG(aUuid);
    OSCL_UNUSED_ARG(aPtr);
}

PvmiDataStreamStatus
PVMFFileBufferDataStreamWriteDataStreamFactoryImpl::GetStreamReadCapacity(uint32& aCapacity)
{
    aCapacity = 0;
    if (iWriteDataStream != NULL)
    {
        //id does not matter
        PvmiDataStreamSession id = 0;
        return (iWriteDataStream->QueryReadCapacity(id, aCapacity));
    }
    return PVDS_FAILURE;
}

//////////////////////////////////////////////////////////////////////
// PVMFFileBufferReadDataStreamImpl
//////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF
PVMFFileBufferReadDataStreamImpl::PVMFFileBufferReadDataStreamImpl(PVMFFileBufferWriteDataStreamImpl* aWriteDataStream, OSCL_wString& aFileName)
{
    iDownloadComplete = false;
    iWriteDataStream = aWriteDataStream;
    iFileHandle = NULL;
    iFileObject = NULL;
    iFileNumBytes = 0;
    iSessionID = 0;
    iFileName = aFileName;
    iFs.Connect();
    iLogger = PVLogger::GetLoggerObject("PVMFFileBufferReadDataStreamImpl");
    LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::PVMFFileBufferReadDataStreamImpl"));
}

OSCL_EXPORT_REF
PVMFFileBufferReadDataStreamImpl::~PVMFFileBufferReadDataStreamImpl()
{
    LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::~PVMFFileBufferReadDataStreamImpl"));
    if (iFileObject)
        OSCL_DELETE(iFileObject);
    iFileObject = NULL;
    iLogger = NULL;
    iFs.Close();
}

OSCL_EXPORT_REF bool
PVMFFileBufferReadDataStreamImpl::queryInterface(const PVUuid& uuid,
        PVInterface*& iface)
{
    iface = NULL;
    LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::queryInterface"));
    if (uuid == PVMIDataStreamSyncInterfaceUuid)
    {
        PVMIDataStreamSyncInterface* myInterface
        = OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return true;
    }
    return false;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferReadDataStreamImpl::OpenSession(PvmiDataStreamSession& aSessionID,
        PvmiDataStreamMode aMode,
        bool nonblocking)
{
    OSCL_UNUSED_ARG(nonblocking);
    int32 result = -1;
    LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::OpenSession"));

    // Check to see if we have an open file object.
    if (!iFileObject)
    {
        // Only open the file.
#if (defined(PVMF_PDL_DATASTREAM_ENABLE_READ_FILECACHE_DURING_PDL))
        iFileObject = OSCL_NEW(Oscl_File, (OSCL_FILE_BUFFER_MAX_SIZE, iFileHandle));
#else
        iFileObject = OSCL_NEW(Oscl_File, (0, iFileHandle));
#endif
        // Should have a file object now
        if (iFileObject)
        {
            switch (aMode)
            {
                case PVDS_READ_ONLY:
                {
                    result = iFileObject->Open(iFileName.get_cstr(), Oscl_File::MODE_READ, iFs);
                    LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::OpenSession - File opened returning %d iFileObject %x", result, iFileObject));
                    aSessionID = 0;
                    return (result == 0 ? PVDS_SUCCESS : PVDS_FAILURE);
                }
                // break;	This statement was removed to avoid compiler warning for Unreachable Code
                default:
                    // Attempt to open in an unsported mode.
                    // Have a iFileObject so clean it up and report PVDS_UNSUPPORTED_MODE
                    int32 result = iFileObject->Close();
                    if (result != 0)
                        LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::OpenSession PVDS_UNSUPPORTED_MODE returning %d", result));
                    OSCL_DELETE(iFileObject);
                    iFileObject = NULL;
                    return PVDS_UNSUPPORTED_MODE;
            }
        }
        else
            return PVDS_FAILURE; // Failed to create a file object.
    }
    // Already have a iFileObject, dont allow a second call to OpenSession.
    return PVDS_FAILURE;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferReadDataStreamImpl::CloseSession(PvmiDataStreamSession aSessionID)
{
    OSCL_UNUSED_ARG(aSessionID);
    if (!iFileObject)
    {
        // Either OpenSession was never called or we already called CloseSession
        // Tell the user and return failure.
        LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::CloseSession returning %d", -1));
        return PVDS_FAILURE;
    }

    // Have a iFileObject so cleanup.
    int32 result = iFileObject->Close();
    LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::CloseSession returning %d iFileObject %x", result, iFileObject));
    OSCL_DELETE(iFileObject);
    iFileObject = NULL;
    if (result == 0)
        return PVDS_SUCCESS;

    // If result != 0, then return failure
    return PVDS_FAILURE;
}

OSCL_EXPORT_REF PvmiDataStreamRandomAccessType
PVMFFileBufferReadDataStreamImpl::QueryRandomAccessCapability()
{
    return PVDS_FULL_RANDOM_ACCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferReadDataStreamImpl::QueryReadCapacity(PvmiDataStreamSession aSessionID,
        uint32& capacity)
{
    int32 result = -1;
    if (!iFileObject)
    {
        // No iFileObject to work with, return failure
        return PVDS_FAILURE;
    }
    // Get the current file position
    uint32 currFilePosition = GetCurrentPointerPosition(aSessionID);

    // for projects on Symbian using RFileBuf cache enabled
    // we need to reload the filecache in symbian, since oscl fileio
    // does not have a sync / reload API we cheat by calling flush
    // It so happens that the flush impl on symbian does a "synch"
    iFileObject->Flush();

    // since the behaviour of fflush is undefined for read-only files
    // file pos may not be preserved. So seek back
    iFileObject->Seek((int32)(currFilePosition), Oscl_File::SEEKSET);

    uint32 lastFilePosition = 0;
    // Determine the file size from write datastream.
    result = iWriteDataStream->QueryReadCapacity(iSessionID, lastFilePosition);
    if (result != 0)
    {
        LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::QueryReadCapacity - iWriteDataStream->QueryReadCapacity ret %d", result));
        return PVDS_FAILURE;
    }

    // Calculate the capacity from these two positions.
    capacity = (lastFilePosition - currFilePosition);

    if (iDownloadComplete == true)
    {
        return PVDS_END_OF_STREAM;
    }
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFFileBufferReadDataStreamImpl::RequestReadCapacityNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& observer,
        uint32 capacity,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(aSessionID);
    PvmiDataStreamStatus result;

    if (iDownloadComplete == true)
    {
        OSCL_LEAVE(OsclErrInvalidState);
    }

    // Check for an open session...
    if (iSessionID == 0)
    {
        // No open session, open one now
        result = iWriteDataStream->OpenSession(iSessionID, PVDS_READ_ONLY, false);

        if (result != PVDS_SUCCESS)
        {
            // No READ sessions left
            OSCL_LEAVE(OsclErrNoResources);
            return 0;
        }
    }

    //get current write datastream pos
    //the capacity passed here means that the entity using the read datastream
    //wants "capacity" number of bytes FROM its CURRENT location
    //Read datastream's current read location cannot exceed write datastream's
    //current location (cant read beyond what is written), therefore subtract
    //writedatastream's current position from capacity, before making the request
    uint32 currwritepos = iWriteDataStream->GetCurrentPointerPosition(iSessionID);
    uint32 currreadpos = GetCurrentPointerPosition(0);
    uint32 finalreadpositionforthisrequest = currreadpos + capacity;
    if (currwritepos >= finalreadpositionforthisrequest)
    {
        //this request should never have been sent
        //there is enough data in the datastream to be read
        OSCL_LEAVE(OsclErrArgument);
        return 0;
    }
    //these many bytes are yet to be written, so ask the writedatastream
    //to notify when they become available
    uint32 requestsize = finalreadpositionforthisrequest - currwritepos;

    PvmiDataStreamCommandId iCommandID = 0;
    int32 error = 0;

    // Trap the error from the RequestReadCapacityNotification
    OSCL_TRY(error, iCommandID = iWriteDataStream->RequestReadCapacityNotification(
                                     iSessionID,
                                     observer,
                                     requestsize,
                                     aContextData));
    if (error)
    {
        // If we have an error, report it to the observer.
        OSCL_LEAVE(error);
        return iCommandID;
    }

    // Else everything worked ok, return the command ID
    return iCommandID;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferReadDataStreamImpl::QueryWriteCapacity(PvmiDataStreamSession aSessionID,
        uint32& capacity)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(capacity);
    return PVDS_NOT_SUPPORTED;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFFileBufferReadDataStreamImpl::RequestWriteCapacityNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& observer,
        uint32 capacity,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(observer);
    OSCL_UNUSED_ARG(capacity);
    OSCL_UNUSED_ARG(aContextData);

    OSCL_LEAVE(OsclErrNotSupported);
    return 0;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFFileBufferReadDataStreamImpl::CancelNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& observer,
        PvmiDataStreamCommandId aID,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(aSessionID);

    PvmiDataStreamCommandId iCommandID = 0;
    int32 error = 0;

    // Trap the error from the CancelNotification
    OSCL_TRY(error, iCommandID = iWriteDataStream->CancelNotification(iSessionID,
                                 observer,
                                 aID,
                                 aContextData));
    if (error)
    {
        OSCL_LEAVE(error);
        // return iCommandID;	This statement was removed to avoid compiler warning for Unreachable Code
    }

    // No error return the Command ID
    return iCommandID;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferReadDataStreamImpl::CancelNotificationSync(PvmiDataStreamSession aSessionID)
{
    OSCL_UNUSED_ARG(aSessionID);

    return iWriteDataStream->CancelNotificationSync(iSessionID);
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferReadDataStreamImpl::Read(PvmiDataStreamSession aSessionID,
                                       uint8* buffer,
                                       uint32 size,
                                       uint32& numelements)
{
    OSCL_UNUSED_ARG(aSessionID);

    if (!iFileObject)
    {
        // No iFileObject to work with, return failure
        return PVDS_FAILURE;
    }

    // Read the requested information from the file.
    uint32 read_result = iFileObject->Read(buffer, size, numelements);
    //LOGDEBUG((0,"PVMFFileBufferReadDataStreamImpl::Read iFileObject %x request size %d numelems %d returning %d", iFileObject, size, numelements, read_result));
    numelements = read_result;

    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferReadDataStreamImpl::Write(PvmiDataStreamSession aSessionID,
                                        uint8* buffer,
                                        uint32 size,
                                        uint32& numelements)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(buffer);
    OSCL_UNUSED_ARG(size);
    OSCL_UNUSED_ARG(numelements);
    // Write not supported in the PVMFFileBufferReadDataStreamImpl object
    return PVDS_NOT_SUPPORTED;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferReadDataStreamImpl::Write(PvmiDataStreamSession aSessionID, OsclRefCounterMemFrag* aFrag, uint32& aNumElements)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(aFrag);
    OSCL_UNUSED_ARG(aNumElements);
    // Write not supported in the PVMFMemoryBufferReadDataStreamImpl object
    return PVDS_NOT_SUPPORTED;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferReadDataStreamImpl::Seek(PvmiDataStreamSession aSessionID,
                                       int32 offset,
                                       PvmiDataStreamSeekType origin)
{
    OSCL_UNUSED_ARG(aSessionID);

    if (!iFileObject)
        return PVDS_FAILURE;  // No iFileObject to work with, return failure

    // Peform the Seek the user requested
    Oscl_File::seek_type seekType = Oscl_File::SEEKSET;
    if (origin == PVDS_SEEK_SET)
    {
        seekType = Oscl_File::SEEKSET;
    }
    if (origin == PVDS_SEEK_CUR)
    {
        seekType = Oscl_File::SEEKCUR;
    }
    if (origin == PVDS_SEEK_END)
    {
        seekType = Oscl_File::SEEKEND;
    }
    int32 result = iFileObject->Seek(offset, seekType);
    LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::SeekContent iFileObject %x offset %d origin %d returning %d", iFileObject, offset, origin, result));
    if (result != 0)
    {
        return PVDS_FAILURE;
    }
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF uint32
PVMFFileBufferReadDataStreamImpl::GetCurrentPointerPosition(PvmiDataStreamSession aSessionID)
{
    OSCL_UNUSED_ARG(aSessionID);

    if (!iFileObject)
        return 0;  // No iFileObject to work with, return zero
    int32 result = (TOsclFileOffsetInt32)iFileObject->Tell();
    LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::GetCurrentContentPosition returning %d", result));
    return (uint32)(result);
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferReadDataStreamImpl::Flush(PvmiDataStreamSession aSessionID)
{
    OSCL_UNUSED_ARG(aSessionID);

    if (!iFileObject)
    {
        // No iFileObject to work with, return failure
        LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::Flush returning %d", -1));
        return PVDS_FAILURE;
    }

    // Perform the flush
    int32 result = iFileObject->Flush();
    if (result != 0)
        LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::Flush returning %d", result));
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF void
PVMFFileBufferReadDataStreamImpl::NotifyDownloadComplete()
{
    iDownloadComplete = true;
    if (iWriteDataStream != NULL)
    {
        iWriteDataStream->NotifyDownloadComplete();
    }
}

//////////////////////////////////////////////////////////////////////
// PVMFFileBufferWriteDataStreamImpl
//////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF
PVMFFileBufferWriteDataStreamImpl::PVMFFileBufferWriteDataStreamImpl(OSCL_wString& aFileName)
{
    iDownloadComplete = false;
    iFileHandle = NULL;
    iFileObject = NULL;
    iFileNumBytes = 0;
    iSessionID = 0;
    iFileName = aFileName;

    for (uint32 i = 0; i < MAX_NUMBER_OF_READ_CONNECTIONS; i++)
    {
        iReadNotifications[i].ReadStructValid = false;
    }
    iLastSessionID = 0;

    iFs.Connect();
    iLogger = PVLogger::GetLoggerObject("PVMFFileBufferWriteDataStreamImpl");
    LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::PVMFFileBufferWriteDataStreamImpl"));
}

OSCL_EXPORT_REF
PVMFFileBufferWriteDataStreamImpl::~PVMFFileBufferWriteDataStreamImpl()
{
    LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::~PVMFFileBufferWriteDataStreamImpl"));
    if (iFileObject)
        OSCL_DELETE(iFileObject);
    iFileObject = NULL;
    iLogger = NULL;
    iFs.Close();
}

OSCL_EXPORT_REF bool
PVMFFileBufferWriteDataStreamImpl::queryInterface(const PVUuid& uuid,
        PVInterface*& iface)
{
    iface = NULL;
    LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::queryInterface"));
    if (uuid == PVMIDataStreamSyncInterfaceUuid)
    {
        PVMIDataStreamSyncInterface* myInterface
        = OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return true;
    }
    return false;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferWriteDataStreamImpl::OpenSession(PvmiDataStreamSession& aSessionID,
        PvmiDataStreamMode aMode,
        bool nonblocking)
{
    OSCL_UNUSED_ARG(nonblocking);

    // This function assumes that the WRITE mode will be requested first!

    int32 result = -1;

    LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::OpenSession"));

    // Check to see if we have an open file object.
    if ((!iFileObject) && (aMode != PVDS_READ_ONLY))
    {
        // Only open the file.
        iFileObject = OSCL_NEW(Oscl_File, ());

        // Should have a file object now
        if (iFileObject)
        {
            //since we flush the file after each write, cache is not helpful.
            iFileObject->SetPVCacheSize(0);
            iFileObject->SetFileHandle(iFileHandle);
            //iFileObject->SetLoggingEnable(true);
            switch (aMode)
            {
                case PVDS_WRITE_ONLY:
                case PVDS_READ_WRITE:
                {
                    result = iFileObject->Open(iFileName.get_cstr(), Oscl_File::MODE_READWRITE, iFs);
                    LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::OpenSession - File opened returning %d", result));
                }
                break;
                case PVDS_READ_PLUS:
                {

                    result = iFileObject->Open(iFileName.get_cstr(), Oscl_File::MODE_READ_PLUS, iFs);
                    LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::OpenSession - File opened returning %d", result));

                }
                break;
                case PVDS_APPEND:
                {
                    result = iFileObject->Open(iFileName.get_cstr(), Oscl_File::MODE_APPEND, iFs);
                    LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::OpenSession - File opened returning %d", result));
                }
                break;
                case PVDS_REWRITE:
                {
                    result = iFs.Oscl_DeleteFile(iFileName.get_cstr());
                    LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::OpenSession - File deleted returning %d", result));
                    if (result != 0)
                    {
                        return PVDS_FAILURE;
                    }
                    result = iFileObject->Open(iFileName.get_cstr(), Oscl_File::MODE_READWRITE, iFs);
                    LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::OpenSession - File opened returning %d", result));
                }
                break;
                default:
                    // Attempt to open in an unsported mode.
                    // Have a iFileObject so clean it up and report PVDS_UNSUPPORTED_MODE
                    int32 result = iFileObject->Close();
                    if (result != 0)
                        LOGDEBUG((0, "PVMFFileBufferReadDataStreamImpl::OpenSession PVDS_UNSUPPORTED_MODE returning %d", result));
                    OSCL_DELETE(iFileObject);
                    iFileObject = NULL;
                    return PVDS_UNSUPPORTED_MODE;
            }
            // file has been opened- update byte count
            aSessionID = 0;
            if (result == 0)
            {
                const int32 filesize = (TOsclFileOffsetInt32)iFileObject->Size();
                if (filesize >= 0)
                    this->iFileNumBytes = filesize;
            }
            return (result == 0 ? PVDS_SUCCESS : PVDS_FAILURE);
        }
        else
            return PVDS_FAILURE; // Failed to create a file object.
    }

    if (aMode == PVDS_READ_ONLY)
    {
        // Check to see if we have free READ connections before setting one.
        if (iLastSessionID < MAX_NUMBER_OF_READ_CONNECTIONS)
        {
            iReadNotifications[iLastSessionID].ReadStructValid = true;
            iReadNotifications[iLastSessionID].iReadSessionID =
                iLastSessionID + MAX_NUMBER_OF_WRITE_CONNECTIONS;
            iReadNotifications[iLastSessionID].iReadObserver = NULL;
            iReadNotifications[iLastSessionID].iFilePosition = 0;
            iReadNotifications[iLastSessionID].iReadCapacity = 0;
            iReadNotifications[iLastSessionID].iContextData = NULL;
            iReadNotifications[iLastSessionID].iCommandID = 0;
            iReadNotifications[iLastSessionID].iCurrentCommandID = 0;
            aSessionID = iReadNotifications[iLastSessionID].iReadSessionID;
            iLastSessionID++;
            return PVDS_SUCCESS;
        }
    }

    // We got here becuase:
    // 1) We already have an iFileObject (IE we opened once WRITE mode)
    // 2) We have all the iReadNotifications filled.
    return PVDS_INVALID_REQUEST;

}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferWriteDataStreamImpl::CloseSession(PvmiDataStreamSession aSessionID)
{
    // Check for the WRITE session
    if (aSessionID == 0)
    {
        // Check to make sure we dont close the write session twice
        if (!iFileObject)
        {
            // Either OpenSession was never called or we already called CloseSession
            // Tell the user and return failure.
            LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::CloseSession no valid iFileObject returning %d", -1));
            return PVDS_FAILURE;
        }

        // Have a iFileObject so clean it up
        int32 result = iFileObject->Close();
        LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::CloseSession returning %d", result));
        OSCL_DELETE(iFileObject);
        iFileObject = NULL;
        if (result == 0)
            return PVDS_SUCCESS;

        // If result != 0, then return failure
        return PVDS_FAILURE;
    }
    else
    {
        // Close the READ sessions
        if ((aSessionID > (MAX_NUMBER_OF_READ_CONNECTIONS + MAX_NUMBER_OF_WRITE_CONNECTIONS)) ||
                (iReadNotifications[aSessionID-MAX_NUMBER_OF_WRITE_CONNECTIONS].ReadStructValid != true))
        {
            return PVDS_INVALID_SESSION;
        }

        // Have a valid READ session so close it by setting the flag to invalid
        iReadNotifications[aSessionID-MAX_NUMBER_OF_WRITE_CONNECTIONS].ReadStructValid = false;
        iLastSessionID--;
        return PVDS_SUCCESS;
    }
}

OSCL_EXPORT_REF PvmiDataStreamRandomAccessType
PVMFFileBufferWriteDataStreamImpl::QueryRandomAccessCapability()
{
    return PVDS_FULL_RANDOM_ACCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferWriteDataStreamImpl::QueryReadCapacity(PvmiDataStreamSession aSessionID,
        uint32& capacity)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(capacity);
    //return number of bytes written thus far
    capacity = iFileNumBytes;
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFFileBufferWriteDataStreamImpl::RequestReadCapacityNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& observer,
        uint32 capacity,
        OsclAny* aContextData)
{
    //  Check if the aSessionID is a valid SessionID
    //  Check that the aSessionID is not the WRITE session
    //  Check that the aSessionID is valid
    if ((aSessionID > (MAX_NUMBER_OF_READ_CONNECTIONS + MAX_NUMBER_OF_WRITE_CONNECTIONS)) ||
            (aSessionID == 0) ||
            (iReadNotifications[aSessionID-MAX_NUMBER_OF_WRITE_CONNECTIONS].ReadStructValid != true))
    {
        OSCL_LEAVE(OsclErrArgument);
        return 0;
    }
    else
    {
        // Read SessionID index is MAX_NUMBER_OF_WRITE_CONNECTIONS less than the aSessionID passed in
        PvmiDataStreamSession temp_session = aSessionID - MAX_NUMBER_OF_WRITE_CONNECTIONS;

        // Its valid, so save the read notification
        iReadNotifications[temp_session].iReadObserver = &observer;
        iReadNotifications[temp_session].iFilePosition = GetCurrentPointerPosition(0);
        iReadNotifications[temp_session].iReadCapacity = capacity;
        iReadNotifications[temp_session].iContextData = aContextData;
        iReadNotifications[temp_session].iCommandID =
            iReadNotifications[temp_session].iCurrentCommandID++;
        return iReadNotifications[temp_session].iCommandID;
    }
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferWriteDataStreamImpl::QueryWriteCapacity(PvmiDataStreamSession sessionID,
        uint32& capacity)
{
    OSCL_UNUSED_ARG(sessionID);
    capacity = 0xFFFFFFFF; // for file write, write capacity would be infinite.
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFFileBufferWriteDataStreamImpl::RequestWriteCapacityNotification(PvmiDataStreamSession sessionID,
        PvmiDataStreamObserver& observer,
        uint32 capacity,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(sessionID);
    OSCL_UNUSED_ARG(observer);
    OSCL_UNUSED_ARG(capacity);
    OSCL_UNUSED_ARG(aContextData);

    OSCL_LEAVE(OsclErrNotSupported);
    return 0;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFFileBufferWriteDataStreamImpl::CancelNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& observer,
        PvmiDataStreamCommandId aID,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(observer);
    OSCL_UNUSED_ARG(aID);
    OSCL_UNUSED_ARG(aContextData);

    //  Check if the aSessionID is a valid number
    //  Check that the aSessionID is not the WRITE session
    //  Check that the aSessionID is valid
    if ((aSessionID > (MAX_NUMBER_OF_READ_CONNECTIONS + MAX_NUMBER_OF_WRITE_CONNECTIONS)) ||
            (aSessionID == 0) ||
            (iReadNotifications[aSessionID-MAX_NUMBER_OF_WRITE_CONNECTIONS].ReadStructValid != true))
    {
        OSCL_LEAVE(OsclErrArgument);
    }

    PvmiDataStreamSession temp_sessionID = aSessionID - MAX_NUMBER_OF_WRITE_CONNECTIONS;
    // SessionID is valid, so zero out the notification info
    iReadNotifications[temp_sessionID].iReadObserver = NULL;
    iReadNotifications[temp_sessionID].iFilePosition = 0;
    iReadNotifications[temp_sessionID].iReadCapacity = 0;
    iReadNotifications[temp_sessionID].iContextData = NULL;
    iReadNotifications[temp_sessionID].iCommandID = 0;
    return iReadNotifications[temp_sessionID].iCurrentCommandID++;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferWriteDataStreamImpl::CancelNotificationSync(PvmiDataStreamSession aSessionID)
{
    //  Check if the aSessionID is a valid number
    //  Check that the aSessionID is not the WRITE session
    //  Check that the aSessionID is valid
    PvmiDataStreamStatus status = PVDS_SUCCESS;
    if ((aSessionID > (MAX_NUMBER_OF_READ_CONNECTIONS + MAX_NUMBER_OF_WRITE_CONNECTIONS)) ||
            (aSessionID == 0) ||
            (iReadNotifications[aSessionID - MAX_NUMBER_OF_WRITE_CONNECTIONS].ReadStructValid != true))
    {
        status = PVDS_FAILURE;
    }
    else
    {
        // zero out the notification info
        PvmiDataStreamSession temp_sessionID = aSessionID - MAX_NUMBER_OF_WRITE_CONNECTIONS;
        iReadNotifications[temp_sessionID].iReadObserver = NULL;
        iReadNotifications[temp_sessionID].iFilePosition = 0;
        iReadNotifications[temp_sessionID].iReadCapacity = 0;
        iReadNotifications[temp_sessionID].iContextData = NULL;
        iReadNotifications[temp_sessionID].iCommandID = 0;
    }
    return status;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferWriteDataStreamImpl::Read(PvmiDataStreamSession aSessionID,
                                        uint8* buffer,
                                        uint32 size,
                                        uint32& numelements)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(buffer);
    OSCL_UNUSED_ARG(size);
    OSCL_UNUSED_ARG(numelements);
    // Read not supported in the PVMFFileBufferWriteDataStreamImpl object
    return PVDS_NOT_SUPPORTED;
}


OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferWriteDataStreamImpl::Write(PvmiDataStreamSession aSessionID, OsclRefCounterMemFrag* aFrag, uint32& aNumElements)
{
    aNumElements = aFrag->getMemFragSize();
    return Write(aSessionID, (uint8*)(aFrag->getMemFragPtr()), sizeof(uint8), aNumElements);
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferWriteDataStreamImpl::Write(PvmiDataStreamSession aSessionID,
        uint8* buffer,
        uint32 size,
        uint32& numelements)
{
    if ((!iFileObject) || (aSessionID != 0))
    {
        // No iFileObject or valid write session to work with, return failure
        return PVDS_FAILURE;
    }

    uint32 result = iFileObject->Write(buffer, size, numelements);
    Flush(0);
    iFileNumBytes += (size * numelements);

//	LOGDEBUG((0,"PVMFFileBufferWriteDataStreamImpl::Write returning %d",result));
    numelements = result;

    // Loop through the iReadNotifications for a Read Notification
    for (uint32 i = 0; i < MAX_NUMBER_OF_READ_CONNECTIONS; i++)
    {
        // If we have a valid iReadNotifications element
        //  AND the iReadObserver != NULL
        if ((iReadNotifications[i].ReadStructValid == true) &&
                (iReadNotifications[i].iReadObserver != NULL))
        {
            uint32 currFilePosition = GetCurrentPointerPosition(0);

            PVMFStatus status;
            if ((currFilePosition -
                    iReadNotifications[i].iFilePosition) >
                    iReadNotifications[i].iReadCapacity)
            {
                status = PVMFSuccess;
                PvmiDataStreamObserver* copy_observer = iReadNotifications[i].iReadObserver;
                OsclAny* copy_ContextData = iReadNotifications[i].iContextData;
                PvmiDataStreamCommandId copy_aID = iReadNotifications[i].iCommandID;

                // Reset the iReadNotifications for the next notification
                iReadNotifications[i].iReadObserver = NULL;
                iReadNotifications[i].iReadCapacity = 0;
                iReadNotifications[i].iFilePosition = 0;
                iReadNotifications[i].iCommandID = 0;
                iReadNotifications[i].iContextData = NULL;

                // Form a command response.
                PVMFCmdResp resp(copy_aID,
                                 copy_ContextData,
                                 status,
                                 NULL,
                                 NULL);

                // Make the Command Complete notification.
                copy_observer->DataStreamCommandCompleted(resp);
            }
            else
            {
                if (iDownloadComplete == true)
                {
                    status = PVMFFailure;
                    PvmiDataStreamObserver* copy_observer = iReadNotifications[i].iReadObserver;
                    OsclAny* copy_ContextData = iReadNotifications[i].iContextData;
                    PvmiDataStreamCommandId copy_aID = iReadNotifications[i].iCommandID;

                    // Reset the iReadNotifications for the next notification
                    iReadNotifications[i].iReadObserver = NULL;
                    iReadNotifications[i].iReadCapacity = 0;
                    iReadNotifications[i].iFilePosition = 0;
                    iReadNotifications[i].iCommandID = 0;
                    iReadNotifications[i].iContextData = NULL;

                    // Form a command response.
                    PVMFCmdResp resp(copy_aID,
                                     copy_ContextData,
                                     status,
                                     NULL,
                                     NULL);

                    // Make the Command Complete notification.
                    copy_observer->DataStreamCommandCompleted(resp);
                }
            }
        }
    }

    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferWriteDataStreamImpl::Seek(PvmiDataStreamSession aSessionID,
                                        int32 offset,
                                        PvmiDataStreamSeekType origin)
{
    OSCL_UNUSED_ARG(aSessionID);

    if (!iFileObject)
        return PVDS_FAILURE;  // No iFileObject to work with, return failure

    // Peform the Seek the user requested
    Oscl_File::seek_type seekType = Oscl_File::SEEKSET;
    if (origin == PVDS_SEEK_SET)
    {
        seekType = Oscl_File::SEEKSET;
    }
    if (origin == PVDS_SEEK_CUR)
    {
        seekType = Oscl_File::SEEKCUR;
    }
    if (origin == PVDS_SEEK_END)
    {
        seekType = Oscl_File::SEEKEND;
    }
    int32 result = iFileObject->Seek(offset, seekType);
    LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::SeekContent returning %d", result));
    if (result != 0)
    {
        return PVDS_FAILURE;
    }
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF uint32
PVMFFileBufferWriteDataStreamImpl::GetCurrentPointerPosition(PvmiDataStreamSession aSessionID)
{
    OSCL_UNUSED_ARG(aSessionID);

    if (!iFileObject)
        return 0;  // No iFileObject to work with, return zero
    int32 result = (TOsclFileOffsetInt32)iFileObject->Tell();
    LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::GetCurrentContentPosition returning %d", result));
    return (uint32)(result);
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFFileBufferWriteDataStreamImpl::Flush(PvmiDataStreamSession aSessionID)
{
    OSCL_UNUSED_ARG(aSessionID);

    if (!iFileObject)
    {
        // No iFileObject to work with, return failure
        LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::Flush returning %d", -1));
        return PVDS_FAILURE;
    }

    // Perform the flush
    int32 result = iFileObject->Flush();
    if (result != 0)
        LOGDEBUG((0, "PVMFFileBufferWriteDataStreamImpl::Flush returning %d", result));
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF void
PVMFFileBufferWriteDataStreamImpl::NotifyDownloadComplete()
{
    iDownloadComplete = true;
    // Loop through the iReadNotifications for a Read Notification
    for (uint32 i = 0; i < MAX_NUMBER_OF_READ_CONNECTIONS; i++)
    {
        // If we have a valid iReadNotifications element
        //  AND the iReadObserver != NULL
        if ((iReadNotifications[i].ReadStructValid == true) &&
                (iReadNotifications[i].iReadObserver != NULL))
        {
            uint32 currFilePosition = GetCurrentPointerPosition(0);

            PVMFStatus status;
            if ((currFilePosition -
                    iReadNotifications[i].iFilePosition) >
                    iReadNotifications[i].iReadCapacity)
            {
                status = PVMFSuccess;
            }
            else
            {
                //fail the request - no more data is possible
                status = PVMFFailure;
            }
            PvmiDataStreamObserver* copy_observer = iReadNotifications[i].iReadObserver;
            OsclAny* copy_ContextData = iReadNotifications[i].iContextData;
            PvmiDataStreamCommandId copy_aID = iReadNotifications[i].iCommandID;

            // Reset the iReadNotifications for the next notification
            iReadNotifications[i].iReadObserver = NULL;
            iReadNotifications[i].iReadCapacity = 0;
            iReadNotifications[i].iFilePosition = 0;
            iReadNotifications[i].iCommandID = 0;
            iReadNotifications[i].iContextData = NULL;

            // Form a command response.
            PVMFCmdResp resp(copy_aID,
                             copy_ContextData,
                             status,
                             NULL,
                             NULL);

            // Make the Command Complete notification.
            copy_observer->DataStreamCommandCompleted(resp);
        }
    }
}


//////////////////////////////////////////////////////////////////////
// PVMFFileBufferDataStream
//////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF
PVMFFileBufferDataStream::PVMFFileBufferDataStream(OSCL_wString& aFileName)
{
    // Create the two factories
    iWriteDataStreamFactory = OSCL_NEW(PVMFFileBufferDataStreamWriteDataStreamFactoryImpl, (aFileName));
    iReadDataStreamFactory = OSCL_NEW(PVMFFileBufferDataStreamReadDataStreamFactoryImpl, (aFileName));

    // Now create a iWriteDataStream and set the pointer in the iReadDataStreamFactory
    PVUuid uuid = PVMIDataStreamSyncInterfaceUuid;
    iWriteDataStream = iWriteDataStreamFactory->CreatePVMFCPMPluginAccessInterface(uuid);
    iReadDataStreamFactory->SetWriteDataStreamPtr(iWriteDataStream);
}

OSCL_EXPORT_REF
PVMFFileBufferDataStream::~PVMFFileBufferDataStream()
{
    // Destroy the instance of the iWriteDataStream
    //OSCL_DELETE(iWriteDataStream);
    //iWriteDataStream = NULL;

    // Delete the two DataStreamFactories
    OSCL_DELETE(iWriteDataStreamFactory);
    OSCL_DELETE(iReadDataStreamFactory);
}

OSCL_EXPORT_REF PVMFDataStreamFactory*
PVMFFileBufferDataStream::GetReadDataStreamFactoryPtr()
{
    return OSCL_STATIC_CAST(PVMFDataStreamFactory*, iReadDataStreamFactory);
}

OSCL_EXPORT_REF PVMFDataStreamFactory*
PVMFFileBufferDataStream::GetWriteDataStreamFactoryPtr()
{
    return OSCL_STATIC_CAST(PVMFDataStreamFactory*, iWriteDataStreamFactory);
}

OSCL_EXPORT_REF void
PVMFFileBufferDataStream::NotifyDownloadComplete()
{
    if (iReadDataStreamFactory != NULL)
    {
        iReadDataStreamFactory->NotifyDownloadComplete();
    }
}










