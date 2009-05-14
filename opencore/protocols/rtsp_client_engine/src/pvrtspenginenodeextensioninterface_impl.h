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
#define PVRTSP_ENGINE_NODE_EXTENSION_INTERFACE_IMPL_H_INCLUDED

#ifndef PVRTSP_ENGINE_NODE_EXTENSION_INTERFACE_H_INCLUDED
#include "pvrtspenginenodeextensioninterface.h"
#endif

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

#ifndef PVRTSP_CLIENT_ENGINE_NODE_H
#include "pvrtsp_client_engine_node.h"
#endif

class PVRTSPEngineNodeExtensionInterfaceImpl : public PVInterfaceImpl<PVRTSPEngineNodeAllocator>,
            public PVRTSPEngineNodeExtensionInterface
{
    public:
        ~PVRTSPEngineNodeExtensionInterfaceImpl();
        PVRTSPEngineNodeExtensionInterfaceImpl(PVRTSPEngineNode* aContainer);

        //**********begin PVInterface
        OSCL_IMPORT_REF virtual void addRef();
        OSCL_IMPORT_REF virtual void removeRef();
        OSCL_IMPORT_REF virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface);
        //**********end PVInterface

        OSCL_IMPORT_REF virtual PVMFStatus SetStreamingType(PVRTSPStreamingType aType = PVRTSP_3GPP_UDP);

        //Either SetSessionURL() or  SetSDPInfo() must be called before Init()
        OSCL_IMPORT_REF virtual PVMFStatus SetSessionURL(OSCL_wString& aURL) ;
        OSCL_IMPORT_REF virtual PVMFStatus GetSDP(OsclRefCounterMemFrag& aSDPBuf) ;

        OSCL_IMPORT_REF virtual PVMFStatus SetRtspProxy(OSCL_String& aRtspProxyName, uint32 aRtspProxyPort) ;
        OSCL_IMPORT_REF virtual PVMFStatus GetRtspProxy(OSCL_String& aRtspProxyName, uint32& aRtspProxyPort) ;

        // to be called before init
        OSCL_IMPORT_REF virtual bool IsRdtTransport() ;

        OSCL_IMPORT_REF virtual void SetPortRdtStreamId(PVMFPortInterface* pPort,
                int iRdtStreamId) ;

        OSCL_IMPORT_REF virtual PVMFStatus SetSDPInfo(OsclSharedPtr<SDPInfo>& aSDPinfo, Oscl_Vector<StreamInfo, OsclMemAllocator> &aSelectedStream);
        OSCL_IMPORT_REF virtual PVMFStatus GetServerInfo(PVRTSPEngineNodeServerInfo& aServerInfo);
        OSCL_IMPORT_REF virtual PVMFStatus GetStreamInfo(Oscl_Vector<StreamInfo, OsclMemAllocator> &aSelectedStream);


        // API to pass in Real related parameters
        OSCL_IMPORT_REF virtual void SetRealChallengeCalculator(IRealChallengeGen* pChallengeCalc);
        OSCL_IMPORT_REF virtual void SetRdtParser(IPayloadParser* pRdtParser);

        //One of these must be called before Start()
        OSCL_IMPORT_REF virtual PVMFStatus SetRequestPlayRange(const RtspRangeType& aRange);
        OSCL_IMPORT_REF virtual PVMFStatus GetActualPlayRange(RtspRangeType& aRange);


        OSCL_IMPORT_REF virtual PVMFStatus GetUserAgent(OSCL_wString& aUserAgent) ;

        OSCL_IMPORT_REF virtual PVMFStatus SetClientParameters(OSCL_wString& aUserAgent,
                OSCL_wString&  aUserNetwork,
                OSCL_wString&  aDeviceInfo);

        OSCL_IMPORT_REF virtual PVMFStatus SetAuthenticationParameters(OSCL_wString& aUserID,
                OSCL_wString& aAuthentication,
                OSCL_wString& aExpiration,
                OSCL_wString& aApplicationSpecificString,
                OSCL_wString& aVerification,
                OSCL_wString& aSignature);
        //OSCL_IMPORT_REF virtual PVMFStatus SetTimeout(uint32 aTimeout);
        //OSCL_IMPORT_REF virtual PVMFStatus GetTimeout(uint32& aTimeout);

        /**
         * This API is to set the keep-alive mechanism for rtsp client.
         *
         * @param aTimeout The interval in milliseconds of sending the RTSP keep-alive commands.
         *	0 to use server defined timeout value.
         * @param aUseSetParameter true to use SET_PARAMETER; false to use OPTIONS
         * @param aKeepAliveInPlay Turns on/off the sending RTSP keep-alive commands during PLAY
         * @returns Completion status
         */
        //OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod(int32 aTimeout=0, bool aUseSetParameter=false, bool aKeepAliveInPlay=false);
        OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod_timeout(int32 aTimeout = 0);
        OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod_use_SET_PARAMETER(bool aUseSetParameter = false);
        OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod_keep_alive_in_play(bool aKeepAliveInPlay = false);

        OSCL_IMPORT_REF virtual PVMFStatus GetKeepAliveMethod(int32 &aTimeout, bool &aUseSetParameter, bool &aKeepAliveInPlay);



        OSCL_IMPORT_REF virtual PVMFStatus GetRTSPTimeOut(int32 &aTimeout);
        OSCL_IMPORT_REF virtual PVMFStatus SetRTSPTimeOut(int32 aTimeout);

        OSCL_IMPORT_REF virtual void UpdateSessionCompletionStatus(bool aSessionCompleted);
    private:
        PVRTSPEngineNode* iContainer;

};

#endif

