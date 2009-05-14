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
#ifndef CPM_INTERNAL_H_INCLUDED
#define CPM_INTERNAL_H_INCLUDED

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
#ifndef PVMF_DATA_SOURCE_INIT_EXTENSION_H_INCLUDED
#include "pvmf_data_source_init_extension.h"
#endif
#ifndef PVMF_CPMPLUGIN_LICENSE_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_license_interface.h"
#endif
#ifndef PVMF_META_DATA_EXTENSION_H_INCLUDED
#include "pvmf_meta_data_extension.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
#endif
#ifndef CPM_H_INCLUDED
#include "cpm.h"
#endif
#ifndef CPM_PLUGIN_REGISTRY_H
#include "cpm_plugin_registry.h"
#endif
#ifndef CPM_TYPES_H
#include "cpm_types.h"
#endif
#ifndef PVMI_KVP_INCLUDED
#include "pvmi_kvp.h"
#endif
#ifndef PVMI_KVP_UTIL_H_INCLUDED
#include "pvmi_kvp_util.h"
#endif

/**
 * Macros for calling PVLogger
 */
#define PVMF_CPM_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define PVMF_CPM_LOGWARNING(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_WARNING,m);
#define PVMF_CPM_LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_CPM_LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_CPM_LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_CPM_LOGINFO(m) PVMF_CPM_LOGINFOMED(m)

/*
 * IDs for all of the asynchronous plugin commands.
 */
enum TPVMFCPMCommand
{
    PVMF_CPM_QUERY_INTERFACE = PVMF_GENERIC_NODE_QUERYINTERFACE,
    /* Starting value for command IDs */
    PVMF_CPM_COMMAND_ID_START = 40000,
    PVMF_CPM_INIT,
    PVMF_CPM_OPEN_SESSION,
    PVMF_CPM_REGISTER_CONTENT,
    PVMF_CPM_APPROVE_USAGE,
    PVMF_CPM_USAGE_COMPLETE,
    PVMF_CPM_CLOSE_SESSION,
    PVMF_CPM_RESET,
    PVMF_CPM_GET_METADATA_KEYS,
    PVMF_CPM_GET_METADATA_VALUES,
    PVMF_CPM_GET_LICENSE_W,
    PVMF_CPM_GET_LICENSE,
    PVMF_CPM_CANCEL_GET_LICENSE,
    PVMF_CPM_COMMAND_LAST
};

#define PVMF_CPM_INTERNAL_QUERY_UUID_CMD                         1000
#define PVMF_CPM_INTERNAL_INIT_CMD                               1001
#define PVMF_CPM_INTERNAL_QUERY_AUTHENTICATION_INTERFACE_CMD     1002
#define PVMF_CPM_INTERNAL_QUERY_AUTHORIZATION_INTERFACE_CMD      1003
#define PVMF_CPM_INTERNAL_QUERY_ACCESS_INTERFACE_FACTORY_CMD	 1004
#define PVMF_CPM_INTERNAL_QUERY_METADATA_EXTENSION_INTERFACE_CMD 1005
#define PVMF_CPM_INTERNAL_AUTHENTICATE_CMD                       1006
#define PVMF_CPM_INTERNAL_AUTHORIZE_CMD                          1007
#define PVMF_CPM_INTERNAL_USAGE_COMPLETE_CMD                     1008
#define PVMF_CPM_INTERNAL_RESET_CMD                              1009
#define PVMF_CPM_INTERNAL_GET_PLUGIN_META_DATA_KEYS_CMD          1010
#define PVMF_CPM_INTERNAL_GET_PLUGIN_META_DATA_VALUES_CMD        1011
#define PVMF_CPM_INTERNAL_QUERY_LICENSE_INTERFACE_CMD            1012
#define PVMF_CPM_INTERNAL_GET_LICENSE_CMD                        1013
#define PVMF_CPM_INTERNAL_QUERY_CAP_CONFIG_INTERFACE_CMD         1014
#define PVMF_CPM_INTERNAL_CANCEL_GET_LICENSE					 1015

#define PVMF_CPM_INTERNAL_CMDQ_SIZE 10

#define PVMFCPMCommandBase PVMFGenericNodeCommand<OsclMemAllocator>

class PVMFCPMCommand : public PVMFCPMCommandBase
{
    public:
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       OsclAny* aArg1,
                       OsclAny* aArg2,
                       OsclAny* aArg3,
                       OsclAny* aArg4,
                       const OsclAny* aContext)
        {
            PVMFCPMCommandBase::Construct(s, cmd, aContext);
            iParam1 = aArg1;
            iParam2 = aArg2;
            iParam3 = aArg3;
            iParam4 = aArg4;
        };

        void Parse(OsclAny*& aArg1,
                   OsclAny*& aArg2,
                   OsclAny*& aArg3,
                   OsclAny*& aArg4)
        {
            aArg1 = iParam1;
            aArg2 = iParam2;
            aArg3 = iParam3;
            aArg4 = iParam4;
        };

        void Construct(PVMFSessionId s,
                       int32 cmd,
                       OsclAny* aArg1,
                       OsclAny* aArg2,
                       OsclAny* aArg3,
                       OsclAny* aArg4,
                       OsclAny* aArg5,
                       const OsclAny* aContext)
        {
            PVMFCPMCommandBase::Construct(s, cmd, aContext);
            iParam1 = aArg1;
            iParam2 = aArg2;
            iParam3 = aArg3;
            iParam4 = aArg4;
            iParam5 = aArg5;
        };

        void Parse(OsclAny*& aArg1,
                   OsclAny*& aArg2,
                   OsclAny*& aArg3,
                   OsclAny*& aArg4,
                   OsclAny*& aArg5)
        {
            aArg1 = iParam1;
            aArg2 = iParam2;
            aArg3 = iParam3;
            aArg4 = iParam4;
            aArg5 = iParam5;
        };


        /* For Init and Reset */
        void Construct(int32 aCmd, const OsclAny* aContext)
        {
            iSession = 0;
            iCmd = aCmd;
            iContext = aContext;
            iParam1 = NULL;
            iParam2 = NULL;
            iParam3 = NULL;
            iParam4 = NULL;
            iParam5 = NULL;
        };

        /* For open session */
        void Construct(int32 aCmd,
                       OsclAny* aArg1,
                       const OsclAny* aContext)
        {
            iSession = 0;
            iCmd = aCmd;
            iContext = aContext;
            iParam1 = aArg1;
            iParam2 = NULL;
            iParam3 = NULL;
            iParam4 = NULL;
            iParam5 = NULL;
        };

        void Parse(OsclAny*& aArg1)
        {
            aArg1 = iParam1;
        };

        /* Constructor and parser for GetNodeMetadataKeys */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       PVMFMetadataList& aKeyList,
                       int32 aStartingIndex,
                       int32 aMaxEntries,
                       char* aQueryKey,
                       const OsclAny* aContext)
        {
            PVMFCPMCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aKeyList;
            iParam2 = (OsclAny*)aStartingIndex;
            iParam3 = (OsclAny*)aMaxEntries;
            if (aQueryKey)
            {
                /* allocate a copy of the query key string. */
                Oscl_TAlloc<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> str;
                iParam4 = str.alloc_and_construct(aQueryKey);
            }
        }
        void Parse(PVMFMetadataList*& MetaDataListPtr,
                   int32 &aStartingIndex,
                   int32 &aMaxEntries,
                   char*& aQueryKey)
        {
            MetaDataListPtr = (PVMFMetadataList*)iParam1;
            aStartingIndex = (int32)iParam2;
            aMaxEntries = (int32)iParam3;
            aQueryKey = NULL;
            if (iParam4)
            {
                OSCL_HeapString<OsclMemAllocator>* keystring =
                    (OSCL_HeapString<OsclMemAllocator>*)iParam4;
                aQueryKey = keystring->get_str();
            }
        }

        /* Constructor and parser for GetNodeMetadataValue */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       PVMFMetadataList& aKeyList,
                       Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                       int32 aStartIndex,
                       int32 aMaxEntries,
                       const OsclAny* aContext)
        {
            PVMFCPMCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aKeyList;
            iParam2 = (OsclAny*) & aValueList;
            iParam3 = (OsclAny*)aStartIndex;
            iParam4 = (OsclAny*)aMaxEntries;

        }
        void Parse(PVMFMetadataList* &aKeyList,
                   Oscl_Vector<PvmiKvp, OsclMemAllocator>* &aValueList,
                   int32 &aStartingIndex,
                   int32 &aMaxEntries)
        {
            aKeyList = (PVMFMetadataList*)iParam1;
            aValueList = (Oscl_Vector<PvmiKvp, OsclMemAllocator>*)iParam2;
            aStartingIndex = (int32)iParam3;
            aMaxEntries = (int32)iParam4;
        }

        /* Constructor and parser for GetLicenseW */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       OSCL_wString& aContentName,
                       OsclAny* aLicenseData,
                       uint32 aDataSize,
                       int32 aTimeoutMsec,
                       OsclAny* aContext)
        {
            PVMFCPMCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aContentName;
            iParam2 = (OsclAny*)aLicenseData;
            iParam3 = (OsclAny*)aDataSize;
            iParam4 = (OsclAny*)aTimeoutMsec;
            iParam5 = NULL;
        }
        void Parse(OSCL_wString*& aContentName,
                   OsclAny*& aLicenseData,
                   uint32& aDataSize,
                   int32& aTimeoutMsec)
        {
            aContentName = (OSCL_wString*)iParam1;
            aLicenseData = (PVMFTimestamp*)iParam2;
            aDataSize = (uint32)iParam3;
            aTimeoutMsec = (int32)iParam4;
        }

        /* Constructor and parser for GetLicense */
        void Construct(PVMFSessionId s,
                       int32 cmd,
                       OSCL_String& aContentName,
                       OsclAny* aLicenseData,
                       uint32 aDataSize,
                       int32 aTimeoutMsec,
                       OsclAny* aContext)
        {
            PVMFCPMCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*) & aContentName;
            iParam2 = (OsclAny*)aLicenseData;
            iParam3 = (OsclAny*)aDataSize;
            iParam4 = (OsclAny*)aTimeoutMsec;
            iParam5 = NULL;
        }
        void Parse(OSCL_String*& aContentName,
                   OsclAny*& aLicenseData,
                   uint32& aDataSize,
                   int32& aTimeoutMsec)
        {
            aContentName = (OSCL_String*)iParam1;
            aLicenseData = (PVMFTimestamp*)iParam2;
            aDataSize = (uint32)iParam3;
            aTimeoutMsec = (int32)iParam4;
        }

};

typedef PVMFNodeCommandQueue<PVMFCPMCommand, OsclMemAllocator> PVMFCPMCommandCmdQ;

class CPMSessionInfo
{
    public:
        CPMSessionInfo()
        {
            iSessionId = 0;
            iSourceFormatType = 0;
            iSourceData = NULL;
            iNumPlugInAunthenticateRequestsPending = 0;
            iNumPlugInAunthenticateRequestsComplete = 0;
            iNumPlugInAuthorizeInterfaceQueryRequestsPending = 0;
            iNumPlugInAuthorizeInterfaceQueryRequestsComplete = 0;
            iNumPlugInAccessInterfaceFactoryQueryRequestsPending = 0;
            iNumPlugInAccessInterfaceFactoryQueryRequestsComplete = 0;
            iNumPlugInLicenseAcquisitionInterfaceRequestsPending = 0;
            iNumPlugInLicenseAcquisitionInterfaceRequestsComplete = 0;
            iAccessPlugInID = 0xFFFFFFFF;
        };

        PVMFSessionId iSessionId;
        OSCL_wHeapString<OsclMemAllocator> iSourceURL;
        PVMFFormatType iSourceFormatType;
        OsclAny*       iSourceData;
        uint32 iNumPlugInAunthenticateRequestsPending;
        uint32 iNumPlugInAunthenticateRequestsComplete;
        uint32 iNumPlugInAuthorizeInterfaceQueryRequestsPending;
        uint32 iNumPlugInAuthorizeInterfaceQueryRequestsComplete;
        uint32 iNumPlugInAccessInterfaceFactoryQueryRequestsPending;
        uint32 iNumPlugInAccessInterfaceFactoryQueryRequestsComplete;
        uint32 iNumPlugInLicenseAcquisitionInterfaceRequestsPending;
        uint32 iNumPlugInLicenseAcquisitionInterfaceRequestsComplete;
        uint32 iAccessPlugInID;
};

class CPMPlugInParams
{
    public:
        CPMPlugInParams()
        {
            iPlugInSessionID = 0;
            iPlugInInterface = NULL;
            iPlugInData = NULL;
            iPlugInAuthenticationInterface = NULL;
            iPlugInAuthorizationInterface = NULL;
            iPlugInAccessInterfaceFactory = NULL;
            iPlugInMetaDataExtensionInterface = NULL;
            iPlugInLicenseInterface = NULL;
            iPlugInCapConfigExtensionInterface = NULL;
            iPlugInAuthenticationInterfacePVI = NULL;
            iPlugInAuthorizationInterfacePVI = NULL;
            iPlugInAccessInterfaceFactoryPVI = NULL;
            iPlugInMetaDataExtensionInterfacePVI = NULL;
            iPlugInLicenseInterfacePVI = NULL;
            iPlugInCapConfigExtensionInterfacePVI = NULL;

            iAuthorizationRequestTimeOut =
                PVMF_CPM_DEFAULT_PLUGIN_AUTHORIZATION_TIMEOUT_IN_MS;
            iConnected = false;
            iAuthorized = false;
            iNumMetaDataKeysAvailable = 0;
            iGetMetaDataValuesComplete = false;
            iNumMetaDataValuesAvailable = 0;
            iMetaDataKeyStartIndex = 0;
            iMetaDataKeyEndIndex = 0;
            iMetaDataValueStartIndex = 0;
            iMetaDataValueEndIndex = 0;
            iGetMetaDataKeysComplete = false;
        };

        PVMFSessionId iPlugInSessionID;
        uint32        iPlugInID;
        OSCL_HeapString<OsclMemAllocator> iPlugInMimeType;
        PVMFCPMPluginInterface* iPlugInInterface;
        OsclAny* iPlugInData;
        Oscl_Vector<PVUuid, OsclMemAllocator> iExtensionUuids;
        PVMFCPMPluginAuthenticationInterface* iPlugInAuthenticationInterface;
        PVMFCPMPluginAuthorizationInterface* iPlugInAuthorizationInterface;
        PVMFCPMPluginAccessInterfaceFactory* iPlugInAccessInterfaceFactory;
        PVMFMetadataExtensionInterface* iPlugInMetaDataExtensionInterface;
        PVMFCPMPluginLicenseInterface* iPlugInLicenseInterface;
        PvmiCapabilityAndConfig* iPlugInCapConfigExtensionInterface;
        PVInterface* iPlugInAuthenticationInterfacePVI;
        PVInterface* iPlugInAuthorizationInterfacePVI;
        PVInterface* iPlugInAccessInterfaceFactoryPVI;
        PVInterface* iPlugInMetaDataExtensionInterfacePVI;
        PVInterface* iPlugInLicenseInterfacePVI;
        PVInterface* iPlugInCapConfigExtensionInterfacePVI;
        uint32 iAuthorizationRequestTimeOut;
        bool iConnected;
        bool iAuthorized;
        uint32 iNumMetaDataKeysAvailable;
        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iAvailableMetadataKeys;
        bool iGetMetaDataValuesComplete;
        uint32 iNumMetaDataValuesAvailable;
        uint32 iMetaDataKeyStartIndex;
        uint32 iMetaDataKeyEndIndex;
        uint32 iMetaDataValueStartIndex;
        uint32 iMetaDataValueEndIndex;
        bool iGetMetaDataKeysComplete;
};

class CPMContentUsageContext
{
    public:
        CPMContentUsageContext()
        {
            iUsageID = 0;
            iNumAuthorizeRequestsPending = 0;
            iNumAuthorizeRequestsComplete = 0;
            iNumUsageCompleteRequestsPending = 0;
            iNumUsageCompleteRequestsComplete = 0;
        };

        PVMFCPMUsageID iUsageID;
        uint32 iNumAuthorizeRequestsPending;
        uint32 iNumAuthorizeRequestsComplete;
        uint32 iNumUsageCompleteRequestsPending;
        uint32 iNumUsageCompleteRequestsComplete;
};

typedef struct tagPVMFCPMCommandContext
{
    tagPVMFCPMCommandContext()
    {
        parentCmd = 0;
        cmd = 0;
        oFree = true;
        plugInID = 0;
        sessionid = 0;
        usageid = 0;
    };

    int32 parentCmd;
    int32 cmd;
    bool  oFree;
    uint32 plugInID;
    PVMFSessionId sessionid;
    PVMFCPMUsageID usageid;
} PVMFCPMCommandContext;

class PVMFCPMImpl : public OsclActiveObject,
            public PVMFCPM,
            public PVMFMetadataExtensionInterface,
            public PvmiCapabilityAndConfig,
            public PVMFCPMPluginCmdStatusObserver,
            public PVMFCPMPluginLicenseInterface
{
    public:
        OSCL_IMPORT_REF PVMFCPMImpl(PVMFCPMStatusObserver& aObserver,
                                    int32 aPriority = OsclActiveObject::EPriorityNominal);

        ~PVMFCPMImpl();

        OSCL_IMPORT_REF void ThreadLogon();
        OSCL_IMPORT_REF void ThreadLogoff();

        OSCL_IMPORT_REF PVMFCommandId Init(const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFCommandId OpenSession(PVMFSessionId& aSessionId,
                const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFCommandId RegisterContent(PVMFSessionId aSessionId,
                OSCL_wString& aSourceURL,
                PVMFFormatType& aSourceFormat,
                OsclAny* aSourceData,
                const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFCPMContentType GetCPMContentType(PVMFSessionId aSessionId);
        OSCL_IMPORT_REF PVMFStatus GetCPMContentFilename(PVMFSessionId aSessionId, OSCL_wString& aFileName) ;

        OSCL_IMPORT_REF PVMFStatus GetContentAccessFactory(PVMFSessionId aSessionId,
                PVMFCPMPluginAccessInterfaceFactory*& aContentAccessFactory);

        OSCL_IMPORT_REF PVMFCommandId ApproveUsage(PVMFSessionId aSessionId,
                PvmiKvp& aRequestedUsage,
                PvmiKvp& aApprovedUsage,
                PvmiKvp& aAuthorizationData,
                PVMFCPMUsageID& aUsageID,
                const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFCommandId UsageComplete(PVMFSessionId aSessionId,
                PVMFCPMUsageID& aUsageID,
                OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFCommandId CloseSession(PVMFSessionId& aSessionId,
                const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF PVMFCommandId Reset(const OsclAny* aContext = NULL);

        CPMPluginRegistry* GetPluginRegistry()
        {
            return iPluginRegistry;
        }

        OSCL_IMPORT_REF PVMFCommandId QueryInterface(PVMFSessionId aSessionId,
                const PVUuid& aUuid,
                PVInterface*& aInterfacePtr,
                const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF void CPMPluginCommandCompleted(const PVMFCmdResp& aResponse);

        /* From PVInterface */
        OSCL_IMPORT_REF void addRef();
        OSCL_IMPORT_REF void removeRef();
        OSCL_IMPORT_REF bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        /* From PVMFMetadataExtensionInterface */
        OSCL_IMPORT_REF uint32 GetNumMetadataKeys(char* aQueryKeyString = NULL);
        OSCL_IMPORT_REF uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList);
        OSCL_IMPORT_REF PVMFCommandId GetNodeMetadataKeys(PVMFSessionId aSessionId,
                PVMFMetadataList& aKeyList,
                uint32 aStartingKeyIndex,
                int32 aMaxKeyEntries,
                char* aQueryKeyString = NULL,
                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVMFCommandId GetNodeMetadataValues(PVMFSessionId aSessionId,
                PVMFMetadataList& aKeyList,
                Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                uint32 aStartingValueIndex,
                int32 aMaxValueEntries = -1,
                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVMFStatus ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList,
                uint32 aStartingKeyIndex,
                uint32 aEndKeyIndex);
        OSCL_IMPORT_REF PVMFStatus ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                uint32 aStartingValueIndex,
                uint32 aEndValueIndex);

        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iAvailableMetadataKeys;

        /* From PVMFCPMPluginLicenseInterface */
        PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                 OSCL_wString& aContentName,
                                 OSCL_wString& aLicenseURL)
        {
            OSCL_UNUSED_ARG(aSessionId);
            OSCL_UNUSED_ARG(aContentName);
            OSCL_UNUSED_ARG(aLicenseURL);
            //must use Async method.
            return PVMFErrNotSupported;
        }
        PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                 OSCL_String&  aContentName,
                                 OSCL_String&  aLicenseURL)
        {
            OSCL_UNUSED_ARG(aSessionId);
            OSCL_UNUSED_ARG(aContentName);
            OSCL_UNUSED_ARG(aLicenseURL);
            //must use Async method.
            return PVMFErrNotSupported;
        }

        PVMFCommandId GetLicense(PVMFSessionId aSessionId,
                                 OSCL_wString& aContentName,
                                 OsclAny* aData,
                                 uint32 aDataSize,
                                 int32 aTimeoutMsec,
                                 OsclAny* aContextData) ;

        PVMFCommandId GetLicense(PVMFSessionId aSessionId,
                                 OSCL_String&  aContentName,
                                 OsclAny* aData,
                                 uint32 aDataSize,
                                 int32 aTimeoutMsec,
                                 OsclAny* aContextData);

        PVMFCommandId CancelGetLicense(PVMFSessionId aSessionId
                                       , PVMFCommandId aCmdId
                                       , OsclAny* aContextData);

        PVMFStatus GetLicenseStatus(
            PVMFCPMLicenseStatus& aStatus) ;

        /* Implement pure virtuals from PvmiCapabilityAndConfig interface */
        PVMFStatus getParametersSync(PvmiMIOSession aSession,
                                     PvmiKeyType aIdentifier,
                                     PvmiKvp*& aParameters,
                                     int& num_parameter_elements,
                                     PvmiCapabilityContext aContext);
        PVMFStatus releaseParameters(PvmiMIOSession aSession,
                                     PvmiKvp* aParameters,
                                     int num_elements);
        void setParametersSync(PvmiMIOSession aSession,
                               PvmiKvp* aParameters,
                               int num_elements,
                               PvmiKvp*& aRet_kvp);
        PVMFStatus verifyParametersSync(PvmiMIOSession aSession,
                                        PvmiKvp* aParameters,
                                        int num_elements);
        /* Unsupported PvmiCapabilityAndConfig methods */
        void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
        {
            OSCL_UNUSED_ARG(aObserver);
        };
        void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
        };
        void setContextParameters(PvmiMIOSession aSession,
                                  PvmiCapabilityContext& aContext,
                                  PvmiKvp* aParameters,
                                  int num_parameter_elements)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(num_parameter_elements);
        };
        void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
        };
        PVMFCommandId setParametersAsync(PvmiMIOSession aSession,
                                         PvmiKvp* aParameters,
                                         int num_elements,
                                         PvmiKvp*& aRet_kvp,
                                         OsclAny* context = NULL)
        {
            OSCL_UNUSED_ARG(aRet_kvp);
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(num_elements);
            OSCL_UNUSED_ARG(context);
            return -1;
        }
        uint32 getCapabilityMetric(PvmiMIOSession aSession)
        {
            OSCL_UNUSED_ARG(aSession);
            return 0;
        }

    private:
        void Run();

        PVMFCPMStatusObserver& iObserver;

        /* Command processing */
        PVMFCPMCommandContext iInternalCmdPool[PVMF_CPM_INTERNAL_CMDQ_SIZE];
        PVMFCPMCommandCmdQ iInputCommands;
        PVMFCPMCommandCmdQ iCurrentCommand;
        PVMFCPMCommandCmdQ iCancelCommand;
        PVMFCommandId QueueCommandL(PVMFCPMCommand&);
        PVMFCPMCommandContext* RequestNewInternalCmd();
        void MoveCmdToCurrentQueue(PVMFCPMCommand& aCmd);
        void MoveCmdToCancelQueue(PVMFCPMCommand& aCmd);
        bool ProcessCommand(PVMFCPMCommand&);
        void CommandComplete(PVMFCPMCommandCmdQ&,
                             PVMFCPMCommand&,
                             PVMFStatus,
                             OsclAny* aData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);
        void CommandComplete(PVMFCPMCommandCmdQ& aCmdQ,
                             PVMFCPMCommand& aCmd,
                             PVMFStatus aStatus,
                             PVInterface* aErrorExtIntf);

        void DoInit(PVMFCPMCommand&);
        PVMFStatus InitRegisteredPlugIns();
        PVMFStatus QueryForPlugInMetaDataExtensionInterface();
        PVMFStatus QueryForPlugInAuthenticationInterface();
        PVMFStatus QueryForPlugInCapConfigInterface();
        void CompleteInitPlugIns();
        void CompleteMetaDataExtInterfaceQueryFromPlugIns();
        void CompleteCapConfigExtInterfaceQueryFromPlugIns();
        void CompleteCPMInit();

        void DoOpenSession(PVMFCPMCommand&);
        PVMFStatus AuthenticateWithAllRegisteredPlugIns(PVMFSessionId);
        void CompleteOpenSession(CPMSessionInfo*);

        void DoRegisterContent(PVMFCPMCommand&);
        PVMFStatus PopulateListOfActivePlugIns(CPMSessionInfo*);
        PVMFStatus QueryForAuthorizationInterface(CPMSessionInfo*);
        PVMFStatus QueryForAccessInterfaceFactory(CPMSessionInfo*);
        PVMFStatus DetermineAccessPlugIn(CPMSessionInfo*);
        PVMFStatus QueryForLicenseInterface(CPMSessionInfo*);
        void CompleteRegisterContentPhase1(CPMSessionInfo*);
        void CompleteRegisterContentPhase2(CPMSessionInfo*);
        void CompleteRegisterContentPhase3(CPMSessionInfo*);

        void DoApproveUsage(PVMFCPMCommand&);
        PVMFStatus RequestApprovalFromActivePlugIns(PVMFCPMCommand& aCmd);
        void CompleteApproveUsage(CPMContentUsageContext*);
        bool CheckForMetaDataInterfaceAvailability();
        PVMFStatus QueryForMetaDataKeys(PVMFCPMCommand& aParentCmd);
        void CompleteGetMetaDataKeys(uint32);
        bool CheckForGetMetaDataKeysCompletion();

        void DoUsageComplete(PVMFCPMCommand&);
        PVMFStatus SendUsageCompleteToRegisteredPlugIns(PVMFCPMUsageID);
        void CompleteUsageComplete(CPMContentUsageContext*);

        void DoCloseSession(PVMFCPMCommand&);

        void DoReset(PVMFCPMCommand&);
        PVMFStatus ResetRegisteredPlugIns();
        void CompleteCPMReset();

        PVMFStatus DoGetMetadataKeys(PVMFCPMCommand& aCmd);
        void DoGetMetadataValues(PVMFCPMCommand& aCmd);
        PVMFStatus CompleteDoGetMetadataKeys(PVMFCPMCommand& aCmd);
        CPMPlugInParams* LookUpNextPlugInForGetMetaDataValues();
        bool IsGetMetaDataValuesFromPlugInsComplete();
        void SendGetMetaDataValuesToPlugIn(CPMPlugInParams*);
        void CompleteGetMetaDataValues(PVMFCPMCommandContext*);

        void DoQueryInterface(PVMFCPMCommand&);

        PVMFStatus DoGetLicense(PVMFCPMCommand& aCmd,
                                bool aWideCharVersion = false);
        void CompleteGetLicense();
        void DoCancelGetLicense(PVMFCPMCommand& aCmd);

        CPMSessionInfo* LookUpSessionInfo(PVMFSessionId);
        CPMContentUsageContext* LookUpContentUsageContext(PVMFCPMUsageID);
        CPMPlugInParams* LookUpPlugInParams(uint32);
        CPMPlugInParams* LookUpPlugInParamsFromActiveList(uint32);
        int32 PushKVPKey(OSCL_String& aString, PVMFMetadataList& aKeyList);
        PVLogger* iLogger;

        CPMPluginRegistry* iPluginRegistry;
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator> iPlugInParamsVec;
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator> iActivePlugInParamsVec;
        Oscl_Vector<CPMContentUsageContext, OsclMemAllocator> iContentUsageContextVec;
        Oscl_Vector<CPMSessionInfo, OsclMemAllocator> iListofActiveSessions;

        uint32 iNumRegisteredPlugInInitPending;
        uint32 iNumRegisteredPlugInInitComplete;
        uint32 iNumQueryAuthenticationInterfacePending;
        uint32 iNumQueryAuthenticationInterfaceComplete;
        uint32 iNumQueryMetaDataExtensionInterfacePending;
        uint32 iNumQueryMetaDataExtensionInterfaceComplete;
        uint32 iNumQueryCapConfigExtensionInterfacePending;
        uint32 iNumQueryCapConfigExtensionInterfaceComplete;
        uint32 iNumRegisteredPlugInResetPending;
        uint32 iNumRegisteredPlugInResetComplete;

        bool iGetMetaDataKeysFromPlugInsDone;
        bool iGetMetaDataKeysInProgress;

        /* Metadata related */
        uint32 iExtensionRefCount;
        PVMFMetadataList* iKeyListPtr;
        Oscl_Vector<PvmiKvp, OsclMemAllocator>* iValueListPtr;
        int32 iGetMetaDataValuesStartingIndex;
        int32 iGetMetaDataValuesMaxEntries;

        /* License Acquire */
        PVMFCPMPluginLicenseInterface* iLicenseInterface ;
        PVMFCommandId iGetLicenseCmdId;
        PVMFCommandId iCancelGetLicenseCmdId;
};




typedef OSCL_HeapString<OsclMemAllocator> string_key_type;

template<class Alloc>
struct CPMPluginMimeStringCompare
{
    bool operator()(const string_key_type& x, const string_key_type& y) const
    {
        if ((oscl_CIstrcmp(x.get_str(), y.get_str())) < 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

#ifndef CPM_PLUGIN_REGISTRY_H_INCLUDED
#include "cpm_plugin_registry.h"
#endif
#ifndef OSCL_MAP_H_INCLUDED
#include "oscl_map.h"
#endif

class OsclSharedLibraryList;
class CPMPluginRegistryImpl: public CPMPluginRegistry
{
    public:
        typedef CPMPluginMimeStringCompare<OsclMemAllocator> string_key_compare_class;
        typedef Oscl_Map<string_key_type, CPMPluginContainer*, OsclMemAllocator, string_key_compare_class>::value_type value_type;

        //From CPMPluginRegistry
        OSCL_IMPORT_REF bool addPluginToRegistry(OSCL_String& aMimeType,
                CPMPluginContainer& aPlugInContainer) ;
        OSCL_IMPORT_REF void removePluginFromRegistry(OSCL_String& aMimeType) ;
        OSCL_IMPORT_REF CPMPluginContainer* lookupPlugin(OSCL_String& aMimeType) ;
        OSCL_IMPORT_REF uint32 GetNumPlugIns() ;

        OSCL_IMPORT_REF bool GetPluginMimeType(uint32 aIndex, OSCL_String& aMimeType) ;

        OsclSharedLibraryList*& AccessSharedLibraryList()
        {
            return iSharedLibList;
        }

    private:
        friend class CPMPluginRegistryFactory;

        OSCL_IMPORT_REF CPMPluginRegistryImpl();
        OSCL_IMPORT_REF ~CPMPluginRegistryImpl();

        Oscl_Map<string_key_type, CPMPluginContainer*, OsclMemAllocator, string_key_compare_class> iCPMPluginRegistry;

        Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iListofPlugInMimeTypes;

        int32 iRefCount;

        OsclSharedLibraryList* iSharedLibList;
};

#endif //CPM_INTERNAL_H_INCLUDED


