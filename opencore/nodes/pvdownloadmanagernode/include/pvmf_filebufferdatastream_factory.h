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
#ifndef PVMF_FILEBUFFERDATASTREAM_FACTORY_H_INCLUDED
#define PVMF_FILEBUFFERDATASTREAM_FACTORY_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
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

#define MAX_NUMBER_OF_READ_CONNECTIONS 4
#define MAX_NUMBER_OF_WRITE_CONNECTIONS 1
#define MAX_NUMBER_OF_TOTAL_CONNECTIONS  MAX_NUMBER_OF_WRITE_CONNECTIONS+MAX_NUMBER_OF_READ_CONNECTIONS

class PVMFFileBufferWriteDataStreamImpl;
class PVMFFileBufferReadDataStreamImpl;

//////////////////////////////////////////////////////////////////////
// PVMFFileBufferDataStreamReadDataStreamFactoryImpl
//////////////////////////////////////////////////////////////////////
class PVMFFileBufferDataStreamReadDataStreamFactoryImpl : public PVMFDataStreamFactory
{
    public:
        OSCL_IMPORT_REF PVMFFileBufferDataStreamReadDataStreamFactoryImpl(OSCL_wString& aFileName);

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

        OSCL_wHeapString<OsclMemAllocator> iFileName;
        PVMFFileBufferWriteDataStreamImpl *iWriteDataStream;

        bool iDownloadComplete;
        Oscl_Vector<PVMFFileBufferReadDataStreamImpl*, OsclMemAllocator> iReadStreamVec;
};


//////////////////////////////////////////////////////////////////////
// PVMFFileBufferDataStreamWriteDataStreamFactoryImpl
//////////////////////////////////////////////////////////////////////
class PVMFFileBufferDataStreamWriteDataStreamFactoryImpl : public PVMFDataStreamFactory,
            public PVMFDataStreamReadCapacityObserver
{
    public:
        OSCL_IMPORT_REF PVMFFileBufferDataStreamWriteDataStreamFactoryImpl(OSCL_wString& aFileName);
        OSCL_IMPORT_REF ~PVMFFileBufferDataStreamWriteDataStreamFactoryImpl();

        OSCL_IMPORT_REF PVMFStatus QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids);
        OSCL_IMPORT_REF PVInterface* CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid);
        OSCL_IMPORT_REF void DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid, PVInterface* aPtr);

        virtual PvmiDataStreamStatus GetStreamReadCapacity(uint32& aCapacity);

    private:
        void addRef() {};
        void removeRef() {};
        bool queryInterface(const PVUuid&, PVInterface*&)
        {
            return false;
        };

        OSCL_wHeapString<OsclMemAllocator> iFileName;
        PVMFFileBufferWriteDataStreamImpl *iWriteDataStream;
        bool iDownloadComplete;
};


//////////////////////////////////////////////////////////////////////
// PVMFFileBufferReadDataStreamImpl
//////////////////////////////////////////////////////////////////////
class PVMFFileBufferReadDataStreamImpl : public PVMIDataStreamSyncInterface
{
    public:
        OSCL_IMPORT_REF PVMFFileBufferReadDataStreamImpl(PVMFFileBufferWriteDataStreamImpl* aWriteDataStream,
                OSCL_wString& aFileName);
        OSCL_IMPORT_REF ~PVMFFileBufferReadDataStreamImpl();

        // From PVInterface
        void addRef() {};
        void removeRef() {};

        OSCL_IMPORT_REF bool queryInterface(const PVUuid& uuid,
                                            PVInterface*& iface);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus OpenSession(PvmiDataStreamSession& aSessionID,
                                         PvmiDataStreamMode aMode,
                                         bool nonblocking = false);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus CloseSession(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF
        PvmiDataStreamRandomAccessType QueryRandomAccessCapability();

        OSCL_IMPORT_REF
        PvmiDataStreamStatus QueryReadCapacity(PvmiDataStreamSession aSessionID,
                                               uint32& capacity);

        OSCL_IMPORT_REF
        PvmiDataStreamCommandId RequestReadCapacityNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& observer,
                uint32 capacity,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus QueryWriteCapacity(PvmiDataStreamSession aSessionID,
                                                uint32& capacity);

        OSCL_IMPORT_REF
        PvmiDataStreamCommandId RequestWriteCapacityNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& observer,
                uint32 capacity,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF
        PvmiDataStreamCommandId CancelNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& observer,
                PvmiDataStreamCommandId aID,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus CancelNotificationSync(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus Read(PvmiDataStreamSession aSessionID,
                                  uint8* buffer,
                                  uint32 size,
                                  uint32& numelements);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus Write(PvmiDataStreamSession aSessionID,
                                   uint8* buffer,
                                   uint32 size,
                                   uint32& numelements);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus Write(PvmiDataStreamSession aSessionID,
                                   OsclRefCounterMemFrag* frag,
                                   uint32& aNumElements);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus Seek(PvmiDataStreamSession aSessionID,
                                  int32 offset,
                                  PvmiDataStreamSeekType origin);

        OSCL_IMPORT_REF
        uint32 GetCurrentPointerPosition(PvmiDataStreamSession aSessionID) ;

        OSCL_IMPORT_REF
        PvmiDataStreamStatus Flush(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF void NotifyDownloadComplete();

        void SetContentLength(uint32 aContentLength)
        {
            OSCL_UNUSED_ARG(aContentLength);
        }

        uint32 GetContentLength()
        {
            return 0;
        }

        uint32 QueryBufferingCapacity()
        {
            return 0;
        }

        PvmiDataStreamStatus SetSourceRequestObserver(PvmiDataStreamRequestObserver& aObserver)
        {
            OSCL_UNUSED_ARG(aObserver);
            return PVDS_NOT_SUPPORTED;
        }

        void SourceRequestCompleted(const PVMFCmdResp& aResponse)
        {
            OSCL_UNUSED_ARG(aResponse);
        }

        PvmiDataStreamStatus MakePersistent(int32 aOffset, uint32 aSize)
        {
            OSCL_UNUSED_ARG(aOffset);
            OSCL_UNUSED_ARG(aSize);
            return PVDS_NOT_SUPPORTED;
        }

    public:
        bool iDownloadComplete;

    private:
        PVMFFileBufferWriteDataStreamImpl* iWriteDataStream;

        Oscl_File *iFileObject;
        OsclFileHandle* iFileHandle;
        Oscl_FileServer iFs;
        OSCL_wHeapString<OsclMemAllocator> iFileName;

        PvmiDataStreamSession iSessionID;

        int32 iFileNumBytes;

        PVLogger* iLogger;
};


//////////////////////////////////////////////////////////////////////
// PVMFFileBufferWriteDataStreamImpl
//////////////////////////////////////////////////////////////////////
class PVMFFileBufferWriteDataStreamImpl : public PVMIDataStreamSyncInterface
{
    public:
        OSCL_IMPORT_REF PVMFFileBufferWriteDataStreamImpl(OSCL_wString& aFileName);
        OSCL_IMPORT_REF ~PVMFFileBufferWriteDataStreamImpl();

        // From PVInterface
        void addRef() {};

        void removeRef() {};

        OSCL_IMPORT_REF bool queryInterface(const PVUuid& uuid,
                                            PVInterface*& iface);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus OpenSession(PvmiDataStreamSession& aSessionID,
                                         PvmiDataStreamMode aMode,
                                         bool nonblocking = false);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus CloseSession(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF
        PvmiDataStreamRandomAccessType QueryRandomAccessCapability();

        OSCL_IMPORT_REF
        PvmiDataStreamStatus QueryReadCapacity(PvmiDataStreamSession aSessionID,
                                               uint32& capacity);

        OSCL_IMPORT_REF
        PvmiDataStreamCommandId RequestReadCapacityNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& observer,
                uint32 capacity,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus QueryWriteCapacity(PvmiDataStreamSession aSessionID,
                                                uint32& capacity);

        OSCL_IMPORT_REF
        PvmiDataStreamCommandId RequestWriteCapacityNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& observer,
                uint32 capacity,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF
        PvmiDataStreamCommandId CancelNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& observer,
                PvmiDataStreamCommandId aID,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus CancelNotificationSync(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus Read(PvmiDataStreamSession aSessionID,
                                  uint8* buffer,
                                  uint32 size,
                                  uint32& numelements);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus Write(PvmiDataStreamSession aSessionID,
                                   uint8* buffer,
                                   uint32 size,
                                   uint32& numelements);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus Write(PvmiDataStreamSession aSessionID,
                                   OsclRefCounterMemFrag* frag,
                                   uint32& aNumElements);

        OSCL_IMPORT_REF
        PvmiDataStreamStatus Seek(PvmiDataStreamSession aSessionID,
                                  int32 offset,
                                  PvmiDataStreamSeekType origin);

        OSCL_IMPORT_REF
        uint32 GetCurrentPointerPosition(PvmiDataStreamSession aSessionID) ;

        OSCL_IMPORT_REF
        PvmiDataStreamStatus Flush(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF void NotifyDownloadComplete();

        void SetContentLength(uint32 aContentLength)
        {
            OSCL_UNUSED_ARG(aContentLength);
        }

        uint32 GetContentLength()
        {
            return 0;
        }

        uint32 QueryBufferingCapacity()
        {
            return 0;
        }

        PvmiDataStreamStatus SetSourceRequestObserver(PvmiDataStreamRequestObserver& aObserver)
        {
            OSCL_UNUSED_ARG(aObserver);
            return PVDS_NOT_SUPPORTED;
        }

        void SourceRequestCompleted(const PVMFCmdResp& aResponse)
        {
            OSCL_UNUSED_ARG(aResponse);
        }

        PvmiDataStreamStatus MakePersistent(int32 aOffset, uint32 aSize)
        {
            OSCL_UNUSED_ARG(aOffset);
            OSCL_UNUSED_ARG(aSize);
            return PVDS_NOT_SUPPORTED;
        }

    public:
        bool iDownloadComplete;

    private:

        struct ReadCapacityNotificationStruct
        {
            bool ReadStructValid;
            PvmiDataStreamSession iReadSessionID;
            PvmiDataStreamObserver *iReadObserver;
            uint32 iFilePosition;
            uint32 iReadCapacity;
            OsclAny* iContextData;
            PVMFCommandId iCommandID;
            PVMFCommandId iCurrentCommandID;
        };
        PvmiDataStreamSession iLastSessionID;
        ReadCapacityNotificationStruct iReadNotifications[MAX_NUMBER_OF_READ_CONNECTIONS];

        Oscl_File *iFileObject;
        OsclFileHandle* iFileHandle;
        Oscl_FileServer iFs;
        OSCL_wHeapString<OsclMemAllocator> iFileName;

        PvmiDataStreamSession iSessionID;

        int32 iFileNumBytes;

        PVLogger* iLogger;
};


//////////////////////////////////////////////////////////////////////
// PVMFFileBufferWriteDataStreamImpl
//////////////////////////////////////////////////////////////////////
class PVMFFileBufferDataStream
{
    public:
        OSCL_IMPORT_REF PVMFFileBufferDataStream(OSCL_wString& aFileName);
        OSCL_IMPORT_REF ~PVMFFileBufferDataStream();

        OSCL_IMPORT_REF
        PVMFDataStreamFactory* GetReadDataStreamFactoryPtr();

        OSCL_IMPORT_REF
        PVMFDataStreamFactory* GetWriteDataStreamFactoryPtr();

        OSCL_IMPORT_REF void NotifyDownloadComplete();

    private:
        PVMFFileBufferDataStreamReadDataStreamFactoryImpl* iReadDataStreamFactory;
        PVMFFileBufferDataStreamWriteDataStreamFactoryImpl* iWriteDataStreamFactory;
        PVInterface* iWriteDataStream;
        OSCL_wHeapString<OsclMemAllocator> iFileName;

};

#endif // PVMF_FILEBUFFERDATASTREAM_FACTORY_H_INCLUDED



