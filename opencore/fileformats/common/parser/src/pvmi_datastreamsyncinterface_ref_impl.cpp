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
#ifndef PVMI_DATASTREAMSYNCINTERFACE_REF_IMPL_H_INCLUDED
#include "pvmi_datastreamsyncinterface_ref_impl.h"
#endif

#define LOGDEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_DEBUG,m);


OSCL_EXPORT_REF PVMIDataStreamSyncInterface* PVMIDataStreamSyncInterfaceRefImpl::CreateDataStreamSyncInterfaceRefImpl(OSCL_wString& aFileName)
{
    PVMIDataStreamSyncInterfaceRefImpl* newsyncif = OSCL_NEW(PVMIDataStreamSyncInterfaceRefImpl, (aFileName));
    return OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, newsyncif);
}


OSCL_EXPORT_REF PVMIDataStreamSyncInterface* PVMIDataStreamSyncInterfaceRefImpl::CreateDataStreamSyncInterfaceRefImpl(OsclFileHandle* aFileHandle)
{
    PVMIDataStreamSyncInterfaceRefImpl* newsyncif = OSCL_NEW(PVMIDataStreamSyncInterfaceRefImpl, (aFileHandle));
    return OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, newsyncif);
}


OSCL_EXPORT_REF void PVMIDataStreamSyncInterfaceRefImpl::DestroyDataStreamSyncInterfaceRefImpl(PVMIDataStreamSyncInterface* aInterface)
{
    PVMIDataStreamSyncInterfaceRefImpl* syncif = OSCL_STATIC_CAST(PVMIDataStreamSyncInterfaceRefImpl*, aInterface);
    OSCL_DELETE(syncif);
}


PVMIDataStreamSyncInterfaceRefImpl::PVMIDataStreamSyncInterfaceRefImpl(OSCL_wString& aFileName)
{
    iSessionID = 0;
    iFileName = aFileName;
    iFileHandle = NULL;
    iFileObject = NULL;
    iLogger = PVLogger::GetLoggerObject("PVMIDataStreamSyncInterfaceRefImpl");
    iFs.Connect();
    LOGDEBUG((0, "PVMIDataStreamSyncInterfaceRefImpl::PVMIDataStreamSyncInterfaceRefImpl"));
}

PVMIDataStreamSyncInterfaceRefImpl::PVMIDataStreamSyncInterfaceRefImpl(OsclFileHandle* aFileHandle)
{
    iSessionID = 0;
    iFileHandle = aFileHandle;
    iFileObject = NULL;
    iLogger = PVLogger::GetLoggerObject("PVMIDataStreamSyncInterfaceRefImpl");
    iFs.Connect();
    LOGDEBUG((0, "PVMIDataStreamSyncInterfaceRefImpl::PVMIDataStreamSyncInterfaceRefImpl"));
}

PVMIDataStreamSyncInterfaceRefImpl::~PVMIDataStreamSyncInterfaceRefImpl()
{
    LOGDEBUG((0, "PVMIDataStreamSyncInterfaceRefImpl::~PVMIDataStreamSyncInterfaceRefImpl"));
    if (iFileObject)
        OSCL_DELETE(iFileObject);
    iFileObject = NULL;
    iFs.Close();
    iLogger = NULL;
}

OSCL_EXPORT_REF bool
PVMIDataStreamSyncInterfaceRefImpl::queryInterface(const PVUuid& uuid,
        PVInterface*& iface)
{
    iface = NULL;
    LOGDEBUG((0, "PVMIDataStreamSyncInterfaceRefImpl::queryInterface"));
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
PVMIDataStreamSyncInterfaceRefImpl::OpenSession(PvmiDataStreamSession& aSessionID,
        PvmiDataStreamMode aMode,
        bool nonblocking)
{
    OSCL_UNUSED_ARG(nonblocking);

    LOGDEBUG((0, "PVMIDataStreamSyncInterfaceRefImpl::OpenSession"));
    if (!iFileObject)
        iFileObject = OSCL_NEW(Oscl_File, (OSCL_FILE_BUFFER_MAX_SIZE, iFileHandle));

    int32 result;
    if (iFileHandle)
    {
        result = 0;
    }
    else
    {
        if (aMode == PVDS_READ_ONLY)
        {
            result = iFileObject->Open(iFileName.get_cstr(), Oscl_File::MODE_READ, iFs);
            LOGDEBUG((0, "PVMIDataStreamSyncInterfaceRefImpl::OpenSession - File Open returning %d", result));
        }
        else
        {
            return PVDS_UNSUPPORTED_MODE;
        }
    }

    if (result == 0)
    {
        iFileNumBytes = 0;
        int32 res = iFileObject->Seek(0, Oscl_File::SEEKEND);
        if (res == 0)
        {
            iFileNumBytes = (TOsclFileOffsetInt32)iFileObject->Tell();
            iFileObject->Seek(0, Oscl_File::SEEKSET);
        }
        aSessionID = iSessionID;
        return PVDS_SUCCESS;
    }
    return PVDS_FAILURE;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIDataStreamSyncInterfaceRefImpl::CloseSession(PvmiDataStreamSession sessionID)
{
    OSCL_UNUSED_ARG(sessionID);
    if (!iFileObject)
    {
        LOGDEBUG((0, "PVMIDataStreamSyncInterfaceRefImpl::CloseSession returning %d", -1));
        return PVDS_FAILURE;
    }
    int32 result = 0;
    if (!iFileHandle)
        result = iFileObject->Close();
    LOGDEBUG((0, "PVMIDataStreamSyncInterfaceRefImpl::CloseSession returning %d", result));
    OSCL_DELETE(iFileObject);
    iFileObject = NULL;
    if (result == 0)
        return PVDS_SUCCESS;

    return PVDS_FAILURE;
}

OSCL_EXPORT_REF PvmiDataStreamRandomAccessType
PVMIDataStreamSyncInterfaceRefImpl::QueryRandomAccessCapability()
{
    return PVDS_FULL_RANDOM_ACCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIDataStreamSyncInterfaceRefImpl::QueryReadCapacity(PvmiDataStreamSession sessionID,
        uint32& capacity)
{
    OSCL_UNUSED_ARG(sessionID);

    uint32 currFilePosition = GetCurrentPointerPosition(iSessionID);
    capacity = ((uint32)iFileNumBytes - currFilePosition);
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMIDataStreamSyncInterfaceRefImpl::RequestReadCapacityNotification(PvmiDataStreamSession sessionID,
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

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIDataStreamSyncInterfaceRefImpl::QueryWriteCapacity(PvmiDataStreamSession sessionID,
        uint32& capacity)
{
    OSCL_UNUSED_ARG(sessionID);
    OSCL_UNUSED_ARG(capacity);

    return PVDS_NOT_SUPPORTED;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMIDataStreamSyncInterfaceRefImpl::RequestWriteCapacityNotification(PvmiDataStreamSession sessionID,
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
PVMIDataStreamSyncInterfaceRefImpl::CancelNotification(PvmiDataStreamSession sessionID,
        PvmiDataStreamObserver& observer,
        PvmiDataStreamCommandId aID,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(sessionID);
    OSCL_UNUSED_ARG(observer);
    OSCL_UNUSED_ARG(aID);
    OSCL_UNUSED_ARG(aContextData);

    OSCL_LEAVE(OsclErrNotSupported);
    return 0;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIDataStreamSyncInterfaceRefImpl::Read(PvmiDataStreamSession sessionID,
        uint8* buffer,
        uint32 size,
        uint32& numelements)
{
    OSCL_UNUSED_ARG(sessionID);

    if (!iFileObject)
        return PVDS_FAILURE;
    uint32 result = iFileObject->Read(buffer, size, numelements);
    LOGDEBUG((0, "PVMIDataStreamSyncInterfaceRefImpl::ReadAndUnlockContent returning %d", result));
    numelements = result;
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIDataStreamSyncInterfaceRefImpl::Write(PvmiDataStreamSession sessionID,
        uint8* buffer,
        uint32 size,
        uint32& numelements)
{
    OSCL_UNUSED_ARG(sessionID);
    OSCL_UNUSED_ARG(buffer);
    OSCL_UNUSED_ARG(size);
    OSCL_UNUSED_ARG(numelements);

    return PVDS_NOT_SUPPORTED;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIDataStreamSyncInterfaceRefImpl::Seek(PvmiDataStreamSession sessionID,
        int32 offset,
        PvmiDataStreamSeekType origin)
{
    OSCL_UNUSED_ARG(sessionID);

    if (!iFileObject)
        return PVDS_FAILURE;
    Oscl_File::seek_type seekType = Oscl_File::SEEKCUR;
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
    LOGDEBUG((0, "PVMIDataStreamSyncInterfaceRefImpl::SeekContent returning %d", result));
    if (result != 0)
    {
        return PVDS_FAILURE;
    }
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF uint32
PVMIDataStreamSyncInterfaceRefImpl::GetCurrentPointerPosition(PvmiDataStreamSession sessionID)
{
    OSCL_UNUSED_ARG(sessionID);

    if (!iFileObject)
        return 0;
    int32 result = (TOsclFileOffsetInt32)iFileObject->Tell();
    LOGDEBUG((0, "PVMIDataStreamSyncInterfaceRefImpl::GetCurrentContentPosition returning %d", result));
    return (uint32)(result);
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIDataStreamSyncInterfaceRefImpl::Flush(PvmiDataStreamSession sessionID)
{
    OSCL_UNUSED_ARG(sessionID);

    if (!iFileObject)
    {
        LOGDEBUG((0, "PVMIDataStreamSyncInterfaceRefImpl::Flush returning %d", -1));
        return PVDS_FAILURE;
    }
    int32 result;
    result = iFileObject->Flush();
    LOGDEBUG((0, "PVMIDataStreamSyncInterfaceRefImpl::Flush returning %d", result));
    if (result == 0) //Flush will return 0 when successful
        return PVDS_SUCCESS;
    else
        return PVDS_FAILURE;

}


