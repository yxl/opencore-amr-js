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
#include "oscl_utf8conv.h"
#include "oscl_exclusive_ptr.h"
#include <stdio.h>
#include <sys/socket.h>
#include "objmng/svc_drm.h"
#include "xml/xml_tinyParser.h"
#include "pvmi_drm_kvp.h"
#include "pvmf_cpmplugin_kmj_oma1.h"
#include "pvlogger.h"
#include "pvmf_basic_errorinfomessage.h"
#include "pvmf_errorinfomessage_extension.h"
#include "pvmf_local_data_source.h"
#include "pvmf_cpmplugin_kmj_oma1_types.h"
#include "log.h"

PVMFCPMPluginInterface* PVMFCPMKmjPlugInOMA1::CreatePlugIn()
{
    PVMFCPMKmjPlugInOMA1* plugIn = NULL;
    int32 err;

    // Create KMJ OMA1.0 Plugin
    OSCL_TRY(err,plugIn = OSCL_NEW(PVMFCPMKmjPlugInOMA1, ()););

    if (err != OsclErrNone)
    {
        OSCL_LEAVE(err);
    }

    return (OSCL_STATIC_CAST(PVMFCPMPluginInterface*, plugIn));
}

void PVMFCPMKmjPlugInOMA1::DestroyPlugIn(PVMFCPMPluginInterface* aPlugIn)
{
    OSCL_DELETE(aPlugIn);
}

PVMFCPMKmjPlugInOMA1::PVMFCPMKmjPlugInOMA1(int32 aPriority)
                      : OsclActiveObject(aPriority,"PVMFCPMKmjPlugInOMA1"),
                        iExtensionRefCount(0)
{
    bRunUT = false;
    m_oSourceSet = false;
    urlPort = HTTP_DEFAULT_PORT;

    int32 err;

    // Create the input command queue.
    // Use a reserve to avoid lots of dynamic memory allocation
    OSCL_TRY(err, iInputCommands.Construct(PVMF_CPM_KMJ_PLUGIN_OMA1_COMMAND_ID_START,
                                           PVMF_CPM_KMJ_PLUGIN_OMA1_INTERNAL_CMDQ_SIZE);

    /*
     * Create the "current command" queue.
     * It will only contain one command at a time, so use a reserve of 1
     */
    iCurrentCommand.Construct(0,1););

    if (err != OsclErrNone)
    {
        OSCL_LEAVE(err);
    }
}

PVMFCPMKmjPlugInOMA1::~PVMFCPMKmjPlugInOMA1()
{
    m_oSourceSet = false;

    /*
     * Cleanup commands
     * The command queues are self-deleting, but we want to
     * notify the observer of unprocessed commands.
     */
    while (!iCurrentCommand.empty())
    {
        CommandComplete(iCurrentCommand,iCurrentCommand.front(),PVMFFailure);
    }

    while (!iInputCommands.empty())
    {
        CommandComplete(iInputCommands,iInputCommands.front(),PVMFFailure);
    }
}

void PVMFCPMKmjPlugInOMA1::ThreadLogon()
{
    if(!iLoggedOn)
    {
        AddToScheduler();
        iLoggedOn = true;
    }
}

void PVMFCPMKmjPlugInOMA1::ThreadLogoff()
{
    if(iLoggedOn)
    {
        Cancel();
        RemoveFromScheduler();
        iLoggedOn = false;
    }
}

PVMFCommandId PVMFCPMKmjPlugInOMA1::Init(PVMFSessionId s, const OsclAny* aContext)
{
    PVMFCPMKmjPlugInOMA1Command cmd;
    cmd.PVMFCPMKmjPlugInOMA1CommandBase::Construct(s,PVMF_CPM_KMJ_PLUGIN_OMA1_INIT, aContext);
    return QueueCommandL(cmd);
}

PVMFCommandId PVMFCPMKmjPlugInOMA1::QueryUUID( PVMFSessionId s,
                                               const PvmfMimeString& aMimeType,
                                               Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                               bool aExactUuidsOnly,
                                               const OsclAny* aContext)
{
    PVMFCPMKmjPlugInOMA1Command cmd;

    cmd.PVMFCPMKmjPlugInOMA1CommandBase::Construct(s,
                  PVMF_CPM_KMJ_PLUGIN_OMA1_QUERYUUID,
                  aMimeType,
                  aUuids,
                  aExactUuidsOnly,
                  aContext);

    return QueueCommandL(cmd);
}

PVMFCommandId PVMFCPMKmjPlugInOMA1::QueryInterface( PVMFSessionId s,
                                                    const PVUuid& aUuid,
                                                    PVInterface*& aInterfacePtr,
                                                    const OsclAny* aContext)
{
    PVMFCPMKmjPlugInOMA1Command cmd;

    cmd.PVMFCPMKmjPlugInOMA1CommandBase::Construct(s,
                  PVMF_CPM_KMJ_PLUGIN_OMA1_QUERYINTERFACE,
                  aUuid,
                  aInterfacePtr,
                  aContext);

    return QueueCommandL(cmd);
}

PVMFCommandId PVMFCPMKmjPlugInOMA1::AuthenticateUser( PVMFSessionId aSessionId,
                                                      OsclAny* aAuthenticationData,
                                                      OsclAny* aContext)
{
    PVMFCPMKmjPlugInOMA1Command cmd;

    cmd.Construct(aSessionId,
                  PVMF_CPM_KMJ_PLUGIN_OMA1_AUTHENTICATE,
                  aAuthenticationData,
                  NULL,
                  aContext);

    return QueueCommandL(cmd);
}

int32_t PVMFCPMKmjPlugInOMA1::GetMimeType(char* drmFileName)
{
    if(!drmFileName)
    {
        return TYPE_DRM_UNKNOWN;
    }

    const char* extension = strrchr(drmFileName, '.');

    if(!extension)
    {
        return TYPE_DRM_UNKNOWN;
    }

    if (strcasecmp(extension, ".dm") == 0)
    {
        return TYPE_DRM_MESSAGE;
    }
    else if (strcasecmp(extension, ".dcf") == 0)
    {
        return TYPE_DRM_CONTENT;
    }
    else if (strcasecmp(extension, ".dr") == 0)
    {
        return TYPE_DRM_RIGHTS_XML;
    }
    else if (strcasecmp(extension, ".drc") == 0)
    {
        return TYPE_DRM_RIGHTS_WBXML;
    }
    else
    {
        return TYPE_DRM_UNKNOWN;
    }
}

PVMFStatus PVMFCPMKmjPlugInOMA1::SetSourceInitializationData( OSCL_wString& aSourceURL,
                                                              PVMFFormatType& aSourceFormat,
                                                              OsclAny* aSourceData)
{
    LOGD("PVMFCPMKmjPlugInOMA1:SetSourceInitializationData in\r\n");

    char originalFile[256] = {0};

    oscl_UnicodeToUTF8( aSourceURL.get_cstr(),
                        aSourceURL.get_size(),
                        originalFile,
                        256);

    int32_t mimeType = GetMimeType(originalFile);

    Oscl_FileServer fileServ;

    // open local file
    int32 ret = m_pvfile.Open( aSourceURL.get_cstr(),Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,fileServ);
    if(0 != ret)
    {
        LOGD("PVMFCPMKmjPlugInOMA1:SetSourceInitializationData m_pvfile.Open failed");
        return PVMFFailure;
    }

    m_dataAccessFile = OSCL_NEW(DrmPluginDataAccess,(&m_pvfile));

    T_DRM_Input_Data drmInputData =
    {
        (int32_t)m_dataAccessFile,
        mimeType,
        DrmPluginGetDataLen,
        DrmPluginReadData,
        DrmPluginSeekData
    };

    m_drmSession = SVC_drm_openSession(drmInputData);

    if((DRM_MEDIA_DATA_INVALID == m_drmSession) || (DRM_FAILURE == m_drmSession))
    {
        LOGD("PVMFCPMKmjPlugInOMA1:SetSourceInitializationData SVC_drm_openSession failed");
        return PVMFFailure;
    }

    int32_t method = SVC_drm_getDeliveryMethod(m_drmSession);

    if(DRM_FAILURE != method)
    {
        LOGD("PVMFCPMKmjPlugInOMA1:SetSourceInitializationData out\r\n");
        m_oSourceSet = true;
        return PVMFSuccess;
    }
    else
    {
        LOGD("PVMFCPMKmjPlugInOMA1:SetSourceInitializationData unknown delivery type\r\n");
        return PVMFFailure;
    }
}

PVMFCommandId PVMFCPMKmjPlugInOMA1::AuthorizeUsage( PVMFSessionId aSessionId,
                                                    PvmiKvp& aRequestedUsage,
                                                    PvmiKvp& aApprovedUsage,
                                                    PvmiKvp& aAuthorizationData,
                                                    uint32&  aRequestTimeOutInMS,
                                                    OsclAny* aContext)
{
    PVMFCPMKmjPlugInOMA1Command cmd;

    cmd.Construct(aSessionId,
                  PVMF_CPM_KMJ_PLUGIN_OMA1_AUTHORIZE_USAGE,
                  &aRequestedUsage,
                  &aApprovedUsage,
                  &aAuthorizationData,
                  &aRequestTimeOutInMS,
                  aContext);

    return QueueCommandL(cmd);
}

PVMFCommandId PVMFCPMKmjPlugInOMA1::UsageComplete(PVMFSessionId s, const OsclAny* aContext)
{
    PVMFCPMKmjPlugInOMA1Command cmd;

    cmd.PVMFCPMKmjPlugInOMA1CommandBase::Construct( s,
                                                    PVMF_CPM_KMJ_PLUGIN_OMA1_USAGE_COMPLETE,
                                                    aContext);

    return QueueCommandL(cmd);
}

PVMFCommandId PVMFCPMKmjPlugInOMA1::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    PVMFCPMKmjPlugInOMA1Command cmd;

    cmd.PVMFCPMKmjPlugInOMA1CommandBase::Construct(s,PVMF_CPM_KMJ_PLUGIN_OMA1_RESET,aContext);

    return QueueCommandL(cmd);
}


PVMFCPMContentType PVMFCPMKmjPlugInOMA1::GetCPMContentType()
{
    return PVMF_CPM_FORMAT_OMA1;
}


PVMFCommandId PVMFCPMKmjPlugInOMA1::GetLicense( PVMFSessionId aSessionId,
                                                OSCL_wString& aContentName,
                                                OsclAny* aData,
                                                uint32 aDataSize,
                                                int32 aTimeoutMsec,
                                                OsclAny* aContextData)
{
    PVMFCPMKmjPlugInOMA1Command cmd;

    cmd.Construct(aSessionId,
                  PVMF_CPM_KMJ_PLUGIN_OMA1_GET_LICENSE_W,
                  aContentName,
                  aData,
                  aDataSize,
                  aTimeoutMsec,
                  aContextData);

    return QueueCommandL(cmd);
}

PVMFCommandId PVMFCPMKmjPlugInOMA1::GetLicense( PVMFSessionId aSessionId,
                                                OSCL_String&  aContentName,
                                                OsclAny* aData,
                                                uint32 aDataSize,
                                                int32 aTimeoutMsec,
                                                OsclAny* aContextData)
{
    PVMFCPMKmjPlugInOMA1Command cmd;

    cmd.Construct(aSessionId,
                  PVMF_CPM_KMJ_PLUGIN_OMA1_GET_LICENSE,
                  aContentName,
                  aData,
                  aDataSize,
                  aTimeoutMsec,
                  aContextData);

    return QueueCommandL(cmd);
}

PVMFCommandId PVMFCPMKmjPlugInOMA1::CancelGetLicense( PVMFSessionId aSessionId,
                                                      PVMFCommandId aCmdId,
                                                      OsclAny* aContext)
{
    PVMFCPMKmjPlugInOMA1Command cmd;

    cmd.PVMFCPMKmjPlugInOMA1CommandBase::Construct( aSessionId,
                                                  PVMF_CPM_KMJ_PLUGIN_OMA1_CANCEL_GET_LICENSE,
                              aCmdId,
                              aContext);

    return QueueCommandL(cmd);
}


void PVMFCPMKmjPlugInOMA1::ParseUrl(char* rightsUrl)
{
    const char *cur = rightsUrl;
    char buf[URL_MAX_LEN] = {0};  /* Buffer saving fields of url */
    int32_t indx = 0;              /* The current position of url */
    int port = 0;                  /* The port in the url */

    //init host, path and port
    urlHost = "";
    urlPath = "";
    urlPort = HTTP_DEFAULT_PORT;

    /* Parse protocol */
    buf[indx] = 0;

    while (*cur)
    {
        if ((':' == cur[0]) && ('/' == cur[1]) && ('/' == cur[2]))
        {
            /* End the protocol */
            buf[indx] = 0;

            indx = 0;
            cur += 3;

            break;
        }

        buf[indx++] = *cur++;
    }

    /* Only protocol name */
    if (0 == *cur)
    {
        return;
    }

    /* Parse hostname */
    buf[indx] = 0;

    while(1)
    {
        if (':' == cur[0])
        {
            /* The hostname is end and next is port */
            buf[indx] = 0;
            urlHost = buf;

            indx = 0;
            cur += 1;

            /* Parse port */
            while (('0' <= *cur) && ('9' >= *cur))
            {
                port *= 10;
                port += *cur - '0';
                cur++;
            }

            if (port != 0)
            {
                urlPort = port;
            }

            while (('/' != cur[0]) && (0 != *cur))
            {
                cur++;
            }

            break;
        }

        if (cur && (('?' == *cur) || ('#' == *cur) || ('/' == *cur) || (0 == *cur)))
        {
            /* The hostname is end and next is path or null*/
            buf[indx] = 0;
            urlHost = buf;

            indx = 0;
            break;
        }

        buf[indx++] = *cur++;
    }

    /* Parse path */
    if (0 == *cur)  /* No path,and default '/' */
    {
        urlPath = "/";
    }
    else
    {
        indx = 0;
        buf[indx] = 0;

        while(*cur)
        {
            buf[indx++] = *cur++;
        }

        buf[indx] = 0;

        if('/' != *buf)
        {
            urlPath = "/";
        }

        urlPath += buf;
    }
}

bool PVMFCPMKmjPlugInOMA1::getIpAddrByName(OSCL_HeapString<OsclMemAllocator> hostName)
{
    struct hostent *h = NULL;

    urlIpAddr = inet_addr(hostName.get_cstr());

    if(INADDR_NONE != urlIpAddr)
    {
       return true;
    }
    else
    {
        h = gethostbyname(hostName.get_cstr());

        if(NULL == h)
        {
           return false;
        }
    }

    struct in_addr ia;

    /* Parse the hostent structure of h, make the sockaddr structure */
    for (int32_t i = 0; h->h_addr_list[i]; i++)
    {
        if (AF_INET == h->h_addrtype)/* A records (IPv4) */
        {
            memcpy(&ia, h->h_addr_list[i], h->h_length);
        }
        else
        {
            break;
        }

        /* Connect a host by socket address */
        urlIpAddr = ia.s_addr;

        return true;
    }

    return false;
}

void PVMFCPMKmjPlugInOMA1::DoGetLicense(PVMFCPMKmjPlugInOMA1Command& aCmd, bool aWideCharVersion)
{
    char url[256] = {0};
    OsclAny* data = NULL;
    uint32 dataSize = 0;
    int32 timeoutMsec = 0;

    LOGD("PVMFCPMKmjPlugInOMA1:DoGetLicense in");

    if (aWideCharVersion == true)
    {
        OSCL_wString* contentName = NULL;
        aCmd.Parse(contentName,data, dataSize, timeoutMsec);
    }
    else
    {
        OSCL_String* contentName = NULL;
        aCmd.Parse(contentName,data,dataSize,timeoutMsec);
    }

    // get rights url
    if(DRM_SUCCESS != SVC_drm_getRightsIssuer(m_drmSession,(uint8_t*)url))
    {
        LOGD("PVMFCPMKmjPlugInOMA1:DoGetLicense SVC_drm_getRightsIssuer failed");
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
        return;
    }

    // parse URL to get host, port and path
    ParseUrl(url);

    // create a socket
    int s = socket(PF_INET, SOCK_STREAM, 0);

    if(s < 0)
    {
        LOGD("PVMFCPMKmjPlugInOMA1:DoGetLicense socket failed");
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
        return;
    }

    if(false == getIpAddrByName(urlHost))
    {
        LOGD("PVMFCPMKmjPlugInOMA1:DoGetLicense getIpAddrByName failed");
        shutdown(s,true);
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
        return;
    }

    struct sockaddr_in sockin;

    sockin.sin_family = AF_INET;
    sockin.sin_addr.s_addr = ((u_long)urlIpAddr);
    sockin.sin_port = htons((u_short)urlPort);

    // connect to server
    if(0 > connect(s, (struct sockaddr *)&sockin, sizeof(sockin)))
    {
        LOGD("PVMFCPMKmjPlugInOMA1:DoGetLicense connect failed");
        shutdown(s,true);
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
        return;
    }

    // construct send http header
    OSCL_HeapString<OsclMemAllocator> sendHttpHeader = "GET ";

    sendHttpHeader += urlPath;
    sendHttpHeader += " HTTP/1.1\r\nHost:";
    sendHttpHeader += urlHost;
    sendHttpHeader += "\r\n\r\n";

    // send request to server
    if( 0 > send(s,sendHttpHeader.get_cstr(),sendHttpHeader.get_size(),0))
    {
        LOGD("PVMFCPMKmjPlugInOMA1:DoGetLicense send failed");
        shutdown(s,true);
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
        return;
    }

    char out[BUFFER_DEFAULT_CACHE_SIZE] = {0};

    // receive response from server
    int res = recv(s,out,sizeof(out),0);

    shutdown(s,true);

    if(0 > res)
    {
        LOGD("PVMFCPMKmjPlugInOMA1:DoGetLicense receive failed");
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
        return;
    }

    // check http response whether success or not
    char* rightsData = strstr(out, "200 OK");
    if(!rightsData)
    {
        LOGD("PVMFCPMKmjPlugInOMA1:DoGetLicense response invalid");
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
        return;
    }

    int32_t rightsType = TYPE_DRM_RIGHTS_WBXML;

    rightsData = strstr(out, "Content-Type:");
    if (rightsData)
    {
        rightsData += strlen("Content-Type:");

        while (*rightsData == ' ' || *rightsData == '\t')
        {
            rightsData++;
        }

        if (0 == strncmp( "application/vnd.oma.drm.rights+xml",
                          rightsData,
                          strlen("application/vnd.oma.drm.rights+xml")))
        {
            rightsType = TYPE_DRM_RIGHTS_XML;
        }
    }

    int rightsLen = 0;

    rightsData = strstr(out, "\r\n\r\n");
    if (rightsData)
    {
        rightsData += 4;
        rightsLen = res - (rightsData - out);
    }

    DrmPluginDataAccess dataAccess( NULL,
                                    DrmPluginDataAccess::ACCESS_BUFFER,
                                    rightsData,
                                    rightsLen);

    T_DRM_Input_Data drmInputData =
    {
        (int32_t)&dataAccess,
        rightsType,
        DrmPluginGetDataLen,
        DrmPluginReadData,
        DrmPluginSeekData
    };
    T_DRM_Rights_Info rightsInfo;

    memset(&rightsInfo, 0, sizeof(T_DRM_Rights_Info));
    if(DRM_SUCCESS != SVC_drm_installRights(drmInputData, &rightsInfo))
    {
        LOGD("PVMFCPMKmjPlugInOMA1:DoGetLicense SVC_drm_installRights failed");
        CommandComplete(iInputCommands, aCmd, PVMFErrResource);
        return;
    }

    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    LOGD("PVMFCPMKmjPlugInOMA1:DoGetLicense out");
}

void PVMFCPMKmjPlugInOMA1::DoCancelGetLicense(PVMFCPMKmjPlugInOMA1Command& aCmd)
{
    /* extract the command ID from the parameters.*/
    PVMFCommandId id = 0;
    aCmd.PVMFCPMKmjPlugInOMA1CommandBase::Parse(id);

    /* first check "current" command if any */
    PVMFCPMKmjPlugInOMA1Command* cmd = iCurrentCommand.FindById(id);
    if(cmd)
    {
        if ( cmd->iCmd == PVMF_CPM_KMJ_PLUGIN_OMA1_GET_LICENSE_W
            || cmd->iCmd == PVMF_CPM_KMJ_PLUGIN_OMA1_GET_LICENSE)
        {
            CommandComplete(iCurrentCommand, *cmd,PVMFErrCancelled);
            CommandComplete(iInputCommands, aCmd,PVMFSuccess);
            return ;
        }
    }

    /*
     * next check input queue.
     * start at element 1 since this cancel command is element 0.
     */
    cmd = iInputCommands.FindById(id,1);
    if (cmd)
    {
        if ( cmd->iCmd == PVMF_CPM_KMJ_PLUGIN_OMA1_GET_LICENSE_W
            || cmd->iCmd == PVMF_CPM_KMJ_PLUGIN_OMA1_GET_LICENSE)
        {
            /* cancel the queued command */
            CommandComplete(iInputCommands, *cmd, PVMFErrCancelled,NULL,NULL);

            /* report cancel success */
            CommandComplete(iInputCommands, aCmd,PVMFSuccess);
            return ;
        }
    }

    /* if we get here the command isn't queued so the cancel fails */
    CommandComplete(iInputCommands, aCmd,PVMFErrArgument);
    return ;
}

uint32 PVMFCPMKmjPlugInOMA1::GetNumMetadataKeys(char* aQueryKeyString)
{
    GetAvailableKeys();
    uint32_t keySize = iAvailableMetadataKeys.size();

    if(0 == keySize)
    {
        return 0;
    }

    if(NULL == aQueryKeyString)
    {
        return keySize;
    }
    else
    {
        uint32 num_entries = 0;

        // Determine the number of metadata keys based on the query key string provided
        for (uint32 i = 0; i < keySize; i++)
        {
            // Check if the key matches the query key
            if (oscl_strcmp(iAvailableMetadataKeys[i].get_cstr(), aQueryKeyString) == 0)
            {
                num_entries++;
            }
        }

        return num_entries;
    }
}

uint32 PVMFCPMKmjPlugInOMA1::GetNumMetadataValues(PVMFMetadataList& aKeyList)
{
    GetAvailableKeys();

    if(0 == iAvailableMetadataKeys.size())
    {
        return 0;
    }

    uint32_t keySize = aKeyList.size();
    if (0 == keySize)
    {
        return 0;
    }

    uint32 numvalentries = 0;

    for(uint32_t i = 0; i < keySize; i++)
    {
        if(true == IsQueryDataAvailable(aKeyList[i]))
        {
             ++numvalentries;
        }
    }

    return numvalentries;
}

PVMFCommandId PVMFCPMKmjPlugInOMA1::GetNodeMetadataKeys( PVMFSessionId aSessionId,
                                                         PVMFMetadataList& aKeyList,
                                                         uint32 aStartingKeyIndex,
                                                         int32 aMaxKeyEntries,
                                                         char* aQueryKeyString,
                                                         const OsclAny* aContextData)
{
    PVMFCPMKmjPlugInOMA1Command cmd;

    cmd.Construct( aSessionId,
                   PVMF_CPM_KMJ_PLUGIN_OMA_GET_NODEMETADATAKEYS,
                   aKeyList,
                   aStartingKeyIndex,
                   aMaxKeyEntries,
                   aQueryKeyString,
                   aContextData);

    return QueueCommandL(cmd);
}

PVMFCommandId PVMFCPMKmjPlugInOMA1::GetNodeMetadataValues( PVMFSessionId aSessionId,
                                                           PVMFMetadataList& aKeyList,
                                                           Oscl_Vector<PvmiKvp,OsclMemAllocator>& aValueList,
                                                           uint32 aStartingValueIndex,
                                                           int32 aMaxValueEntries,
                                                           const OsclAny* aContextData)
{
    PVMFCPMKmjPlugInOMA1Command cmd;

    uint32_t keySize = aKeyList.size();

    cmd.Construct( aSessionId,
                   PVMF_CPM_KMJ_PLUGIN_OMA_GET_NODEMETADATAVALUES,
                   aKeyList,
                   aValueList,
                   aStartingValueIndex,
                   aMaxValueEntries,
                   aContextData);

    return QueueCommandL(cmd);
}

PVMFStatus PVMFCPMKmjPlugInOMA1::ReleaseNodeMetadataKeys( PVMFMetadataList& aKeyList,
                                                          uint32 aStartingKeyIndex,
                                                          uint32 aEndKeyIndex)
{
    // Nothing needed-- there's no dynamic allocation in this node's key list
    return PVMFSuccess;
}

PVMFStatus PVMFCPMKmjPlugInOMA1::ReleaseNodeMetadataValues( Oscl_Vector<PvmiKvp,OsclMemAllocator>& aValueList,
                                                            uint32 aStartingValueIndex,
                                                            uint32 aEndValueIndex)
{
    uint32_t numVaules = aValueList.size();

    if( (0 == numVaules)
        || (aStartingValueIndex >= numVaules)
        || (aStartingValueIndex > aEndValueIndex))
    {
        return PVMFErrArgument;
    }

    for(uint32 i = 0; i < numVaules; i++)
    {
        if(i < aStartingValueIndex)
        {
            continue;
        }

        if(i > (uint32_t)aEndValueIndex)
        {
            break;
        }

        char* key = aValueList[i].key;

        if (key)
        {
            if(oscl_strstr(key,"char*"))
            {
                if (aValueList[i].value.pChar_value)
                {
                    OSCL_ARRAY_DELETE( aValueList[i].value.pChar_value);
                    aValueList[i].value.pChar_value = NULL;
                }
            }
            else if(oscl_strstr(key,"uint8*"))
            {
                if (aValueList[i].value.pUint8_value)
                {
                    OSCL_ARRAY_DELETE( aValueList[i].value.pUint8_value);
                    aValueList[i].value.pUint8_value = NULL;
                }
            }
        }
    }

    return PVMFSuccess;
}

void PVMFCPMKmjPlugInOMA1::GetAvailableKeys()
{
    iAvailableMetadataKeys.clear();

    T_DRM_Rights_Info rightsInfo;
    oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

    // no rigths, no any available keys
    if(DRM_SUCCESS != SVC_drm_getRightsInfo(m_drmSession,&rightsInfo))
    {
        return;
    }

    if(0 == rightsInfo.playRights.indicator)
    {
        return;
    }

    //init metadata keys
    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_IS_PROTECTED_QUERY);
    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_IS_LICENSE_AVAILABLE_QUERY);
    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_IS_FORWARD_LOCKED_QUERY);
    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_CAN_USE_AS_RINGTONE_QUERY);
    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_LICENSE_TYPE_QUERY);

    //has start time license
    if(rightsInfo.playRights.indicator & DRM_START_TIME_CONSTRAINT)
    {
        iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_LICENSE_START_TIME_QUERY);
    }

    // has count license
    if(rightsInfo.playRights.indicator & DRM_COUNT_CONSTRAINT)
    {
        iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_LICENSE_COUNT_QUERY);
    }

    // has duration license
    if(rightsInfo.playRights.indicator & DRM_INTERVAL_CONSTRAINT)
    {
        iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_LICENSE_DURATION_SINCE_FIRST_USE_QUERY);
    }

    // has end time license
    if(rightsInfo.playRights.indicator & DRM_END_TIME_CONSTRAINT)
    {
        iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_LICENSE_EXPIRATION_TIME_QUERY);
    }

    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_ENVELOPE_DATA_SIZE_QUERY);
    iAvailableMetadataKeys.push_front(PVMF_DRM_INFO_ENVELOPE_DATA_QUERY);
}

bool PVMFCPMKmjPlugInOMA1::IsQueryDataAvailable(OSCL_HeapString<OsclMemAllocator> queryData)
{
    uint32_t numKeys = iAvailableMetadataKeys.size();

    for(uint32_t i = 0; i < numKeys; i++)
    {
        if(oscl_strcmp(queryData.get_cstr(), iAvailableMetadataKeys[i].get_cstr()) == 0)
        {
            return true;
        }
    }

    return false;
}

void PVMFCPMKmjPlugInOMA1::DoGetMetadataKeys(PVMFCPMKmjPlugInOMA1Command& aCmd)
{
    PVMFMetadataList* keylistptr = NULL;
    uint32 starting_index;
    int32 max_entries;
    char* query_key = NULL;

    aCmd.Parse( keylistptr,starting_index,max_entries,query_key);

    GetAvailableKeys();
    uint32_t numKeys = iAvailableMetadataKeys.size();

    if(0 == numKeys)
    {
        CommandComplete(iInputCommands,aCmd,PVMFSuccess);
        return ;
    }

    if((starting_index >= numKeys) || (max_entries < -1))
    {
        CommandComplete(iInputCommands,aCmd,PVMFErrArgument);
        return ;
    }

    if(-1 == max_entries)
    {
        max_entries = numKeys;
    }

    if(starting_index > (uint32)max_entries)
    {
        CommandComplete(iInputCommands,aCmd,PVMFErrArgument);
        return ;
    }

    for(uint32_t i = 0; i < numKeys; i++)
    {
        if(i < starting_index)
        {
            continue;
        }

        if(i > (uint32_t)max_entries)
        {
            break;
        }

        if(query_key == NULL)
        {
            keylistptr->push_back(iAvailableMetadataKeys[i]);
        }
        else
        {
            if(oscl_strcmp(iAvailableMetadataKeys[i].get_cstr(),query_key) == 0)
            {
                keylistptr->push_back(iAvailableMetadataKeys[i]);
            }
        }
    }

    CommandComplete(iInputCommands,aCmd,PVMFSuccess);
}

int32_t PVMFCPMKmjPlugInOMA1::IsStatelessLicense(void)
{
    T_DRM_Rights_Info rights;

    memset(&rights, 0, sizeof(T_DRM_Rights_Info));

    if (DRM_SUCCESS != SVC_drm_getRightsInfo(m_drmSession, &rights))
    {
        return FALSE;
    }

    /* For play permission rights, because this interface is currently provide to PV integrate */
    if (DRM_NO_CONSTRAINT == rights.playRights.indicator)
    {
        return TRUE;
    }

    if (DRM_NO_PERMISSION == rights.playRights.indicator)
    {
        return FALSE;
    }

    if ((0 == (DRM_INTERVAL_CONSTRAINT & rights.playRights.indicator))
        && (0 == (DRM_COUNT_CONSTRAINT & rights.playRights.indicator)))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

uint8_t* PVMFCPMKmjPlugInOMA1::GetLicenseType(void)
{
    T_DRM_Rights_Info rights;

    memset(&rights, 0, sizeof(T_DRM_Rights_Info));

    if (DRM_SUCCESS != SVC_drm_getRightsInfo(m_drmSession, &rights))
    {
        return NULL;
    }

    /* For play permission rights, because this interface is currently provide to PV integrate */
    if (DRM_NO_CONSTRAINT == rights.playRights.indicator)
    {
        return (uint8_t *)"unlimited";
    }

    /* time base */
    if(((DRM_END_TIME_CONSTRAINT | DRM_START_TIME_CONSTRAINT) == rights.playRights.indicator)
       || (DRM_START_TIME_CONSTRAINT == rights.playRights.indicator)
       || (DRM_END_TIME_CONSTRAINT == rights.playRights.indicator))
    {
        return (uint8_t *)"time";
    }

    if(DRM_INTERVAL_CONSTRAINT == rights.playRights.indicator)
    {
        return (uint8_t *)"duration";
    }

    if(DRM_COUNT_CONSTRAINT == rights.playRights.indicator)
    {
        return (uint8_t *)"count";
    }

    if((DRM_END_TIME_CONSTRAINT | DRM_START_TIME_CONSTRAINT | DRM_COUNT_CONSTRAINT) == rights.playRights.indicator)
    {
        return (uint8_t *)"time-count";
    }

    if((DRM_COUNT_CONSTRAINT | DRM_INTERVAL_CONSTRAINT) == rights.playRights.indicator)
    {
        return (uint8_t *)"duration-count";
    }

    if((DRM_END_TIME_CONSTRAINT | DRM_INTERVAL_CONSTRAINT) == rights.playRights.indicator)
    {
        return (uint8_t *)"duration-time";
    }

    if((DRM_END_TIME_CONSTRAINT | DRM_START_TIME_CONSTRAINT | DRM_COUNT_CONSTRAINT | DRM_INTERVAL_CONSTRAINT) == rights.playRights.indicator)
    {
        return (uint8_t *)"duration-time-count";
    }

    return NULL;
}

void PVMFCPMKmjPlugInOMA1::DoGetMetadataValues(PVMFCPMKmjPlugInOMA1Command& aCmd)
{
    PVMFMetadataList* keylistptr = NULL;
    Oscl_Vector<PvmiKvp,OsclMemAllocator>* valuelistptr = NULL;
    uint32 starting_index;
    int32 max_entries;

    aCmd.Parse( keylistptr,valuelistptr,starting_index,max_entries);

    GetAvailableKeys();
    uint32_t numKeys = iAvailableMetadataKeys.size();

    if(0 == numKeys)
    {
        CommandComplete(iInputCommands,aCmd,PVMFSuccess);
        return ;
    }

    uint32 numkeys = keylistptr->size();

    if (numkeys == 0 || starting_index >= numkeys || max_entries < -1)
    {
        CommandComplete(iInputCommands,aCmd,PVMFErrArgument);
        return ;
    }

    if(-1 == max_entries)
    {
        max_entries = iAvailableMetadataKeys.size();
    }

    if(starting_index > (uint32)max_entries)
    {
        CommandComplete(iInputCommands,aCmd,PVMFErrArgument);
        return ;
    }

    for(uint32_t i = 0; i < numkeys; i++)
    {
        if(i < starting_index)
        {
            continue;
        }

        if(i > (uint32_t)max_entries)
        {
            break;
        }

        PvmiKvp KeyVal = {NULL,1,1,{0}};

        /// get is protected
        if (oscl_strstr((*keylistptr)[i].get_cstr(), PVMF_DRM_INFO_IS_PROTECTED_QUERY))
        {
            KeyVal.key = (char*)PVMF_DRM_INFO_IS_PROTECTED_VALUE;
            KeyVal.value.bool_value = true;  ///drm 1.0 file is always protected
        }
        /// get is license available
        else if (oscl_strstr((*keylistptr)[i].get_cstr(), PVMF_DRM_INFO_IS_LICENSE_AVAILABLE_QUERY))
        {
            KeyVal.key = (char*)PVMF_DRM_INFO_IS_LICENSE_AVAILABLE_VALUE;

            // because pv only support audio and vedio, so we only check play license
            if(DRM_SUCCESS == SVC_drm_checkRights(m_drmSession,DRM_PERMISSION_PLAY))
            {
                KeyVal.value.bool_value = true;
            }
            else
            {
                KeyVal.value.bool_value = false;
            }
        }
        /// get is forward-lock
        else if (oscl_strstr((*keylistptr)[i].get_cstr(), PVMF_DRM_INFO_IS_FORWARD_LOCKED_QUERY))
        {
            KeyVal.key = (char*)PVMF_DRM_INFO_IS_FORWARD_LOCKED_VALUE;

            if(SEPARATE_DELIVERY == SVC_drm_getDeliveryMethod(m_drmSession))
            {
                KeyVal.value.bool_value = false;
            }
            else
            {
                KeyVal.value.bool_value = true;
            }
        }
        /// get can be used as ring-tone
        else if (oscl_strstr((*keylistptr)[i].get_cstr(), PVMF_DRM_INFO_CAN_USE_AS_RINGTONE_QUERY))
        {
            KeyVal.key = (char*)PVMF_DRM_INFO_CAN_USE_AS_RINGTONE_VALUE;

            if(TRUE == IsStatelessLicense())
            {
                KeyVal.value.bool_value = true;
            }
            else
            {
                KeyVal.value.bool_value = false;
            }
        }
        /// get license type
        else if (oscl_strstr((*keylistptr)[i].get_cstr(), PVMF_DRM_INFO_LICENSE_TYPE_QUERY))
        {
            char* value = (char*)GetLicenseType();

            if(NULL == value)
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            uint32 valuelen = oscl_strlen(value) + 1;

            char* metaKeyValue = OSCL_ARRAY_NEW(char, valuelen);
            if(NULL == metaKeyValue)
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            oscl_memset(metaKeyValue,0,valuelen);
            oscl_memcpy(metaKeyValue,value,valuelen);

            KeyVal.key = (char*)PVMF_DRM_INFO_LICENSE_TYPE_VALUE;
            KeyVal.length = valuelen;
            KeyVal.capacity = valuelen;
            KeyVal.value.pChar_value = metaKeyValue;
        }
        /// get license count
        else if (oscl_strstr((*keylistptr)[i].get_cstr(), PVMF_DRM_INFO_LICENSE_COUNT_QUERY))
        {
            T_DRM_Rights_Info rightsInfo;
            oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

            if(DRM_SUCCESS != SVC_drm_getRightsInfo(m_drmSession,&rightsInfo))
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            /// no license count
            if(0 == (rightsInfo.playRights.indicator & DRM_COUNT_CONSTRAINT))
            {
                continue;
            }

            KeyVal.key = (char*)PVMF_DRM_INFO_LICENSE_COUNT_VALUE;

            // because pv only support audio and vedio, so we only check play count
            KeyVal.value.uint32_value = rightsInfo.playRights.count;
        }
        /// get license start time
        else if (oscl_strstr((*keylistptr)[i].get_cstr(), PVMF_DRM_INFO_LICENSE_START_TIME_QUERY))
        {
            T_DRM_Rights_Info rightsInfo;
            oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

            if(DRM_SUCCESS != SVC_drm_getRightsInfo(m_drmSession,&rightsInfo))
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            // no license start time
            if(0 == (rightsInfo.playRights.indicator & DRM_START_TIME_CONSTRAINT))
            {
                continue;
            }

            int32_t startDate = rightsInfo.playRights.startDate;
            int32_t startTime = rightsInfo.playRights.startTime;

            if(startDate < 0 || startTime < 0)
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            //convert to ISO 8601 Timeformat
            char* p = GetIso8601Time(startDate,startTime);
            if(p == NULL)
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            uint32_t len = oscl_strlen(p) + 1;

            KeyVal.key = (char*)PVMF_DRM_INFO_LICENSE_START_TIME_VALUE;
            KeyVal.length = len;
            KeyVal.capacity = len;
            KeyVal.value.pChar_value = p;
        }
        /// get license expiration time
        else if (oscl_strstr((*keylistptr)[i].get_cstr(), PVMF_DRM_INFO_LICENSE_EXPIRATION_TIME_QUERY))
        {
            T_DRM_Rights_Info rightsInfo;
            oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

            if(DRM_SUCCESS != SVC_drm_getRightsInfo(m_drmSession,&rightsInfo))
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            // no license expiration time
            if(0 == (rightsInfo.playRights.indicator & DRM_END_TIME_CONSTRAINT))
            {
                continue;
            }

            int32_t endDate = rightsInfo.playRights.endDate;
            int32_t endTime = rightsInfo.playRights.endTime;

            if(endDate < 0 || endTime < 0)
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            //convert to ISO 8601 Timeformat
            char* p = GetIso8601Time(endDate,endTime);
            if(p == NULL)
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            uint32_t len = oscl_strlen(p) + 1;

            KeyVal.key = (char*)PVMF_DRM_INFO_LICENSE_EXPIRATION_TIME_VALUE;
            KeyVal.length = len;
            KeyVal.capacity = len;
            KeyVal.value.pChar_value = p;
        }
        /// get license remain duration
        else if (oscl_strstr((*keylistptr)[i].get_cstr(), PVMF_DRM_INFO_LICENSE_DURATION_SINCE_FIRST_USE_QUERY))
        {
            T_DRM_Rights_Info rightsInfo;
            oscl_memset(&rightsInfo,0,sizeof(T_DRM_Rights_Info));

            if(DRM_SUCCESS != SVC_drm_getRightsInfo(m_drmSession,&rightsInfo))
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            // no license remain duration
            if(0 == (rightsInfo.playRights.indicator & DRM_INTERVAL_CONSTRAINT))
            {
                continue;
            }

            int32_t intervalDate = rightsInfo.playRights.intervalDate;
            int32_t intervalTime = rightsInfo.playRights.intervalTime;

            if(intervalDate < 0 || intervalTime < 0)
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            KeyVal.key = (char*)PVMF_DRM_INFO_LICENSE_DURATION_SINCE_FIRST_USE_VALUE;
            KeyVal.value.uint32_value = GetDuration(intervalDate,intervalTime);
        }
        /// get enveloped content length
        else if (oscl_strstr((*keylistptr)[i].get_cstr(), PVMF_DRM_INFO_ENVELOPE_DATA_SIZE_QUERY))
        {
            int32_t contentSize = SVC_drm_getContentLength(m_drmSession);

            if(0 == contentSize)
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            KeyVal.key = (char*)PVMF_DRM_INFO_ENVELOPE_DATA_SIZE_VALUE;
            KeyVal.value.uint32_value = (uint32_t)contentSize;
        }
        /// get enveloped content
        else if (oscl_strstr((*keylistptr)[i].get_cstr(), PVMF_DRM_INFO_ENVELOPE_DATA_QUERY))
        {
            int32_t contentSize = SVC_drm_getContentLength(m_drmSession);

            if(0 == contentSize)
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            uint8_t* buf = OSCL_ARRAY_NEW(uint8_t,contentSize);
            if(NULL == buf)
            {
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            oscl_memset(buf,0,contentSize);

            if(0 >= SVC_drm_getContent(m_drmSession,0,buf,contentSize))
            {
                OSCL_ARRAY_DELETE(buf);
                CommandComplete(iInputCommands,aCmd,PVMFFailure);
                return;
            }

            KeyVal.key = (char*)PVMF_DRM_INFO_ENVELOPE_DATA_VALUE;
            KeyVal.length = contentSize;
            KeyVal.capacity = contentSize;
            KeyVal.value.pUint8_value = buf;
        }
        else
        {
            continue;
        }

        // adds metavalue
        valuelistptr->push_back(KeyVal);
    }

    CommandComplete(iInputCommands,aCmd,PVMFSuccess);
}

/// iDate Like: 20070915,  iTime like: 200202
/// ISO 8601 time like: 2007-09-15T20:02:02Z
char* PVMFCPMKmjPlugInOMA1::GetIso8601Time(int32_t iDate, int32_t iTime)
{
    char temp[20] = {0};
    char* p = temp;
    uint32_t digitalNum = 0;
    uint32_t count = 0;
    OSCL_HeapString<OsclMemAllocator> isoTime;

    // convert date
    do
    {
        // insert '-' between YYYY and MM and DD
        if( count == 2 || count == 4)
        {
            *p++ = '-';
        }

        digitalNum = iDate % 10;
        iDate /= 10;

        *p++ = (char)(digitalNum + '0');

        count++;
    }while(iDate > 0);

    if(8 != count)
    {
        return NULL;
    }

    p--;

    /* We now have the digit of the number in the buffer, but in reverse
       order.  Thus we reverse them now. */
    do
    {
        isoTime += *p;
        p--;
    }while(p >= temp);

    // insert date and time separator
    isoTime += 'T';

    oscl_memset(temp,0,sizeof(temp));
    p = temp;
    count = 0;

    // convert time
    do
    {
        // insert ':' between HH and MM and SS
        if( count == 2 || count == 4)
        {
            *p++ = ':';
        }

        digitalNum = iTime % 10;
        iTime /= 10;

        *p++ = (char)(digitalNum + '0');

        count++;
    }while(iTime > 0);

    if((6 != count) && (5 != count))
    {
        return NULL;
    }

    p--;

    /* We now have the digit of the number in the buffer, but in reverse
       order.  Thus we reverse them now. */
    do
    {
        isoTime += *p;
        p--;
    }while(p >= temp);

    /// 'Z' means UTC time
    isoTime += 'Z';

    uint32_t len = oscl_strlen(isoTime.get_cstr()) + 1;

    p = OSCL_ARRAY_NEW(char,len);

    if(p == NULL)
    {
        return NULL;
    }

    oscl_memset(p,0,len);
    oscl_strncpy(p,isoTime.get_cstr(),len);

    return p;
}

/// convert day and time to seconds
uint32_t PVMFCPMKmjPlugInOMA1::GetDuration(int32_t iDay, int32_t iTime)
{
    if((iDay < 0) || (iTime < 0))
    {
        return 0;
    }

    if((0 == iDay) && (0 == iTime))
    {
        return 0;
    }

    uint32_t duration = iDay * 24 * 60 * 60;
    uint32_t hours = iTime / 10000;
    uint32_t minutes = (iTime / 100) % 100;
    uint32_t seconds = iTime % 100;

    duration += hours * 60 * 60;
    duration += minutes * 60;
    duration += seconds;

    return duration;
}

// Called by the command handler AO to do the Query UUID
void PVMFCPMKmjPlugInOMA1::DoQueryUuid(PVMFCPMKmjPlugInOMA1Command& aCmd)
{
    PvmfMimeString* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;

    aCmd.PVMFCPMKmjPlugInOMA1CommandBase::Parse(mimetype, uuidvec, exactmatch);

    // Try to match the input mimetype against any of the interfaces supported by this plugin
    if (*mimetype == PVMF_CPMPLUGIN_AUTHENTICATION_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(PVMFCPMPluginAuthenticationInterfaceUuid);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMF_CPMPLUGIN_AUTHORIZATION_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(PVMFCPMPluginAuthorizationInterfaceUuid);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMF_CPMPLUGIN_ACCESS_INTERFACE_FACTORY_MIMETYPE)
    {
        PVUuid uuid(PVMFCPMPluginAccessInterfaceFactoryUuid);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMF_CPMPLUGIN_LICENSE_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(PVMFCPMPluginLicenseInterfaceUuid);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMF_META_DATA_EXTENSION_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(KPVMFMetadataExtensionUuid);
        uuidvec->push_back(uuid);
    }

    CommandComplete(iInputCommands,aCmd,PVMFSuccess);
}

// Called by the command handler AO to do the Query Interface.
void PVMFCPMKmjPlugInOMA1::DoQueryInterface(PVMFCPMKmjPlugInOMA1Command& aCmd)
{
    PVUuid* uuid;
    PVInterface** ptr;

    aCmd.PVMFCPMKmjPlugInOMA1CommandBase::Parse(uuid, ptr);

    if ((*uuid == PVMFCPMPluginAuthorizationInterfaceUuid)
        || (*uuid == PVMFCPMPluginAccessInterfaceFactoryUuid))
    {
        if (m_oSourceSet == false)
        {
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            return;
        }
    }

    if(queryInterface(*uuid, *ptr))
    {
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else  // not supported
    {
        *ptr = NULL;

        CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
    }
}

bool PVMFCPMKmjPlugInOMA1::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if(uuid == PVMFCPMPluginAuthenticationInterfaceUuid)
    {
        PVMFCPMPluginAuthenticationInterface* myInterface
            = OSCL_STATIC_CAST(PVMFCPMPluginAuthenticationInterface*,this);

        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if(uuid == PVMFCPMPluginAuthorizationInterfaceUuid)
    {
        PVMFCPMPluginAuthorizationInterface* myInterface
            = OSCL_STATIC_CAST(PVMFCPMPluginAuthorizationInterface*,this);

        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (uuid == PVMFCPMPluginAccessInterfaceFactoryUuid)
    {
        PVMFCPMPluginAccessInterfaceFactory* myInterface
            = OSCL_STATIC_CAST(PVMFCPMPluginAccessInterfaceFactory*,this);

        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (uuid == PVMFCPMPluginLicenseInterfaceUuid)
    {
        PVMFCPMPluginLicenseInterface* myInterface
            = OSCL_STATIC_CAST(PVMFCPMPluginLicenseInterface*,this);

        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (uuid == KPVMFMetadataExtensionUuid)
    {
        PVMFMetadataExtensionInterface* myInterface
            = OSCL_STATIC_CAST(PVMFMetadataExtensionInterface*,this);

        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else
    {
        return false;
    }

    addRef();
    return true;
}

void PVMFCPMKmjPlugInOMA1::DoInit(PVMFCPMKmjPlugInOMA1Command& aCmd)
{
    CommandComplete(iInputCommands,aCmd,PVMFSuccess);
}

void PVMFCPMKmjPlugInOMA1::DoReset(PVMFCPMKmjPlugInOMA1Command& aCmd)
{
    m_oSourceSet = false;
    iAvailableMetadataKeys.clear();

  if(true == m_pvfile.IsOpen())
  {
      m_pvfile.Close();
  }

    if(m_dataAccessFile)
    {
        OSCL_DELETE(m_dataAccessFile);
        m_dataAccessFile = NULL;
    }

    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

void PVMFCPMKmjPlugInOMA1::DoAuthenticate(PVMFCPMKmjPlugInOMA1Command& aCmd)
{
    OsclAny* authenticationData;
    OsclAny* dummy;
    aCmd.Parse(authenticationData,dummy);

    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

void PVMFCPMKmjPlugInOMA1::DoAuthorizeUsage(PVMFCPMKmjPlugInOMA1Command& aCmd)
{
    PvmiKvp* requestedUsage;
    PvmiKvp* approvedUsage;
    PvmiKvp* authorizationData;
    uint32*  requestTimeOutInMS;

    aCmd.Parse(OSCL_STATIC_CAST(OsclAny*&,requestedUsage),
               OSCL_STATIC_CAST(OsclAny*&,approvedUsage),
               OSCL_STATIC_CAST(OsclAny*&,authorizationData),
               OSCL_STATIC_CAST(OsclAny*&,requestTimeOutInMS));

    //Get DRM delivery type
    int32_t drmMethod = SVC_drm_getDeliveryMethod(m_drmSession);

    //Check and update the rights
    switch (drmMethod)
    {
        case FORWARD_LOCK:
            break;

        case COMBINED_DELIVERY:
        case SEPARATE_DELIVERY:
        case SEPARATE_DELIVERY_FL:
        {
            if (BITMASK_PVMF_CPM_DRM_INTENT_PLAY & requestedUsage->value.uint32_value)
            {
                //Check whether there is a valid rights
                if (DRM_SUCCESS != SVC_drm_checkRights(m_drmSession, DRM_PERMISSION_PLAY))
                {
                    if( (SEPARATE_DELIVERY == drmMethod ) || (SEPARATE_DELIVERY_FL == drmMethod))
                    {
                        CommandComplete(iInputCommands, aCmd,PVMFErrLicenseRequired);
                    }
                    else
                    {
                        CommandComplete(iInputCommands, aCmd, PVMFErrAccessDenied);
                    }

                    LOGD("PVMFCPMKmjPlugInOMA1:DoAuthorizeUsage SVC_drm_checkRights failed");
                    return;
               }
            }
            else  //unsupported rights
            {
                LOGD("PVMFCPMKmjPlugInOMA1:DoAuthorizeUsage unsupported rights");
                CommandComplete(iInputCommands, aCmd, PVMFErrAccessDenied);
                return;
            }
        }
            break;
        default:
            CommandComplete(iInputCommands, aCmd, PVMFFailure);
            return;
    }

    approvedUsage->value.uint32_value = requestedUsage->value.uint32_value;
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

void PVMFCPMKmjPlugInOMA1::DoUsageComplete(PVMFCPMKmjPlugInOMA1Command& aCmd)
{
    //consume rights
    if (DRM_SUCCESS != SVC_drm_consumeRights(m_drmSession, DRM_PERMISSION_PLAY))
    {
        LOGD("PVMFCPMKmjPlugInOMA1:DoAuthorizeUsage SVC_drm_consumeRights failed");
        CommandComplete(iInputCommands, aCmd, PVMFErrAccessDenied);
        return;
    }

    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

PVMFStatus PVMFCPMKmjPlugInOMA1::QueryAccessInterfaceUUIDs( Oscl_Vector<PVUuid,
                                                            OsclMemAllocator>& aUuids)
{
    aUuids.push_back(PVMFCPMPluginLocalSyncAccessInterfaceUuid);
    return PVMFSuccess;
}

PVInterface* PVMFCPMKmjPlugInOMA1::CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid)
{
    if (aUuid == PVMFCPMPluginLocalSyncAccessInterfaceUuid)
    {
        PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl* iface
            = OSCL_NEW( PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl,(*this));

        PVMFCPMPluginLocalSyncAccessInterface* interimPtr1
            = OSCL_STATIC_CAST(PVMFCPMPluginLocalSyncAccessInterface*, iface);

        PVMFCPMPluginAccessInterface* interimPtr2
            = OSCL_STATIC_CAST(PVMFCPMPluginAccessInterface*, interimPtr1);

        return (OSCL_STATIC_CAST(PVInterface*, interimPtr2));
    }

    return NULL;
}

void PVMFCPMKmjPlugInOMA1::DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid,PVInterface* aPtr)
{
    if (aUuid == PVMFCPMPluginLocalSyncAccessInterfaceUuid)
    {
        PVMFCPMPluginAccessInterface* interimPtr1
            = OSCL_STATIC_CAST(PVMFCPMPluginAccessInterface*,aPtr);

        PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl* iface
            = OSCL_STATIC_CAST(PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl*,interimPtr1);

        OSCL_DELETE(iface);
    }
    else
    {
        OSCL_ASSERT(false);
    }
}

/**
 * This routine is called by various command APIs to queue an
 * asynchronous command for processing by the command handler AO.
 * This function may leave if the command can't be queued due to memory allocation failure.
 */
PVMFCommandId PVMFCPMKmjPlugInOMA1::QueueCommandL(PVMFCPMKmjPlugInOMA1Command& aCmd)
{
    PVMFCommandId id = iInputCommands.AddL(aCmd);

    /* wakeup the AO */
    if(false == bRunUT)
    {
        RunIfNotReady();
    }

    return id;
}

// The various command handlers call this when a command is complete.
void PVMFCPMKmjPlugInOMA1::CommandComplete( PVMFCPMKmjPlugInOMA1CmdQ& aCmdQ,
                                            PVMFCPMKmjPlugInOMA1Command& aCmd,
                                            PVMFStatus aStatus,
                                            OsclAny* aEventData,
                                            PVUuid* aEventUUID,
                                            int32* aEventCode)
{
    PVInterface* extif = NULL;
    PVMFBasicErrorInfoMessage* errormsg = NULL;

    if (aEventUUID && aEventCode)
    {
        errormsg = OSCL_NEW(PVMFBasicErrorInfoMessage,(*aEventCode,*aEventUUID,NULL));
        extif = OSCL_STATIC_CAST(PVInterface*,errormsg);
    }

    /* create event response */
    PVMFCmdResp resp(aCmd.iId,aCmd.iContext,aStatus,extif,aEventData);
    PVMFSessionId session = aCmd.iSession;

    /* Erase the command from the queue */
    aCmdQ.Erase(&aCmd);

    /* Report completion to the session observer */
    ReportCmdCompleteEvent(session,resp);

    if (errormsg)
    {
        errormsg->removeRef();
    }

    //Reschedule if there are more commands.
    if(!iInputCommands.empty() && IsAdded())
    {
        RunIfNotReady();
    }

    cmdStatus = aStatus;
}

void PVMFCPMKmjPlugInOMA1::MoveCmdToCurrentQueue(PVMFCPMKmjPlugInOMA1Command& aCmd)
{
    int32 err;
    OSCL_TRY(err,iCurrentCommand.StoreL(aCmd););

    if (err != OsclErrNone)
    {
        CommandComplete(iInputCommands,aCmd,PVMFErrNoMemory);
        return;
    }

    iInputCommands.Erase(&aCmd);
    return;
}

/**
  * This AO handles API commands. The AO will process one command  per call.
  * It will re-schedule itself and run continuously until it runs out of things to do.
  */
void PVMFCPMKmjPlugInOMA1::Run()
{
    // Process commands.
    if (!iInputCommands.empty())
    {
        ProcessCommand(iInputCommands.front());
    }
}

/**
 * Called by the command handler AO to process a command from the input queue.
 * Return true if a command was processed, false if the command
 * processor is busy and can't process another command now.
 */
bool PVMFCPMKmjPlugInOMA1::ProcessCommand(PVMFCPMKmjPlugInOMA1Command& aCmd)
{
    /*
     * normally this node will not start processing one command until the prior one is finished.
     * However, a hi priority command such as Cancel must be able to interrupt a command in progress.
     */
    if (!iCurrentCommand.empty()
        && !aCmd.hipri()
        && aCmd.iCmd != PVMF_CPM_KMJ_PLUGIN_OMA1_CANCEL_GET_LICENSE)
    {
        return false;
    }

    switch(aCmd.iCmd)
    {
        case PVMF_CPM_KMJ_PLUGIN_OMA1_QUERYUUID:
            DoQueryUuid(aCmd);
            break;
        case PVMF_CPM_KMJ_PLUGIN_OMA1_QUERYINTERFACE:
            DoQueryInterface(aCmd);
            break;
        case PVMF_CPM_KMJ_PLUGIN_OMA1_INIT:
            DoInit(aCmd);
            break;
        case PVMF_CPM_KMJ_PLUGIN_OMA1_RESET:
            DoReset(aCmd);
            break;
        case PVMF_CPM_KMJ_PLUGIN_OMA1_AUTHENTICATE:
            DoAuthenticate(aCmd);
            break;
        case PVMF_CPM_KMJ_PLUGIN_OMA1_AUTHORIZE_USAGE:
            DoAuthorizeUsage(aCmd);
            break;
        case PVMF_CPM_KMJ_PLUGIN_OMA1_USAGE_COMPLETE:
            DoUsageComplete(aCmd);
            break;
        case PVMF_CPM_KMJ_PLUGIN_OMA1_GET_LICENSE_W:
            DoGetLicense(aCmd,true);
            break;
        case PVMF_CPM_KMJ_PLUGIN_OMA1_GET_LICENSE:
            DoGetLicense(aCmd);
            break;
        case PVMF_CPM_KMJ_PLUGIN_OMA1_CANCEL_GET_LICENSE:
            DoCancelGetLicense(aCmd);
            break;
        case PVMF_CPM_KMJ_PLUGIN_OMA_GET_NODEMETADATAKEYS:
            DoGetMetadataKeys(aCmd);
            break;
        case PVMF_CPM_KMJ_PLUGIN_OMA_GET_NODEMETADATAVALUES:
            DoGetMetadataValues(aCmd);
            break;
        default:
            /* unknown command type */
            CommandComplete(iInputCommands,aCmd,PVMFErrNotSupported);
            break;
    }

    return true;
}

int32 PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl::OpenContent()
{
    //file should has been opened by outer when come to here
    if(false == iContainer.m_pvfile.IsOpen())
    {
    return -1;
    }

    filePos = 0;
    return 0;
}

uint32 PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl::ReadAndUnlockContent( OsclAny *buffer,
                                                                               uint32 size,
                                                                               uint32 numelements)
{
    uint32 readLen = size * numelements;
    int32_t ret = SVC_drm_getContent(iContainer.m_drmSession,filePos,(uint8_t*)buffer,readLen);
    LOGD("in drm plugin ReadAndUnlockContent: require read len: %d, actul read len: %d\r\n",readLen, ret);
    filePos += readLen;
    return readLen;
}

int32 PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl::SeekContent( int32 offset,
                                                                     Oscl_File::seek_type origin)
{
    switch(origin)
    {
        case Oscl_File::SEEKSET:
            filePos = offset;
            break;
        case Oscl_File::SEEKCUR:
            filePos += offset;
            break;
        case Oscl_File::SEEKEND:
            filePos = SVC_drm_getContentLength(iContainer.m_drmSession);
            break;
    }

    return 0;
}

int32 PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl::GetCurrentContentPosition()
{
    return filePos;
}

int32 PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl::Flush()
{
    return iContainer.m_pvfile.Flush();
}

int32 PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl::GetContentAccessError()
{
    return 0;
}

int32 PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl::GetContentSize()
{
    return SVC_drm_getContentLength(iContainer.m_drmSession);
}

int32 PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl::CloseContent()
{
  //file would be closed by outer
    filePos = 0;
    return 0;
}

bool PVMFCPMKmjPlugInOMA1LocalSyncAccessInterfaceImpl::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == PVMFCPMPluginLocalSyncAccessInterfaceUuid)
    {
        PVMFCPMPluginLocalSyncAccessInterface* interimPtr1
            = OSCL_STATIC_CAST(PVMFCPMPluginLocalSyncAccessInterface*, this);

        PVMFCPMPluginAccessInterface* interimPtr2
            = OSCL_STATIC_CAST(PVMFCPMPluginAccessInterface*, interimPtr1);

        iface = OSCL_STATIC_CAST(PVInterface*, interimPtr2);
        return true;
    }

    return false;
}

