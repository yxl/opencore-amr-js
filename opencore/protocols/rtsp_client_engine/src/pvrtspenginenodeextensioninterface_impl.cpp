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
#ifndef PVRTSP_ENGINE_NODE_EXTENSION_INTERFACE_IMPL_H_INCLUDED
#include "pvrtspenginenodeextensioninterface_impl.h"
#endif

///////////////////////////////////////////////////////////////////////////////
//Interface ctor and dtor
///////////////////////////////////////////////////////////////////////////////
PVRTSPEngineNodeExtensionInterfaceImpl::PVRTSPEngineNodeExtensionInterfaceImpl(PVRTSPEngineNode* aContainer): PVInterfaceImpl<PVRTSPEngineNodeAllocator>(KPVRTSPEngineNodeExtensionUuid), iContainer(aContainer)
{
}

PVRTSPEngineNodeExtensionInterfaceImpl::~PVRTSPEngineNodeExtensionInterfaceImpl()
{
}

///////////////////////////////////////////////////////////////////////////////
//Implemenatation of virtuals declared in class PVInterface
///////////////////////////////////////////////////////////////////////////////
void PVRTSPEngineNodeExtensionInterfaceImpl::addRef()
{
    PVInterfaceImpl<PVRTSPEngineNodeAllocator>::addRef();
}

void PVRTSPEngineNodeExtensionInterfaceImpl::removeRef()
{
    PVInterfaceImpl<PVRTSPEngineNodeAllocator>::removeRef();
}

bool PVRTSPEngineNodeExtensionInterfaceImpl::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == Uuid())
    {
        addRef();
        iface = this;
        return true;
    }
    else
    {
        iface = NULL;
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////
//Implemenatation of virtuals exposed by  PVRTSPEngineNodeExtensionInterface
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::SetStreamingType(PVRTSPStreamingType aType)
{
    return iContainer->SetStreamingType(aType);
}

//Either SetSessionURL() or  SetSDPInfo() must be called before Init()
OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::SetSessionURL(OSCL_wString& aURL)
{
    return iContainer->SetSessionURL(aURL);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::GetSDP(OsclRefCounterMemFrag& aSDPBuf)
{
    return iContainer->GetSDP(aSDPBuf);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::SetRtspProxy(OSCL_String& aRtspProxyName, uint32 aRtspProxyPort)
{
    return iContainer->SetRtspProxy(aRtspProxyName, aRtspProxyPort);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::GetRtspProxy(OSCL_String& aRtspProxyName, uint32& aRtspProxyPort)
{
    return iContainer->GetRtspProxy(aRtspProxyName, aRtspProxyPort);
}

OSCL_EXPORT_REF bool PVRTSPEngineNodeExtensionInterfaceImpl::IsRdtTransport()
{
    return iContainer->IsRdtTransport();
}

OSCL_EXPORT_REF void PVRTSPEngineNodeExtensionInterfaceImpl::SetPortRdtStreamId(PVMFPortInterface* pPort, int iRdtStreamId)
{
    iContainer->SetPortRdtStreamId(pPort, iRdtStreamId);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::SetSDPInfo(OsclSharedPtr<SDPInfo>& aSDPinfo, Oscl_Vector<StreamInfo, OsclMemAllocator> &aSelectedStream)
{
    return iContainer->SetSDPInfo(aSDPinfo, aSelectedStream);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::GetServerInfo(PVRTSPEngineNodeServerInfo& aServerInfo)
{
    return iContainer->GetServerInfo(aServerInfo);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::GetStreamInfo(Oscl_Vector<StreamInfo, OsclMemAllocator> &aSelectedStream)
{
    return iContainer->GetStreamInfo(aSelectedStream);
}

OSCL_EXPORT_REF void PVRTSPEngineNodeExtensionInterfaceImpl::SetRealChallengeCalculator(IRealChallengeGen* pChallengeCalc)
{
    iContainer->SetRealChallengeCalculator(pChallengeCalc);
}

OSCL_EXPORT_REF void PVRTSPEngineNodeExtensionInterfaceImpl::SetRdtParser(IPayloadParser* pRdtParser)
{
    iContainer->SetRdtParser(pRdtParser);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::SetRequestPlayRange(const RtspRangeType& aRange)
{
    return iContainer->SetRequestPlayRange(aRange);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::GetActualPlayRange(RtspRangeType& aRange)
{
    return iContainer->GetActualPlayRange(aRange);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::GetUserAgent(OSCL_wString& aUserAgent)
{
    return iContainer->GetUserAgent(aUserAgent);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::SetClientParameters(OSCL_wString& aUserAgent,
        OSCL_wString&  aUserNetwork,
        OSCL_wString&  aDeviceInfo)
{
    return iContainer->SetClientParameters(aUserAgent, aUserNetwork, aDeviceInfo);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::SetAuthenticationParameters(OSCL_wString& aUserID,
        OSCL_wString& aAuthentication,
        OSCL_wString& aExpiration,
        OSCL_wString& aApplicationSpecificString,
        OSCL_wString& aVerification,
        OSCL_wString& aSignature)
{
    return iContainer->SetAuthenticationParameters(aUserID, aAuthentication, aExpiration, aApplicationSpecificString, aVerification, aSignature);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::SetKeepAliveMethod_timeout(int32 aTimeout)
{
    return iContainer->SetKeepAliveMethod_timeout(aTimeout);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::SetKeepAliveMethod_use_SET_PARAMETER(bool aUseSetParameter)
{
    return iContainer->SetKeepAliveMethod_use_SET_PARAMETER(aUseSetParameter);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::SetKeepAliveMethod_keep_alive_in_play(bool aKeepAliveInPlay)
{
    return iContainer->SetKeepAliveMethod_keep_alive_in_play(aKeepAliveInPlay);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::GetKeepAliveMethod(int32 &aTimeout, bool &aUseSetParameter, bool &aKeepAliveInPlay)
{
    return iContainer->GetKeepAliveMethod(aTimeout, aUseSetParameter, aKeepAliveInPlay);
}



OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::GetRTSPTimeOut(int32 &aTimeout)
{
    return iContainer->GetRTSPTimeOut(aTimeout);
}

OSCL_EXPORT_REF PVMFStatus PVRTSPEngineNodeExtensionInterfaceImpl::SetRTSPTimeOut(int32 aTimeout)
{
    return iContainer->SetRTSPTimeOut(aTimeout);
}

OSCL_EXPORT_REF void PVRTSPEngineNodeExtensionInterfaceImpl::UpdateSessionCompletionStatus(bool aSessionCompleted)
{
    iContainer->UpdateSessionCompletionStatus(aSessionCompleted);
}
