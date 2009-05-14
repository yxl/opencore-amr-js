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
#ifndef PVMF_STREAMING_MANAGER_INTERNAL_H_INCLUDED
#include "pvmf_streaming_manager_internal.h"
#endif
#ifndef PVMF_STREAMING_MANAGER_NODE_H_INCLUDED
#include "pvmf_streaming_manager_node.h"
#endif
#ifndef PV_LOGGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_SOCKET_NODE_H_INCLUDED
#include "pvmf_socket_node.h"
#endif
#ifndef PVRTSP_ENGINE_NODE_EXTENSION_INTERFACE_H_INCLUDED
#include "pvrtspenginenodeextensioninterface.h"
#endif
#ifndef PVMF_JITTER_BUFFER_NODE_H_INCLUDED
#include "pvmf_jitter_buffer_node.h"
#endif
#ifndef PVMF_MEDIALAYER_NODE_H_INCLUDED
#include "pvmf_medialayer_node.h"
#endif

/**
 * Macros for calling PVLogger
 */
#define LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFO(m) LOGINFOMED(m)


PVMFStreamingManagerExtensionInterfaceImpl::PVMFStreamingManagerExtensionInterfaceImpl(PVMFStreamingManagerNode*c,
        PVMFSessionId sid)
        : PVInterfaceImpl<PVMFStreamingManagerNodeAllocator>(PVUuid(PVMF_STREAMINGMANAGERNODE_EXTENSIONINTERFACE_UUID))
        , iSessionId(sid)
        , iContainer(c)

{}

PVMFStreamingManagerExtensionInterfaceImpl::~PVMFStreamingManagerExtensionInterfaceImpl()
{}

OSCL_EXPORT_REF
PVMFStatus PVMFStreamingManagerExtensionInterfaceImpl::setClientParameters(PVMFSMClientParams* clientParams)
{
    PVMFSMNodeContainer* iSessionControllerNodeContainer =
        iContainer->getNodeContainer(PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_NODE);

    if (iSessionControllerNodeContainer == NULL) OSCL_LEAVE(OsclErrBadHandle);

    PVRTSPEngineNodeExtensionInterface* rtspExtIntf =
        (PVRTSPEngineNodeExtensionInterface*)
        (iSessionControllerNodeContainer->iExtensions[0]);

    return (rtspExtIntf->SetClientParameters(clientParams->_deviceInfo,
            clientParams->_userAgent,
            clientParams->_userNetwork));

};

OSCL_EXPORT_REF
PVMFStatus PVMFStreamingManagerExtensionInterfaceImpl::setTimeParams(PVMFSMTimeParams* timeParams)
{
    OSCL_UNUSED_ARG(timeParams);
    return PVMFFailure;
};

OSCL_EXPORT_REF
PVMFStatus PVMFStreamingManagerExtensionInterfaceImpl::setAuthenticationParameters(PVMFSMAuthenticationParmas* authenticationParams)
{
    PVMFSMNodeContainer* iSessionControllerNodeContainer =
        iContainer->getNodeContainer(PVMF_STREAMING_MANAGER_RTSP_SESSION_CONTROLLER_NODE);

    if (iSessionControllerNodeContainer == NULL) OSCL_LEAVE(OsclErrBadHandle);

    PVRTSPEngineNodeExtensionInterface* rtspExtIntf =
        (PVRTSPEngineNodeExtensionInterface*)
        (iSessionControllerNodeContainer->iExtensions[0]);

    return (rtspExtIntf->SetAuthenticationParameters(authenticationParams->_userID,
            authenticationParams->_authenticationInfo,
            authenticationParams->_expirationInfo,
            authenticationParams->_applicationSpecificString,
            authenticationParams->_verificationInfo,
            authenticationParams->_signatureInfo));
};

OSCL_EXPORT_REF
PVMFStatus PVMFStreamingManagerExtensionInterfaceImpl::setJitterBufferParams(PVMFSMJitterBufferParmas* jitterBufferParams)
{
    PVMFSMNodeContainer* iJitterBufferNodeContainer =
        iContainer->getNodeContainer(PVMF_STREAMING_MANAGER_JITTER_BUFFER_NODE);

    if (iJitterBufferNodeContainer == NULL) OSCL_LEAVE(OsclErrBadHandle);

    PVMFJitterBufferExtensionInterface* jbExtIntf =
        (PVMFJitterBufferExtensionInterface*)
        (iJitterBufferNodeContainer->iExtensions[0]);

    jbExtIntf->setJitterBufferDurationInMilliSeconds(jitterBufferParams->_bufferDurationInMilliSeconds);
    jbExtIntf->setPlayBackThresholdInMilliSeconds(jitterBufferParams->_playBackThresholdInMilliSeconds);

    iContainer->setJitterBufferDurationInMilliSeconds(jitterBufferParams->_bufferDurationInMilliSeconds);

    return PVMFSuccess;
}

OSCL_EXPORT_REF
PVMFStatus PVMFStreamingManagerExtensionInterfaceImpl::resetJitterBuffer()
{
    return PVMFFailure;
}

OSCL_EXPORT_REF PVMFStatus
PVMFStreamingManagerExtensionInterfaceImpl::setPayloadParserRegistry(PayloadParserRegistry* registry)
{
    PVMFSMNodeContainer* iMediaLayerNodeContainer =
        iContainer->getNodeContainer(PVMF_STREAMING_MANAGER_MEDIA_LAYER_NODE);

    if (iMediaLayerNodeContainer == NULL) OSCL_LEAVE(OsclErrBadHandle);

    PVMFMediaLayerNodeExtensionInterface* mlExtIntf =
        (PVMFMediaLayerNodeExtensionInterface*)
        (iMediaLayerNodeContainer->iExtensions[0]);

    return (mlExtIntf->setPayloadParserRegistry(registry));
}

OSCL_EXPORT_REF PVMFStatus
PVMFStreamingManagerExtensionInterfaceImpl::setPacketSourceInterface(PVMFPacketSource* aPacketSource)
{
    iPacketSource = aPacketSource;
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus
PVMFStreamingManagerExtensionInterfaceImpl::setDataPortLogging(bool logEnable,
        OSCL_String* logPath)
{
    PVMFSMNodeContainer* iMediaLayerNodeContainer =
        iContainer->getNodeContainer(PVMF_STREAMING_MANAGER_MEDIA_LAYER_NODE);

    if (iMediaLayerNodeContainer == NULL) OSCL_LEAVE(OsclErrBadHandle);

    PVMFMediaLayerNodeExtensionInterface* mlExtIntf =
        (PVMFMediaLayerNodeExtensionInterface*)
        (iMediaLayerNodeContainer->iExtensions[0]);

    return (mlExtIntf->setPortDataLogging(logEnable, logPath));
}

OSCL_EXPORT_REF PVMFStatus
PVMFStreamingManagerExtensionInterfaceImpl::switchStreams(uint32 aSrcStreamID, uint32 aDestStreamID)
{
    return (iContainer->switchStreams(aSrcStreamID, aDestStreamID));
}

OSCL_EXPORT_REF PVMFStatus
PVMFStreamingManagerExtensionInterfaceImpl::SetSourceInitializationData(OSCL_wString& aSourceURL,
        PVMFFormatType& aSourceFormat,
        OsclAny* aSourceData)
{
    return (iContainer->setSessionSourceInfo(aSourceURL,
            aSourceFormat,
            aSourceData));
}

OSCL_EXPORT_REF PVMFStatus
PVMFStreamingManagerExtensionInterfaceImpl::SetClientPlayBackClock(PVMFMediaClock* clientClock)
{
    PVMFSMNodeContainer* iJitterBufferNodeContainer =
        iContainer->getNodeContainer(PVMF_STREAMING_MANAGER_JITTER_BUFFER_NODE);

    if (iJitterBufferNodeContainer == NULL) OSCL_LEAVE(OsclErrBadHandle);

    PVMFJitterBufferExtensionInterface* jbExtIntf =
        (PVMFJitterBufferExtensionInterface*)
        (iJitterBufferNodeContainer->iExtensions[0]);

    jbExtIntf->setClientPlayBackClock(clientClock);

    PVMFSMNodeContainer* iMediaLayerNodeContainer =
        iContainer->getNodeContainer(PVMF_STREAMING_MANAGER_MEDIA_LAYER_NODE);

    if (iMediaLayerNodeContainer == NULL) OSCL_LEAVE(OsclErrBadHandle);

    PVMFMediaLayerNodeExtensionInterface* mlExtIntf =
        (PVMFMediaLayerNodeExtensionInterface*)
        (iMediaLayerNodeContainer->iExtensions[0]);

    mlExtIntf->setClientPlayBackClock(clientClock);

    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus
PVMFStreamingManagerExtensionInterfaceImpl::SetEstimatedServerClock(PVMFMediaClock* aClientClock)
{
    OSCL_UNUSED_ARG(aClientClock);
    return PVMFErrNotSupported;
}

OSCL_EXPORT_REF PVMFStatus
PVMFStreamingManagerExtensionInterfaceImpl::GetMediaPresentationInfo(PVMFMediaPresentationInfo& aInfo)
{
    return (iContainer->GetMediaPresentationInfo(aInfo));
}

OSCL_EXPORT_REF PVMFStatus
PVMFStreamingManagerExtensionInterfaceImpl::SelectTracks(PVMFMediaPresentationInfo& aInfo)
{
    return (iContainer->SelectTracks(aInfo, iSessionId));
}

OSCL_EXPORT_REF void PVMFStreamingManagerExtensionInterfaceImpl::addRef()
{
    PVInterfaceImpl<PVMFStreamingManagerNodeAllocator>::addRef();
}

OSCL_EXPORT_REF void PVMFStreamingManagerExtensionInterfaceImpl::removeRef()
{
    PVInterfaceImpl<PVMFStreamingManagerNodeAllocator>::removeRef();
}

OSCL_EXPORT_REF bool
PVMFStreamingManagerExtensionInterfaceImpl::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == PVMF_STREAMINGMANAGERNODE_EXTENSIONINTERFACE_UUID)
    {
        PVMFStreamingManagerExtensionInterface* interimPtr =
            OSCL_STATIC_CAST(PVMFStreamingManagerExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, interimPtr);
        return true;
    }
    else if (uuid == PVMF_TRACK_SELECTION_INTERFACE_UUID)
    {
        PVMFTrackSelectionExtensionInterface* interimPtr =
            OSCL_STATIC_CAST(PVMFTrackSelectionExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, interimPtr);
        return true;
    }
    else if (uuid == PVMF_DATA_SOURCE_INIT_INTERFACE_UUID)
    {
        PVMFDataSourceInitializationExtensionInterface* interimPtr =
            OSCL_STATIC_CAST(PVMFDataSourceInitializationExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, interimPtr);
        return true;
    }
    else if (uuid == PvmfDataSourcePlaybackControlUuid)
    {
        PvmfDataSourcePlaybackControlInterface* interimPtr =
            OSCL_STATIC_CAST(PvmfDataSourcePlaybackControlInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, interimPtr);
        return true;
    }
    else if (uuid == KPVMFMetadataExtensionUuid)
    {
        PVMFMetadataExtensionInterface* interimPtr =
            OSCL_STATIC_CAST(PVMFMetadataExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, interimPtr);
        return true;
    }
    else if (uuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        PvmiCapabilityAndConfig* interimPtr =
            OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, interimPtr);
        return true;
    }
    else if (uuid == PVMFCPMPluginLicenseInterfaceUuid)
    {
        PVMFCPMPluginLicenseInterface* interimPtr =
            OSCL_STATIC_CAST(PVMFCPMPluginLicenseInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, interimPtr);
        return true;
    }
    else
    {
        iface = NULL;
        return false;
    }
}

OSCL_EXPORT_REF PVMFCommandId
PVMFStreamingManagerExtensionInterfaceImpl::SetDataSourcePosition(PVMFSessionId aSessionId,
        PVMFTimestamp aTargetNPT,
        PVMFTimestamp& aActualNPT,
        PVMFTimestamp& aActualMediaDataTS,
        bool aSeekToSyncPoint,
        uint32 aStreamID,
        OsclAny* aContext)
{
    return (iContainer->SetDataSourcePosition(aSessionId,
            aTargetNPT,
            aActualNPT,
            aActualMediaDataTS,
            aSeekToSyncPoint,
            aStreamID,
            aContext));
}

OSCL_EXPORT_REF PVMFCommandId
PVMFStreamingManagerExtensionInterfaceImpl::SetDataSourcePosition(PVMFSessionId aSessionId,
        PVMFDataSourcePositionParams& aPVMFDataSourcePositionParams,
        OsclAny* aContext)
{
    return (iContainer->SetDataSourcePosition(aSessionId,
            aPVMFDataSourcePositionParams,
            aContext));
}

OSCL_EXPORT_REF PVMFCommandId
PVMFStreamingManagerExtensionInterfaceImpl::QueryDataSourcePosition(PVMFSessionId aSessionId,
        PVMFTimestamp aTargetNPT,
        PVMFTimestamp& aActualNPT,
        bool aSeekToSyncPoint,
        OsclAny* aContext)
{
    return (iContainer->QueryDataSourcePosition(aSessionId,
            aTargetNPT,
            aActualNPT,
            aSeekToSyncPoint,
            aContext));
}

OSCL_EXPORT_REF PVMFCommandId
PVMFStreamingManagerExtensionInterfaceImpl::QueryDataSourcePosition(PVMFSessionId aSessionId,
        PVMFTimestamp aTargetNPT,
        PVMFTimestamp& aSeekPointBeforeTargetNPT,
        PVMFTimestamp& aSeekPointAfterTargetNPT,
        OsclAny* aContext,
        bool aSeekToSyncPoint)
{
    OSCL_UNUSED_ARG(aSeekPointAfterTargetNPT);
    return (iContainer->QueryDataSourcePosition(aSessionId,
            aTargetNPT,
            aSeekPointBeforeTargetNPT,
            aSeekToSyncPoint,
            aContext));
}

OSCL_EXPORT_REF PVMFCommandId
PVMFStreamingManagerExtensionInterfaceImpl::SetDataSourceRate(PVMFSessionId aSessionId,
        int32 aRate,
        PVMFTimebase* aTimebase,
        OsclAny* aContext)
{

    return (iContainer->SetDataSourceRate(aSessionId,
                                          aRate,
                                          aTimebase,
                                          aContext));
}

OSCL_EXPORT_REF PVMFStatus
PVMFStreamingManagerExtensionInterfaceImpl::ComputeSkipTimeStamp(PVMFTimestamp aTargetNPT,
        PVMFTimestamp aActualNPT,
        PVMFTimestamp aActualMediaDataTS,
        PVMFTimestamp& aSkipTimeStamp,
        PVMFTimestamp& aStartNPT)
{
    return (iContainer->ComputeSkipTimeStamp(aTargetNPT,
            aActualNPT,
            aActualMediaDataTS,
            aSkipTimeStamp,
            aStartNPT));
}

OSCL_EXPORT_REF uint32
PVMFStreamingManagerExtensionInterfaceImpl::GetNumMetadataKeys(char* query_key)
{
    return (iContainer->GetNumMetadataKeys(query_key));
}

OSCL_EXPORT_REF uint32
PVMFStreamingManagerExtensionInterfaceImpl::GetNumMetadataValues(PVMFMetadataList& aKeyList)
{
    return (iContainer->GetNumMetadataValues(aKeyList));
}

OSCL_EXPORT_REF PVMFCommandId
PVMFStreamingManagerExtensionInterfaceImpl::GetNodeMetadataKeys(PVMFSessionId aSessionId,
        PVMFMetadataList& aKeyList,
        uint32 starting_index,
        int32 max_entries,
        char* query_key,
        const OsclAny* aContextData)
{
    return (iContainer->GetNodeMetadataKeys(aSessionId,
                                            aKeyList,
                                            starting_index,
                                            max_entries,
                                            query_key,
                                            aContextData));
}

OSCL_EXPORT_REF PVMFCommandId
PVMFStreamingManagerExtensionInterfaceImpl::GetNodeMetadataValues(PVMFSessionId aSessionId,
        PVMFMetadataList& aKeyList,
        Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
        uint32 starting_index,
        int32 max_entries,
        const OsclAny* aContextData)
{
    return (iContainer->GetNodeMetadataValues(aSessionId,
            aKeyList,
            aValueList,
            starting_index,
            max_entries,
            aContextData));
}

// From PVMFMetadataExtensionInterface
OSCL_EXPORT_REF PVMFStatus
PVMFStreamingManagerExtensionInterfaceImpl::ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList,
        uint32 aStart,
        uint32 aEnd)
{
    return iContainer->ReleaseNodeMetadataKeys(aKeyList, aStart, aEnd);
}

// From PVMFMetadataExtensionInterface
OSCL_EXPORT_REF PVMFStatus
PVMFStreamingManagerExtensionInterfaceImpl::ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
        uint32 start,
        uint32 end)
{
    return iContainer->ReleaseNodeMetadataValues(aValueList, start, end);
}


// From PVMFCPMPluginLicenseInterface
OSCL_EXPORT_REF PVMFCommandId
PVMFStreamingManagerExtensionInterfaceImpl::GetLicense(PVMFSessionId aSessionId,
        OSCL_wString& aContentName,
        OsclAny* aData,
        uint32 aDataSize,
        int32 aTimeoutMsec,
        OsclAny* aContextData)
{
    return iContainer->GetLicense(aSessionId,
                                  aContentName,
                                  aData,
                                  aDataSize,
                                  aTimeoutMsec,
                                  aContextData);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFStreamingManagerExtensionInterfaceImpl::GetLicense(PVMFSessionId aSessionId,
        OSCL_String&  aContentName,
        OsclAny* aData,
        uint32 aDataSize,
        int32 aTimeoutMsec,
        OsclAny* aContextData)
{
    return iContainer->GetLicense(aSessionId,
                                  aContentName,
                                  aData,
                                  aDataSize,
                                  aTimeoutMsec,
                                  aContextData);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFStreamingManagerExtensionInterfaceImpl::CancelGetLicense(PVMFSessionId aSessionId, PVMFCommandId aCmdId, OsclAny* aContextData)
{
    return iContainer->CancelGetLicense(aSessionId, aCmdId, aContextData);
}

PVMFStatus PVMFStreamingManagerExtensionInterfaceImpl::GetLicenseStatus(
    PVMFCPMLicenseStatus& aStatus)
{
    return iContainer->GetLicenseStatus(aStatus);
}
