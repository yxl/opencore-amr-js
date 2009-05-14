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
#ifndef PVMF_CPMPLUGIN_PASSTHRU_OMA1_H_INCLUDED
#define PVMF_CPMPLUGIN_PASSTHRU_OMA1_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVMF_CPMPLUGIN_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_interface.h"
#endif
#ifndef PVMF_CPMPLUGIN_AUTHORIZATION_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_authorization_interface.h"
#endif
#ifndef PVMF_CPMPLUGIN_AUTHENTICATION_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_authentication_interface.h"
#endif
#ifndef PVMF_CPMPLUGIN_ACCESS_INTERFACE_FACTORY_H_INCLUDED
#include "pvmf_cpmplugin_access_interface_factory.h"
#endif
#ifndef PVMF_CPMPLUGIN_ACCESS_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_access_interface.h"
#endif
#ifndef PVMF_CPMPLUGIN_LICENSE_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_license_interface.h"
#endif
#ifndef PVMF_META_DATA_EXTENSION_H_INCLUDED
#include "pvmf_meta_data_extension.h"
#endif

/**
 * Macros for calling PVLogger
 */
#define PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGWARNING(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_WARNING,m);
#define PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFO(m) PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFOMED(m)
#define PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_DEBUG,m);

#define PVMF_CPM_PASSTHRU_PLUGIN_OMA1_INTERNAL_CMDQ_SIZE 10
//use a #define instead of a typedef here to avoid warnings on some compilers.
#define PVMFCPMPassThruPlugInOMA1CommandBase PVMFGenericNodeCommand<OsclMemAllocator>

class PVMFCPMPassThruPlugInOMA1Command : public PVMFCPMPassThruPlugInOMA1CommandBase
{
    public:
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       OsclAny* aArg1,
                       OsclAny* aArg2,
                       const OsclAny* aContext)
        {
            PVMFCPMPassThruPlugInOMA1CommandBase::Construct(s, cmd, aContext);
            iParam1 = aArg1;
            iParam2 = aArg2;
        };

        void Parse(OsclAny*&arg1, OsclAny*&arg2)
        {
            arg1 = iParam1;
            arg2 = iParam2;
        };

        void Construct(PVMFSessionId s,
                       int32 cmd,
                       OsclAny* aArg1,
                       OsclAny* aArg2,
                       OsclAny* aArg3,
                       const OsclAny* aContext)
        {
            PVMFCPMPassThruPlugInOMA1CommandBase::Construct(s, cmd, aContext);
            iParam1 = aArg1;
            iParam2 = aArg2;
            iParam3 = aArg3;
        };

        void Parse(OsclAny*&arg1, OsclAny*&arg2, OsclAny*&arg3)
        {
            arg1 = iParam1;
            arg2 = iParam2;
            arg3 = iParam3;
        };

        void Construct(PVMFSessionId s,
                       int32 cmd,
                       OsclAny* aArg1,
                       OsclAny* aArg2,
                       OsclAny* aArg3,
                       OsclAny* aArg4,
                       const OsclAny* aContext)
        {
            PVMFCPMPassThruPlugInOMA1CommandBase::Construct(s, cmd, aContext);
            iParam1 = aArg1;
            iParam2 = aArg2;
            iParam3 = aArg3;
            iParam4 = aArg4;
        };

        void Parse(OsclAny*&arg1, OsclAny*&arg2, OsclAny*&arg3, OsclAny*&arg4)
        {
            arg1 = iParam1;
            arg2 = iParam2;
            arg3 = iParam3;
            arg4 = iParam4;
        };
};

typedef PVMFNodeCommandQueue<PVMFCPMPassThruPlugInOMA1Command, OsclMemAllocator> PVMFCPMPassThruPlugInOMA1CmdQ;

/* Start range for command IDs */
#define PVMF_CPM_PASSTHRU_PLUGIN_OMA1_COMMAND_ID_START 50000

/*
 * IDs for all of the asynchronous plugin commands.
 */
enum TPVMFCPMPassThruPlugInOMA1Command
{
    PVMF_CPM_PASSTHRU_PLUGIN_OMA1_QUERYUUID = PVMF_GENERIC_NODE_QUERYUUID
    , PVMF_CPM_PASSTHRU_PLUGIN_OMA1_QUERYINTERFACE = PVMF_GENERIC_NODE_QUERYINTERFACE
    , PVMF_CPM_PASSTHRU_PLUGIN_OMA1_INIT = PVMF_GENERIC_NODE_COMMAND_LAST
    , PVMF_CPM_PASSTHRU_PLUGIN_OMA1_RESET
    , PVMF_CPM_PASSTHRU_PLUGIN_OMA1_AUTHENTICATE
    , PVMF_CPM_PASSTHRU_PLUGIN_OMA1_AUTHORIZE_USAGE
    , PVMF_CPM_PASSTHRU_PLUGIN_OMA1_USAGE_COMPLETE
    , PVMF_CPM_PASSTHRU_PLUGIN_OMA1_GET_LICENSE
    , PVMF_CPM_PASSTHRU_PLUGIN_OMA1_CANCEL_GET_LICENSE
    , PVMF_CPM_PASSTHRU_PLUGIN_OMA1_METADATA_KEYS
    , PVMF_CPM_PASSTHRU_PLUGIN_OMA1_METADATA_VALUES
};

class OsclFileHandle;

/*
 * CPM OMA1.0 Plugin
 */

class PVMFCPMPassThruPlugInOMA1 : public OsclActiveObject,
            public PVMFCPMPluginInterface,
            public PVMFCPMPluginAuthenticationInterface,
            public PVMFCPMPluginAuthorizationInterface,
            public PVMFCPMPluginAccessInterfaceFactory,
            public PVMFCPMPluginLicenseInterface,
            public PVMFMetadataExtensionInterface
{
    public:
        static PVMFCPMPluginInterface* CreatePlugIn(bool aFailAuthorizeUsage = false, bool aCancelAcquireLicense = false,
                bool aSourceInitDataNotSupported = false,
                PVMFCPMContentType aCPMContentType = PVMF_CPM_FORMAT_AUTHORIZE_BEFORE_ACCESS);
        static void DestroyPlugIn(PVMFCPMPluginInterface*);

    public:
        /* From PVInterface */
        void addRef()
        {
            ++iExtensionRefCount;
        }
        void removeRef()
        {
            --iExtensionRefCount;
        }
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        /* From PVMFCPMPluginInterface */
        OSCL_IMPORT_REF PVMFCommandId QueryUUID(PVMFSessionId aSession,
                                                const PvmfMimeString& aMimeType,
                                                Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                                bool aExactUuidsOnly = false,
                                                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryInterface(PVMFSessionId aSession,
                const PVUuid& aUuid,
                PVInterface*& aInterfacePtr,
                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Init(PVMFSessionId aSession,
                                           const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Reset(PVMFSessionId aSession,
                                            const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFStatus SetSourceInitializationData(OSCL_wString& aSourceURL,
                PVMFFormatType& aSourceFormat,
                OsclAny* aSourceData);

        OSCL_IMPORT_REF PVMFCPMContentType GetCPMContentType();


        /* From PVMFCPMPluginAuthenticationInterface */
        OSCL_IMPORT_REF PVMFCommandId AuthenticateUser(PVMFSessionId aSessionId,
                OsclAny* aAuthenticationData,
                OsclAny* aContext = NULL);

        /* From PVMFCPMPluginAuthorizationInterface */
        OSCL_IMPORT_REF PVMFCommandId AuthorizeUsage(PVMFSessionId aSessionId,
                PvmiKvp& aRequestedUsage,
                PvmiKvp& aApprovedUsage,
                PvmiKvp& aAuthorizationData,
                uint32&  aRequestTimeOutInMS,
                OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFCommandId UsageComplete(PVMFSessionId aSessionId,
                const OsclAny* aContext = NULL);

        /* From PVMFCPMPluginAccessInterfaceFactory */
        OSCL_IMPORT_REF PVMFStatus QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids);
        OSCL_IMPORT_REF PVInterface* CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid);
        OSCL_IMPORT_REF void DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid,
                PVInterface* aPtr);
        OSCL_IMPORT_REF void SetStreamReadCapacityObserver(PVMFDataStreamReadCapacityObserver* aObs);


        /* From PVMFCPMPluginAccessInterfaceFactory */
        OSCL_IMPORT_REF PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                OSCL_wString& aContentName,
                OSCL_wString& aLicenseURL);
        OSCL_IMPORT_REF PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                OSCL_String&  aContentName,
                OSCL_String&  aLicenseURL);
        OSCL_IMPORT_REF PVMFCommandId GetLicense(PVMFSessionId aSessionId
                , OSCL_wString& aContentName
                , OsclAny* aLicenseData = NULL
                                          , uint32 aDataSize = 0
                                                               , int32 aTimeoutMsec = (-1)
                                                                                      , OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId GetLicense(PVMFSessionId aSessionId
                , OSCL_String&  aContentName
                , OsclAny* aLicenseData = NULL
                                          , uint32 aDataSize = 0
                                                               , int32 aTimeoutMsec = (-1)
                                                                                      , OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelGetLicense(PVMFSessionId aSessionId
                , PVMFCommandId aCmdId
                , OsclAny* aContext = NULL);

        //From PVMFMetadataExtensionInterface
        OSCL_IMPORT_REF uint32 GetNumMetadataKeys(char* aQueryKeyString = NULL) ;
        OSCL_IMPORT_REF uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList) ;
        OSCL_IMPORT_REF PVMFCommandId GetNodeMetadataKeys(PVMFSessionId aSessionId,
                PVMFMetadataList& aKeyList,
                uint32 aStartingKeyIndex,
                int32 aMaxKeyEntries = -1,
                char* aQueryKeyString = NULL,
                const OsclAny* aContextData = NULL) ;
        OSCL_IMPORT_REF PVMFCommandId GetNodeMetadataValues(PVMFSessionId aSessionId,
                PVMFMetadataList& aKeyList,
                Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                uint32 aStartingValueIndex,
                int32 aMaxValueEntries = -1,
                const OsclAny* aContextData = NULL) ;
        OSCL_IMPORT_REF PVMFStatus ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList,
                uint32 aStartingKeyIndex,
                uint32 aEndKeyIndex) ;
        OSCL_IMPORT_REF PVMFStatus ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                uint32 aStartingValueIndex,
                uint32 aEndValueIndex) ;

    private:
        /* From PVMFCPMPluginInterface */
        void ThreadLogon();
        void ThreadLogoff();

        OSCL_IMPORT_REF PVMFCPMPassThruPlugInOMA1(bool aFailAuthorizeUsage = false,
                bool aCancelAcquireLicense = false,
                bool aSourceInitDataNotSupported = false,
                PVMFCPMContentType aCPMContentType = PVMF_CPM_FORMAT_AUTHORIZE_BEFORE_ACCESS,
                int32 aPriority = OsclActiveObject::EPriorityNominal);

        virtual ~PVMFCPMPassThruPlugInOMA1();

        void Run();

        PVMFCPMPassThruPlugInOMA1CmdQ iInputCommands;
        PVMFCPMPassThruPlugInOMA1CmdQ iCurrentCommand;
        PVMFCommandId QueueCommandL(PVMFCPMPassThruPlugInOMA1Command&);
        void MoveCmdToCurrentQueue(PVMFCPMPassThruPlugInOMA1Command& aCmd);
        bool ProcessCommand(PVMFCPMPassThruPlugInOMA1Command&);
        void CommandComplete(PVMFCPMPassThruPlugInOMA1CmdQ&,
                             PVMFCPMPassThruPlugInOMA1Command&,
                             PVMFStatus,
                             OsclAny* aData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);

        void DoQueryUuid(PVMFCPMPassThruPlugInOMA1Command&);
        void DoQueryInterface(PVMFCPMPassThruPlugInOMA1Command&);
        void DoInit(PVMFCPMPassThruPlugInOMA1Command&);
        void DoReset(PVMFCPMPassThruPlugInOMA1Command&);
        void DoAuthenticate(PVMFCPMPassThruPlugInOMA1Command&);
        void DoAuthorizeUsage(PVMFCPMPassThruPlugInOMA1Command&);
        void DoUsageComplete(PVMFCPMPassThruPlugInOMA1Command& aCmd);
        PVMFStatus DoGetLicense(PVMFCPMPassThruPlugInOMA1Command& aCmd);
        void DoCancelGetLicense(PVMFCPMPassThruPlugInOMA1Command& aCmd);
        void DoMetadataKeys(PVMFCPMPassThruPlugInOMA1Command& aCmd);
        void DoMetadataValues(PVMFCPMPassThruPlugInOMA1Command& aCmd);

        PVLogger* iLogger;
        uint32 iExtensionRefCount;

        OSCL_wHeapString<OsclMemAllocator> iFilename;
        PVMFFormatType iSourceFormatType;
        bool oSourceSet;
        OsclFileHandle* iFileHandle;
        friend class PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl;
        bool iFailAuthorizeUsage;
        bool iCancelAcquireLicense;
        bool iSourceInitDataNotSupported;
        PVMFCPMContentType iCPMContentType;
        PVMFDataStreamReadCapacityObserver* iDataStreamReadCapacityObserver;

        //metadata related
        enum TOMA1Metadata
        {
            EIsProtectedOMA1
            , ELicAvailableOMA1
            , ELicTypeOMA1
            , EForwardLockOMA1
            , ELicLastOMA1 //placeholder.
        };
        class TOMA1MetadataEntry
        {
            public:
                TOMA1MetadataEntry(): iValid(false) {}
                bool iValid;
                OSCL_HeapString<OsclMemAllocator> iQuery;//query key string.
                OSCL_HeapString<OsclMemAllocator> iValue;//value string.
                PvmiKvp iKvp;//value kvp.
        };
        //metadata array.
        TOMA1MetadataEntry iMetaData[ELicLastOMA1];
        OSCL_HeapString<OsclMemAllocator> iLicType;
        void MakeMetadata();
};


/*!
** PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl
*/
class PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl : public PVMIDataStreamSyncInterface
{
    public:
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
        PvmiDataStreamStatus Seek(PvmiDataStreamSession aSessionID,
                                  int32 offset,
                                  PvmiDataStreamSeekType origin);

        OSCL_IMPORT_REF
        uint32 GetCurrentPointerPosition(PvmiDataStreamSession aSessionID) ;

        OSCL_IMPORT_REF
        PvmiDataStreamStatus Flush(PvmiDataStreamSession aSessionID);

    private:
        friend class PVMFCPMPassThruPlugInOMA1;
        PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl(PVMFCPMPassThruPlugInOMA1&);
        ~PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl();

        Oscl_File *iFileObject;
        int32 iFileNumBytes;
        Oscl_FileServer iFs;
        PVMFCPMPassThruPlugInOMA1& iContainer;
        PVLogger* iLogger;
        PVMFDataStreamReadCapacityObserver* iDataStreamReadCapacityObserver;
};
#endif //PVMF_CPMPLUGIN_OMA1_H_INCLUDED


