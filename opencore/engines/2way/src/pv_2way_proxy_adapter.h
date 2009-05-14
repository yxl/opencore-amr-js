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

#ifndef PV_2WAY_PROXY_ADAPTER_H_INCLUDED
#define PV_2WAY_PROXY_ADAPTER_H_INCLUDED

#ifndef PV_INTERFACE_PROXY_H_INCLUDED
#include "pv_interface_proxy.h"
#endif

#ifndef PV_2WAY_INTERFACE_H_INCLUDED
#include "pv_2way_interface.h"
#endif

#ifndef PV_2WAY_INTERFACE_CMD_MESSAGES_H_INCLUDED
#include "pv_2way_interface_cmd_messages.h"
#endif

#ifndef PV_2WAY_ENGINE_H_INCLUDED
#include "pv_2way_engine.h"
#endif

#ifndef OSCL_SHARED_PTR_H_INCLUDED
#include "oscl_shared_ptr.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#include "pv_engine_observer.h"
#include "pv_engine_observer_message.h"

class PVLogger;
class PVMFNodeInterface;


class PVCmnCmdRespMsg : public CPVCmnInterfaceObserverMessage,
            public PVCmdResponse
{
    public:
        PVCmnCmdRespMsg() : CPVCmnInterfaceObserverMessage(0),
                PVCmdResponse(0, (OsclAny*)NULL, PVMFSuccess, NULL, 0)
        {};

        ~PVCmnCmdRespMsg() {};

        void Set(PVCommandId aId,
                 void *aContext,
                 PVMFStatus aStatus,
                 OsclAny* aEventData = NULL,
                 int32 aEventDataSize = 0);

        void SetId(PVCommandId aId)
        {
            iId = aId;
        }
        void SetContextData(void *aContext)
        {
            iContext = aContext;
        }
};

class PVCmnAsyncEventMsg : public CPVCmnInterfaceObserverMessage,
            public PVAsyncInformationalEvent
{
    public:
        PVCmnAsyncEventMsg() : CPVCmnInterfaceObserverMessage(1)
                , PVAsyncInformationalEvent(PVT_INDICATION_INCOMING_TRACK)
        {};

        ~PVCmnAsyncEventMsg()
        {
            if (iEventExtInterface)
            {
                iEventExtInterface->removeRef();
                iEventExtInterface = NULL;
            }
        };

        void Set(const PVAsyncInformationalEvent& aEvent, PVEventType aType,
                 PVExclusivePtr aPtr,
                 uint8 *aLocalBuffer,
                 uint32 aLocalBufferSize);
};

class PVCmnAsyncErrorEvent: public CPVCmnInterfaceObserverMessage,
            public PVAsyncErrorEvent
{
    public:
        PVCmnAsyncErrorEvent() : CPVCmnInterfaceObserverMessage(2)
                , PVAsyncErrorEvent(0)
        {};

        ~PVCmnAsyncErrorEvent() {};

        void Set(PVEventType aEventType,
                 PVExclusivePtr aEventData,
                 uint8* aLocalBuffer,
                 int32 aLocalBufferSize);

};

/**
 * CPV2WayProxyAdapter Class
 *
 * CPV2WayProxyAdapter is the interface to the pv2way SDK, which
 * allows initialization, control, and termination of a two-way terminal.
 * The application is expected to contain and maintain a pointer to the
 * CPV2WayInterface instance at all times that a call is active.
 * The CPV2WayFactory factory class is to be used to create and
 * delete instances of this class
 **/
class CPV2WayProxyAdapter :
            public CPV2WayInterface,
            public PVProxiedEngine,
            public PVCommandStatusObserver,
            public PVInformationalEventObserver,
            public PVErrorEventObserver,
            public PVProxiedInterfaceClient,
            public PVProxiedInterfaceServer
{
    public:
        static CPV2WayProxyAdapter* New(TPVTerminalType aTerminalType,
                                        PVCommandStatusObserver* aCmdStatusObserver,
                                        PVInformationalEventObserver *aInfoEventObserver,
                                        PVErrorEventObserver *aErrorEventObserver);
        ~CPV2WayProxyAdapter();

        // CPV2WayInterface virtuals
        PVCommandId GetSDKInfo(PVSDKInfo &aSDKInfo, OsclAny* aContextData = NULL);
        PVCommandId GetSDKModuleInfo(PVSDKModuleInfo &aSDKModuleInfo, OsclAny* aContextData = NULL);
        PVCommandId Init(PV2WayInitInfo& aInitInfo, OsclAny* aContextData = NULL);
        PVCommandId Reset(OsclAny* aContextData = NULL);
        PVCommandId AddDataSource(PVTrackId aChannelId, PVMFNodeInterface& aDataSource, OsclAny* aContextData = NULL);
        PVCommandId RemoveDataSource(PVMFNodeInterface& aDataSource, OsclAny* aContextData = NULL);
        PVCommandId AddDataSink(PVTrackId aChannelId, PVMFNodeInterface& aDataSink, OsclAny* aContextData = NULL);
        PVCommandId RemoveDataSink(PVMFNodeInterface& aDataSink, OsclAny* aContextData = NULL);
        PVCommandId Connect(const PV2WayConnectOptions& aOptions, PVMFNodeInterface* aCommServer = NULL, OsclAny* aContextData = NULL);
        PVCommandId Disconnect(OsclAny* aContextData = NULL);
        PVCommandId GetState(PV2WayState& aState, OsclAny* aContextData = NULL);
        PVCommandId SetLatencyQualityTradeoff(PVMFNodeInterface& aTrack, int32 aTradeoff, OsclAny* aContextData = NULL);
        PVCommandId Pause(PV2WayDirection aDirection, PVTrackId aTrackId, OsclAny* aContextData = NULL);
        PVCommandId Resume(PV2WayDirection aDirection, PVTrackId aTrackId, OsclAny* aContextData = NULL);
        PVCommandId SetLogAppender(const char* aTag, OsclSharedPtr<PVLoggerAppender>& aAppender, OsclAny* aContextData = NULL);
        PVCommandId RemoveLogAppender(const char* aTag, OsclSharedPtr<PVLoggerAppender>& aAppender, OsclAny* aContextData = NULL);
        PVCommandId SetLogLevel(const char* aTag, int32 aLevel, bool aSetSubtree = false, OsclAny* aContextData = NULL);
        PVCommandId GetLogLevel(const char* aTag, int32& aLogInfo, OsclAny* aContextData = NULL);
        //PVCommandId SendUserInput(CPVUserInput& user_input, OsclAny* aContextData = NULL);
        //PVCommandId GetCallStatistics(CPVCmn2WayStatistics& aStats, OsclAny* aContextData = NULL);
        PVCommandId QueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr, OsclAny* aContext = NULL);
        PVCommandId QueryUUID(const PvmfMimeString& aMimeType, Oscl_Vector<PVUuid, BasicAlloc>& aUuids,
                              bool aExactUuidsOnly = false, OsclAny* aContextData = NULL);
        PVCommandId CancelAllCommands(OsclAny* aContextData = NULL);


        OSCL_IMPORT_REF void CreateAppenders(PVLogger *aLogger = NULL)
        {
            OSCL_UNUSED_ARG(aLogger);
        };
        OSCL_IMPORT_REF void CreateTerminal(PVLogger *aLogger = NULL);
        OSCL_IMPORT_REF void DeleteTerminal(PVLogger *aLogger = NULL);
        OSCL_IMPORT_REF void CleanupMessage(CPVCmnInterfaceCmdMessage *cmdMsg, PVLogger *aLogger = NULL);
        OSCL_IMPORT_REF void CleanupNotification(CPVCmnInterfaceObserverMessage *obsMsg, PVLogger *aLogger = NULL);
        OSCL_IMPORT_REF void ProcessNotification(CPVCmnInterfaceObserverMessage *aMsg, PVLogger *aLogger = NULL);
        OSCL_IMPORT_REF void ProcessMessage(CPVCmnInterfaceCmdMessage *aMsg, PVLogger *aLogger = NULL);

        //from PVProxiedEngine
        OSCL_IMPORT_REF void CreateLoggerAppenders();
        OSCL_IMPORT_REF void PVThreadLogon(PVMainProxy &proxy);
        OSCL_IMPORT_REF void PVThreadLogoff(PVMainProxy &proxy);

        //from PVProxiedInterfaceServer
        OSCL_IMPORT_REF void HandleCommand(TPVProxyMsgId aMsgId, OsclAny* aMsg);
        OSCL_IMPORT_REF void CleanupNotification(TPVProxyMsgId aId, OsclAny* aMsg);

        //from PVProxiedInterfaceClient
        OSCL_IMPORT_REF void HandleNotification(TPVProxyMsgId aId, OsclAny* aMsg);
        OSCL_IMPORT_REF void CleanupCommand(TPVProxyMsgId aId, OsclAny* aMsg);

        //from MPVErrorEventObserver
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);

        //from MPVInformationalEventObserver
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent) ;

        //from MPVCommandStatusObserver
        void CommandCompleted(const PVCmdResponse& aResponse) ;

    private:
        CPV2WayProxyAdapter(): iCmdStatusObserver(NULL),
                iInfoEventObserver(NULL),
                iErrorEventObserver(NULL),
                iterminalType(PV_TERMINAL_TYPE_NONE),
                iterminalEngine(NULL),
                iLogger(NULL),
                iPVProxy(NULL)
        {};

        OsclAny ConstructL(TPVTerminalType aTerminalType,
                           PVCommandStatusObserver* aCmdStatusObserver,
                           PVInformationalEventObserver *aInfoEventObserver,
                           PVErrorEventObserver *aErrorEventObserver);

        PVCmnCmdRespMsg *GetCmdMsgL();
        void FreeCmdMsg(PVCmnCmdRespMsg *msg)
        {
            iFreeCmdMsg.push_back(msg);
        }

        PVCmnAsyncEventMsg *GetEventMsgL();
        void FreeEventMsg(PVCmnAsyncEventMsg *msg)
        {
            iFreeEventMsg.push_back(msg);
        }

        PVCmnAsyncErrorEvent *GetErrorMsgL();
        void FreeErrorMsg(PVCmnAsyncErrorEvent *msg)
        {
            iFreeErrorMsg.push_back(msg);
        }

        static int Construct(CPV2WayProxyAdapter*& aRet,
                             TPVTerminalType aTerminalType,
                             PVCommandStatusObserver* aCmdStatusObserver,
                             PVInformationalEventObserver *aInfoEventObserver,
                             PVErrorEventObserver *aErrorEventObserver);


        PVCommandStatusObserver *iCmdStatusObserver;
        PVInformationalEventObserver *iInfoEventObserver;
        PVErrorEventObserver *iErrorEventObserver;

        TPVTerminalType iterminalType;
        CPV2WayInterface * iterminalEngine;

        PVLogger *iLogger;

        int ProcessMessageLTry(CPVCmnInterfaceCmdMessage *aMsg);
        void ProcessMessageL(CPVCmnInterfaceCmdMessage *aMsg);
        void ProcessNotificationL(CPVCmnInterfaceObserverMessage *aMsg);

        friend class CPV2WayInterfaceProxyHandler;
        friend class CPV2WayInterfaceProxyNotifier;

        Oscl_Vector<PVCmnCmdRespMsg *, OsclMemAllocator> iFreeCmdMsg;
        PVCmnCmdRespMsg iCmdMsg[MAX_PENDING_2WAY_COMMANDS];

        Oscl_Vector<PVCmnAsyncEventMsg *, OsclMemAllocator> iFreeEventMsg;
        PVCmnAsyncEventMsg iEventMsg[MAX_PENDING_2WAY_EVENTS];

        Oscl_Vector<PVCmnAsyncErrorEvent *, OsclMemAllocator> iFreeErrorMsg;
        PVCmnAsyncErrorEvent iErrorMsg[MAX_PENDING_2WAY_ERRORS];

        CPVInterfaceProxy *iPVProxy;
        friend class CPV2WayFactory;
        TPVProxyId iProxyId;
};

#endif //


