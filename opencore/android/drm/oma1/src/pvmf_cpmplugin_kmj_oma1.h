/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PVMF_CPMPLUGIN_KMJ_OMA1_H_INCLUDED
#define PVMF_CPMPLUGIN_KMJ_OMA1_H_INCLUDED

#include "oscl_base.h"
#include "oscl_types.h"
#include "pvfile.h"
#include "oscl_file_io.h"
#include "pvlogger.h"
#include "pvmf_node_interface.h"
#include "pvmf_node_utils.h"
#include "oscl_scheduler_ao.h"
#include "pvmf_cpmplugin_interface.h"
#include "pvmf_cpmplugin_authorization_interface.h"
#include "pvmf_cpmplugin_authentication_interface.h"
#include "pvmf_cpmplugin_access_interface_factory.h"
#include "pvmf_cpmplugin_access_interface.h"
#include "pvmf_cpmplugin_factory_registry.h"
#include "pvmf_cpmplugin_kmj_oma1_factory.h"
#include "pvmf_cpmplugin_kmj_oma1_types.h"
#include "oscl_string_containers.h"
#include "pvmf_cpmplugin_license_interface.h"
#include "pvmf_meta_data_extension.h"
#include "pvmf_cpmplugin_kmj_oma1_data_access.h"


#define PVMFCPMKmjPlugInOMA1CommandBase PVMFGenericNodeCommand<OsclMemAllocator>

class PVMFCPMKmjPlugInOMA1Command : public PVMFCPMKmjPlugInOMA1CommandBase
{
public:
    void Construct(PVMFSessionId s,
                   int32 cmd,
                   OsclAny* aArg1,
                   OsclAny* aArg2,
                   const OsclAny* aContext)
    {
        PVMFCPMKmjPlugInOMA1CommandBase::Construct(s, cmd, aContext);

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
        PVMFCPMKmjPlugInOMA1CommandBase::Construct(s, cmd, aContext);

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
        PVMFCPMKmjPlugInOMA1CommandBase::Construct(s, cmd, aContext);

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

    /* Constructor and parser for GetLicenseW */
    void Construct(PVMFSessionId s,
                   int32 cmd,
                   OSCL_wString& aContentName,
                   OsclAny* aLicenseData,
                   uint32 aDataSize,
                   int32 aTimeoutMsec,
                   OsclAny* aContext)
    {
        PVMFCPMKmjPlugInOMA1CommandBase::Construct(s,cmd,aContext);

        iParam1 = (OsclAny*)&aContentName;
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
        PVMFCPMKmjPlugInOMA1CommandBase::Construct(s,cmd,aContext);

        iParam1 = (OsclAny*)&aContentName;
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

    // Constructor and parser for GetNodeMetadataKeys
    void Construct( PVMFSessionId s,
                    int32 cmd,
                    PVMFMetadataList& aKeyList,
                    uint32 aStartingIndex,
                    int32 aMaxEntries,
                    char* aQueryKey,
                    const OsclAny* aContext)
    {
        PVMFCPMKmjPlugInOMA1CommandBase::Construct(s,cmd,aContext);

        iParam1 = (OsclAny*)&aKeyList;
        iParam2 = (OsclAny*)aStartingIndex;
        iParam3 = (OsclAny*)aMaxEntries;

        if (aQueryKey)
        {
            //allocate a copy of the query key string.
            Oscl_TAlloc<OSCL_HeapString<OsclMemAllocator>,OsclMemAllocator> str;
            iParam4 = str.ALLOC_AND_CONSTRUCT(aQueryKey);
        }
    }

    void Parse( PVMFMetadataList*& MetaDataListPtr,
                uint32 &aStartingIndex,
                int32 &aMaxEntries,
                char*& aQueryKey)
    {
        MetaDataListPtr = (PVMFMetadataList*)iParam1;
        aStartingIndex = (uint32)iParam2;
        aMaxEntries = (int32)iParam3;
        aQueryKey = NULL;

        if ( iParam4 )
        {
            OSCL_HeapString<OsclMemAllocator>* keystring = (OSCL_HeapString<OsclMemAllocator>*)iParam4;
            aQueryKey = keystring->get_str();
        }
    }

    // Constructor and parser for GetNodeMetadataValue
    void Construct( PVMFSessionId s,
                    int32 cmd,
                    PVMFMetadataList& aKeyList,
                    Oscl_Vector<PvmiKvp,OsclMemAllocator>& aValueList,
                    uint32 aStartIndex,
                    int32 aMaxEntries,
                    const OsclAny* aContext)
    {
        PVMFCPMKmjPlugInOMA1CommandBase::Construct(s,cmd,aContext);

        iParam1 = (OsclAny*)&aKeyList;
        iParam2 = (OsclAny*)&aValueList;
        iParam3 = (OsclAny*)aStartIndex;
        iParam4 = (OsclAny*)aMaxEntries;

    }

    void Parse( PVMFMetadataList* &aKeyList,
                Oscl_Vector<PvmiKvp,OsclMemAllocator>* &aValueList,
                uint32 &aStartingIndex,
                int32 &aMaxEntries)
    {
        aKeyList = (PVMFMetadataList*)iParam1;
        aValueList = (Oscl_Vector<PvmiKvp,OsclMemAllocator>*)iParam2;
        aStartingIndex = (uint32)iParam3;
        aMaxEntries = (int32)iParam4;
    }
};

typedef PVMFNodeCommandQueue<PVMFCPMKmjPlugInOMA1Command,OsclMemAllocator> PVMFCPMKmjPlugInOMA1CmdQ;

#define PVMF_CPM_KMJ_PLUGIN_OMA1_INTERNAL_CMDQ_SIZE 15

/* Start range for command IDs */
#define PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START 50000

/*
 * IDs for all of the asynchronous plugin commands.
 */
enum TPVMFCPMKmjPlugInOMA1Command
{
    PVMF_CPM_KMJ_PLUGIN_OMA1_QUERYUUID = PVMF_GENERIC_NODE_QUERYUUID,
    PVMF_CPM_KMJ_PLUGIN_OMA1_QUERYINTERFACE = PVMF_GENERIC_NODE_QUERYINTERFACE,
    PVMF_CPM_KMJ_PLUGIN_OMA1_INIT = PVMF_GENERIC_NODE_COMMAND_LAST,
    PVMF_CPM_KMJ_PLUGIN_OMA1_RESET,
    PVMF_CPM_KMJ_PLUGIN_OMA1_AUTHENTICATE,
    PVMF_CPM_KMJ_PLUGIN_OMA1_AUTHORIZE_USAGE,
    PVMF_CPM_KMJ_PLUGIN_OMA1_USAGE_COMPLETE,
    PVMF_CPM_KMJ_PLUGIN_OMA1_GET_LICENSE_W,
    PVMF_CPM_KMJ_PLUGIN_OMA1_GET_LICENSE,
    PVMF_CPM_KMJ_PLUGIN_OMA1_CANCEL_GET_LICENSE,
    PVMF_CPM_KMJ_PLUGIN_OMA_GET_NODEMETADATAKEYS,
    PVMF_CPM_KMJ_PLUGIN_OMA_GET_NODEMETADATAVALUES
};

class OsclFileHandle;

// CPM OMA1.0 Plugin for KMJ DRM
class PVMFCPMKmjPlugInOMA1 : public OsclActiveObject,
                             public PVMFCPMPluginInterface,
                             public PVMFCPMPluginAuthenticationInterface,
                             public PVMFCPMPluginAuthorizationInterface,
                             public PVMFCPMPluginAccessInterfaceFactory,
                             public PVMFCPMPluginLicenseInterface,
                             public PVMFMetadataExtensionInterface
{
public:
    /**
     * create this plugin
     * @param  none
     *
     * @returns  this plugin created
     **/
    static PVMFCPMPluginInterface* CreatePlugIn();

    /**
     * destroy this plugin
     *
     * @param  PVMFCPMPluginInterface  the plugin to be destroied
     *
     * @returns  none
     **/
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

    ////////////////* From PVMFCPMPluginInterface */
    /**
     * starts initialization of the plugin
     *
     * @param aSession   a valid session id
     * @param aContext   Optional opaque data to be passed back to user with the command response
     *
     * @returns  a unique commmand id for asynchronous completion
     **/
    PVMFCommandId Init(PVMFSessionId aSession,const OsclAny* aContext = NULL);

    /**
     * This API is to allow for extensibility of the CPM plugin interface.
     * It allows a caller to ask for all UUIDs associated with a particular MIME type.
     * If interfaces of the requested MIME type are found within the system, they are added
     * to the UUIDs array.
     *
     * Also added to the UUIDs array will be all interfaces which have the requested MIME
     * type as a base MIME type.  This functionality can be turned off.
     *
     * @param aMimeType The MIME type of the desired interfaces
     * @param aUuids A vector to hold the discovered UUIDs
     * @param aExactUuidsOnly Turns on/off the retrival of UUIDs with aMimeType as a base type
     * @param aContext Optional opaque data to be passed back to user with the command response
     *
     * @returns A unique command id for asynchronous completion
     **/
    PVMFCommandId QueryUUID( PVMFSessionId aSession,
                             const PvmfMimeString& aMimeType,
                             Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                             bool aExactUuidsOnly = false,
                             const OsclAny* aContext = NULL);

    /**
     * This API is to allow for extensibility of the CPM Plugin interface.
     * It allows a caller to ask for an instance of a particular interface object to be returned.
     * The mechanism is analogous to the COM IUnknown method.  The interfaces are identified with
     * an interface ID that is a UUID as in DCE and a pointer to the interface object is
     * returned if it is supported.  Otherwise the returned pointer is NULL.
     *
     * @param aUuid The UUID of the desired interface
     * @param aInterfacePtr The output pointer to the desired interface
     * @param aContext Optional opaque data to be passed back to user with the command response
     *
     * @returns A unique command id for asynchronous completion
     **/
    PVMFCommandId QueryInterface( PVMFSessionId aSession,
                                  const PVUuid& aUuid,
                                  PVInterface*& aInterfacePtr,
                                  const OsclAny* aContext = NULL);


    /**
     * intimate the plugin of the source content for current playback
     *
     * @param  aSourceURL   string containing the source URL( fila name etc)
     * @param  aSourceFormat  format type of the source(MP4, MP3,ASF, etc)
     * @param  aSourceData  opaque pointer interpreted based on source type
     *
     * @returns   PVMFSuccess  success
     *            PVMFFailure  failure
     *            PVMFErrLicenseRequired  license required
     **/
    PVMFStatus SetSourceInitializationData( OSCL_wString& aSourceURL,
                                            PVMFFormatType& aSourceFormat,
                                            OsclAny* aSourceData);

    /**
     * resets the plugin
     *
     * @param aSession   a valid session id
     * @param aContext   Optional opaque data to be passed back to user with the command response
     *
     * @returns  a unique commmand id for asynchronous completion
     **/
    PVMFCommandId Reset(PVMFSessionId aSession,const OsclAny* aContext = NULL);

    /**
     * returns the cpm content type
     *
     * @param none
     *
     * @returns  PVMF_CPM_FORMAT_OMA1
     **/
    PVMFCPMContentType GetCPMContentType();

    ///////////////////////////////* From PVMFCPMPluginAuthenticationInterface */
    /**
     * asynchronous method to authenticate with a plugin
     *
     * @param aSessionId  a valid session id
     * @param aAuthenticationData the authentication data that may be needed by the plugin to authenticate
     * @param aContext Optional opaque data to be passed back to user with the command response
     *
     * @returns  a unique commmand id for asynchronous completion
     **/
    PVMFCommandId AuthenticateUser( PVMFSessionId aSessionId,
                                    OsclAny* aAuthenticationData,
                                    OsclAny* aContext = NULL);


    ///////////////////////////////* From PVMFCPMPluginAuthorizationInterface */
    /**
     * asynchronous method to athorize usage with a plugin
     *
     * @param aSessionId  a valid session id
     * @param aRequestedUsage the requested usage
     * @param aApprovedUsage  the approved intent
     * @param aAuthorizationData  the authorization data
     * @param aRequestTimeOutInMS timeout with which the authorization request has to complete
     * @param aContext Optional opaque data to be passed back to user with the command response
     *
     * @returns  a unique commmand id for asynchronous completion
     **/
    PVMFCommandId AuthorizeUsage( PVMFSessionId aSessionId,
                                  PvmiKvp& aRequestedUsage,
                                  PvmiKvp& aApprovedUsage,
                                  PvmiKvp& aAuthorizationData,
                                  uint32&  aRequestTimeOutInMS,
                                  OsclAny* aContext = NULL);
    /**
     * asynchronous method to indicate to the plugin that the content usage is complete
     *
     * @param  aSessionId  a valid session id
     * @param  aContext Optional opaque data to be passed back to user with the command response
     *
     * @returns  a unique commmand id for asynchronous completion
     **/
    PVMFCommandId UsageComplete(PVMFSessionId aSessionId,const OsclAny* aContext = NULL);

    //////////////////////////////* From PVMFCPMPluginLicenseInterface */
    /**
     * get license URL
     *
     * @param aSessionId    the assigned plugin session ID to use for this request
     * @param aContentName  Null terminated unicode string containing the content name
     * @param aLicenseURL   Null terminated unicode string containing the license URL
     *
     * @returns  PVMFErrNotSupported   not supported
     **/
    PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                             OSCL_wString& aContentName,
                             OSCL_wString& aLicenseURL)
    {
        //must use Async method.
        return PVMFErrNotSupported;
    }

    /**
     * get license URL
     *
     * @param aSessionId    the assigned plugin session ID to use for this request
     * @param aContentName  Null terminated unicode string containing the content name
     * @param aLicenseURL   Null terminated unicode string containing the license URL
     *
     * @returns  PVMFErrNotSupported   not supported
     **/
    PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                             OSCL_String&  aContentName,
                             OSCL_String&  aLicenseURL)
    {
        //must use Async method.
        return PVMFErrNotSupported;
    }

    /**
     * get license
     *
     * @param aSessionId    the assigned plugin session ID to use for this request
     * @param aContentName  Null terminated unicode string containing the content name
     * @param aData         buffer to store license
     * @param aDataSize     the length of license
     * @param aTimeoutMsec  the timeout for get license
     *
     * @returns  a unique command id for asynchronous completion
     **/
    PVMFCommandId GetLicense(PVMFSessionId aSessionId,
                             OSCL_wString& aContentName,
                             OsclAny* aData,
                             uint32 aDataSize,
                             int32 aTimeoutMsec,
                             OsclAny* aContextData);
    /**
     * get license
     *
     * @param aSessionId    the assigned plugin session ID to use for this request
     * @param aContentName  Null terminated unicode string containing the content name
     * @param aData         buffer to store license
     * @param aDataSize     the length of license
     * @param aTimeoutMsec  the timeout for get license
     * @param aContextData
     *
     * @returns  a unique command id for asynchronous completion
     **/
    PVMFCommandId GetLicense(PVMFSessionId aSessionId,
                             OSCL_String&  aContentName,
                             OsclAny* aData,
                             uint32 aDataSize,
                             int32 aTimeoutMsec,
                             OsclAny* aContextData);

    /**
   * Method to cancel GetLicense requests.
   *
   * @param [in] The assigned plugin session ID to use for this request
   * @param aContext [in] Optional opaque data that will be passed back to
   *                          the user with the command response
   *
   * @returns A unique command id for asynchronous completion.
   */
    PVMFCommandId CancelGetLicense(PVMFSessionId aSessionId,
                                   PVMFCommandId aCmdId,
                                   OsclAny* aContext = NULL);

    /////////////////////////////* From PVMFCPMPluginAccessInterfaceFactory */
    /**
     * ask for all supported access interface UUIDs
     *
     * @param  aUuids   a vector to hold the all supported uuids
     *
     * @returns   PVMFSuccess  success
     *            PVMFFailure  failure
     **/
    PVMFStatus QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids);

    /**
     * create a access interface
     *
     * @param  aUuid   UUID of the interface to be created
     *
     * @returns  the interface pointer on successful creation, NULL otherwise
     **/
    PVInterface* CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid);

    /**
     * destroy a access interface
     *
     * @param  aUuid   UUID of the interface to be destroyed
     * @param  aPtr    ptr to the interface
     *
     * @returns  none
     **/
    void DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid,PVInterface* aPtr);

    /////////////////////////////* From PVMFMetadataExtensionInterface
    /**
     * Synchronous method to return the number of metadata keys for the specified query key string
     *
     * @param aQueryKeyString A NULL terminated character string specifying a subset of metadata keys to count.
     *                        If the string is NULL, total number of all keys will be returned
     *
     * @returns The number of metadata keys
     **/
    uint32 GetNumMetadataKeys(char* aQueryKeyString = NULL);

    /**
     * Synchronous method to return the number of metadata values for the specified list of key strings
     *
     * @param aKeyList A reference to a metadata key list to specifying the values of interest to count
     *
     * @returns The number of metadata values based on the provided key list
     **/
    uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList);

    /**
     * Asynchronous method to retrieve a list of metadata keys. The subset of all available keys in the node can
     * be specified by providing a combination of query key string, starting index, and maximum number of keys to retrieve
     *
     * @param aSessionId The assigned node session ID to use for this request
     * @param aKeyList A reference to a metadata key list to add the metadata keys
     * @param aStartingKeyIndex Index into the node's total key list that corresponds to the first key entry to retrieve
     * @param aMaxKeyEntries The maximum number of key entries to add to aKeyList. If there is no maximum, set to -1.
     * @param aQueryKeyString Optional NULL terminated character string to select a subset of keys
     * @param aContext Optional opaque data to be passed back to user with the command response
     *
     * @returns A unique command ID for asynchronous completion
     **/
    PVMFCommandId GetNodeMetadataKeys( PVMFSessionId aSessionId,
                                       PVMFMetadataList& aKeyList,
                                       uint32 aStartingKeyIndex,
                                       int32 aMaxKeyEntries,
                                       char* aQueryKeyString = NULL,
                                       const OsclAny* aContextData = NULL);

    /**
     * Asynchronous method to retrieve a list of metadata values. The subset of all available values in the node can
     * be specified by providing a combination of key list, starting index, and maximum number of values to retrieve
     *
     * @param aSessionId The assigned node session ID to use for this request
     * @param aKeyList A reference to a metadata key list specifying the metadata values to retrieve
     * @param aValueList A reference to a metadata value list to add the metadata values
     * @param aStartingValueIndex Index into the node's value list specified by the key list that corresponds to the first value entry to retrieve
     * @param aMaxValueEntries The maximum number of value entries to add to aValueList. If there is no maximum, set to -1.
     * @param aContext Optional opaque data to be passed back to user with the command response
     *
     * @returns A unique command ID for asynchronous completion
     **/
    PVMFCommandId GetNodeMetadataValues( PVMFSessionId aSessionId,
                                         PVMFMetadataList& aKeyList,
                                         Oscl_Vector<PvmiKvp,OsclMemAllocator>& aValueList,
                                         uint32 aStartingValueIndex,
                                         int32 aMaxValueEntries,
                                         const OsclAny* aContextData = NULL);

    /**
     * Synchronous method to free up the specified range of metadata keys in the list. It is assumed that caller of this function
     * knows that start and end indices should correspond to metadata keys returned by this particular instance of the
     * metadata extension interface using GetNodeMetadataKeys().
     *
     * @param aKeyList A reference to a metadata key list to free the key entries
     * @param aStartingKeyIndex Index into aKeyList that corresponds to the first key entry to release
     * @param aEndKeyIndex Index into aKeyList that corresponds to the last key entry to release
     *
     * @returns  PVMFSuccess if the release of specified keys succeeded.
     *           PVMFErrArgument if indices are invalid or the list is empty.
     *           PVMFFailure otherwise.
     **/
    PVMFStatus ReleaseNodeMetadataKeys( PVMFMetadataList& aKeyList,
                                        uint32 aStartingKeyIndex,
                                        uint32 aEndKeyIndex);

    /**
     * Synchronous method to free up the specified range of metadata values in the list. It is assumed that caller of this function
     * knows that start and end indices should correspond to metadata values returned by this particular instance of the
     * metadata extension interface using GetNodeMetadataValues().
     *
     * @param aValueList A reference to a metadata value list to free the value entries
     * @param aStartingValueIndex Index into aValueList that corresponds to the first value entry to release
     * @param aEndValueIndex Index into aValueList that corresponds to the last value entry to release
     *
     * @returns PVMFSuccess if the release of specified values succeeded. PVMFErrArgument if indices are invalid or the list is empty.
     *          PVMFFailure otherwise.
     **/
    PVMFStatus ReleaseNodeMetadataValues( Oscl_Vector<PvmiKvp,OsclMemAllocator>& aValueList,
                                          uint32 aStartingValueIndex,
                                          uint32 aEndValueIndex);
PRIVATE:
    /* From PVMFCPMPluginInterface */
    void ThreadLogon();
    void ThreadLogoff();

    PVMFCPMKmjPlugInOMA1(int32 aPriority = OsclActiveObject::EPriorityNominal);
    virtual ~PVMFCPMKmjPlugInOMA1();
    void Run();

    PVMFCommandId QueueCommandL(PVMFCPMKmjPlugInOMA1Command&);
    void MoveCmdToCurrentQueue(PVMFCPMKmjPlugInOMA1Command& aCmd);
    bool ProcessCommand(PVMFCPMKmjPlugInOMA1Command&);
    void CommandComplete( PVMFCPMKmjPlugInOMA1CmdQ&,
                          PVMFCPMKmjPlugInOMA1Command&,
                          PVMFStatus,
                          OsclAny* aData = NULL,
                          PVUuid* aEventUUID = NULL,
                          int32* aEventCode = NULL);

    void DoQueryUuid(PVMFCPMKmjPlugInOMA1Command&);
    void DoQueryInterface(PVMFCPMKmjPlugInOMA1Command&);
    void DoInit(PVMFCPMKmjPlugInOMA1Command&);
    void DoReset(PVMFCPMKmjPlugInOMA1Command&);
    void DoAuthenticate(PVMFCPMKmjPlugInOMA1Command&);
    void DoAuthorizeUsage(PVMFCPMKmjPlugInOMA1Command&);
    void DoUsageComplete(PVMFCPMKmjPlugInOMA1Command& aCmd);
    bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

    int32_t GetMimeType(char* drmFileName);

    // for get license
    void ParseUrl(char* rightsUrl);
    bool getIpAddrByName(OSCL_HeapString<OsclMemAllocator> hostName);
    void DoGetLicense(PVMFCPMKmjPlugInOMA1Command& aCmd,bool aWideCharVersion = false);
    void DoCancelGetLicense(PVMFCPMKmjPlugInOMA1Command& aCmd);

    // For metadata extention interface
    void DoGetMetadataKeys(PVMFCPMKmjPlugInOMA1Command& aCmd);
    void DoGetMetadataValues(PVMFCPMKmjPlugInOMA1Command& aCmd);
    void GetAvailableKeys();
    char* GetIso8601Time(int32_t iDate, int32_t iTime);
    uint32_t GetDuration(int32_t iDay, int32_t iTime);
    bool IsQueryDataAvailable(OSCL_HeapString<OsclMemAllocator> queryData);
    int32_t IsStatelessLicense(void);
    uint8_t* GetLicenseType(void);

    // for get license
    static const uint32_t HTTP_DEFAULT_PORT = 80;
    static const uint32_t URL_MAX_LEN = 1024;
    static const uint32 BUFFER_DEFAULT_CACHE_SIZE = 4096;
    uint32_t urlIpAddr;
    int urlPort;
    OSCL_HeapString<OsclMemAllocator> urlHost;
    OSCL_HeapString<OsclMemAllocator> urlPath;

    PVMFCPMKmjPlugInOMA1CmdQ iInputCommands;
    PVMFCPMKmjPlugInOMA1CmdQ iCurrentCommand;

    uint32 iExtensionRefCount;
    Oscl_Vector<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> iAvailableMetadataKeys;

    // for set data source
    bool m_oSourceSet;
    int32_t m_drmSession;
    PVFile m_pvfile;
    DrmPluginDataAccess* m_dataAccessFile;
    OsclFileHandle* iFileHandle;

    /// the following is used for UT
    bool bRunUT;
    PVMFStatus cmdStatus;

    friend class PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl;
};

class PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl // : public PVMFCPMPluginLocalSyncAccessInterface
{
public:
    PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl(PVMFCPMKmjPlugInOMA1&c)
        : iContainer(c){ }

    virtual ~PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl(){ }

    /* From PVInterface */
    virtual void addRef() {};
    virtual void removeRef() {};
    virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

    /* From PVMFCPMPluginLocalSyncAccessInterface */
    void Init(void) {};
    void Reset(void) {};

    /**
     * Opens the registered content.
     * @return returns 0 if successful and a non-zero value otherwise
     */
    int32 OpenContent();

    /**
     * The File Read & Decrypt operation
     * Reads from the file into the buffer a maximum of 'numelements'
     * of size 'size'.
     *
     * @param buffer pointer to buffer of type void
     * @param size   element size in bytes
     * @param numelements    max number of elements to read
     * @return returns the number of full elements actually read, which
     *         may be less than count if an error occurs or if the end
     *         of the file is encountered before reaching count. Use the
     *         CheckEndOfFile or GetError function to distinguish a read
     *         error from an end-of-file condition.
     */
    uint32 ReadAndUnlockContent(OsclAny *buffer,uint32 size,uint32 numelements);

    /**
     * The File Seek operation
     * Sets the position for file pointer
     * @param offset offset from the specified origin.
     * @param origin starting point
     * @return returns 0 on success, and a non-zero value otherwise
     */
    int32 SeekContent(int32 offset, Oscl_File::seek_type origin);

    /**
     * The File Tell operation
     * Returns the current file position for file specified by fp
     */
    int32 GetCurrentContentPosition();

    /**
     * The File Size operation
     * Returns the file size
     */
    int32 GetContentSize();

    /**
     * The File Close operation
     * Closes the file after flushing any remaining data in the buffers.
     * @return returns 0 if successful, and a non-zero value otherwise
     */
    int32 CloseContent();

    /**
     * The File Flush operation
     * On an output stream OSCL_FileFlush causes any buffered
     * but unwritten data to be written to the file.
     * @return returns 0 if successful, and a non-zero value otherwise
     */
    int32 Flush();

    /**
     * The File Error operation
     * @return  If no error has occurred on stream, returns 0.
     *          Otherwise,it returns a nonzero value
     */
    int32 GetContentAccessError();

    /**
     * Determines if the content is drm protected or not.
     * @param aProtected set to true if protected, false otherwise.
     * Value undefined in case of error
     * @return  returns PVMFSuccess if successful and an appropriate errcode
     *          otherwise
     */
    PVMFStatus IsContentProtected(bool& aProtected)
    {
        aProtected = true;
        return PVMFSuccess;
    }

PRIVATE:
    PVMFCPMKmjPlugInOMA1& iContainer;
    uint32 filePos;
};

#endif


