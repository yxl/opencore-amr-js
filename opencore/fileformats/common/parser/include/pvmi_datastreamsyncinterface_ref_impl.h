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
#define PVMI_DATASTREAMSYNCINTERFACE_REF_IMPL_H_INCLUDED

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
#ifndef PVMI_DATA_STREAM_INTERFACE_H_INCLUDED
#include "pvmi_data_stream_interface.h"
#endif

class PVMIDataStreamSyncInterfaceRefImpl : public PVMIDataStreamSyncInterface
{
    public:
        OSCL_IMPORT_REF static PVMIDataStreamSyncInterface* CreateDataStreamSyncInterfaceRefImpl(OSCL_wString& aFileName);
        OSCL_IMPORT_REF static PVMIDataStreamSyncInterface* CreateDataStreamSyncInterfaceRefImpl(OsclFileHandle* aFileHandle);
        OSCL_IMPORT_REF static void DestroyDataStreamSyncInterfaceRefImpl(PVMIDataStreamSyncInterface* aInterface);

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

        PvmiDataStreamStatus CancelNotificationSync(PvmiDataStreamSession aSessionID)
        {
            OSCL_UNUSED_ARG(aSessionID);
            return PVDS_NOT_SUPPORTED;
        }

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
        PvmiDataStreamStatus Write(PvmiDataStreamSession aSessionID,
                                   OsclRefCounterMemFrag* aFrag,
                                   uint32& numelements)
        {
            OSCL_UNUSED_ARG(aSessionID);
            OSCL_UNUSED_ARG(aFrag);
            OSCL_UNUSED_ARG(numelements);
            return PVDS_NOT_SUPPORTED;
        }

        OSCL_IMPORT_REF
        PvmiDataStreamStatus Seek(PvmiDataStreamSession aSessionID,
                                  int32 offset,
                                  PvmiDataStreamSeekType origin);

        OSCL_IMPORT_REF
        uint32 GetCurrentPointerPosition(PvmiDataStreamSession aSessionID) ;

        OSCL_IMPORT_REF
        PvmiDataStreamStatus Flush(PvmiDataStreamSession aSessionID);

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


    private:
        PVMIDataStreamSyncInterfaceRefImpl(OSCL_wString& aFileName);
        PVMIDataStreamSyncInterfaceRefImpl(OsclFileHandle* aFileHandle);
        virtual ~PVMIDataStreamSyncInterfaceRefImpl();

        PvmiDataStreamSession iSessionID;
        OsclFileHandle* iFileHandle;
        OSCL_wHeapString<OsclMemAllocator> iFileName;
        Oscl_File *iFileObject;
        int32 iFileNumBytes;
        Oscl_FileServer iFs;
        PVLogger* iLogger;
};

#endif //PVMI_DATASTREAMSYNCINTERFACE_REF_IMPL_H_INCLUDED


