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
/**
 * @file pvmi_io_interface_node.h
 * @brief
 */

#ifndef PV_MEDIA_OUTPUT_NODE_H_INCLUDED
#define PV_MEDIA_OUTPUT_NODE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif
#ifndef PVMF_PORT_INTERFACE_H_INCLUDED
#include "pvmf_port_interface.h"
#endif
#ifndef PVMI_MIO_CONTROL_H_INCLUDED
#include "pvmi_mio_control.h"
#endif
#ifndef PVMI_MEDIA_IO_OBSERVER_H_INCLUDED
#include "pvmi_media_io_observer.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#ifndef PVMF_NODES_SYNC_CONTROL_H_INCLUDED
#include "pvmf_nodes_sync_control.h"
#endif
#ifndef PV_MEDIA_OUTPUT_NODE_INPORT_H_INCLUDED
#include "pv_media_output_node_inport.h"
#endif
#ifndef PV_MEDIA_OUTPUT_NODE_EVENTS_H_INCLUDED
#include "pv_media_output_node_events.h"
#endif
#ifndef PVMI_MEDIA_IO_CLOCK_EXTENSION_H_INCLUDED
#include "pvmi_media_io_clock_extension.h"
#endif

/** Port tags.  For now engine must use these directly since
* port tag query is not yet implemented
*/
enum PVMediaOutputNodePortTags
{
    PVMF_MEDIAIO_NODE_INPUT_PORT_TAG = 0
};


/**
 * Command queue for internal use in the node
 */

//command type enums.  This node has one extra async command.
enum PVMediaOutputNodeCmdType
{
    PVMF_MEDIAOUTPUTNODE_SKIPMEDIADATA = PVMF_GENERIC_NODE_COMMAND_LAST
};

//command class
class PVMediaOutputNodeCmd: public PVMFGenericNodeCommand<OsclMemAllocator>
{
    public:
        //for SkipMediaData
        void Construct(PVMFSessionId s, int32 aCmd
                       , PVMFTimestamp aResumeTimestamp
                       , uint32 aStreamID
                       , bool aPlayBackPositionContinuous
                       , const OsclAny* aContext)
        {
            iSession = s;
            iCmd = aCmd;
            iContext = aContext;
            iParam1 = (OsclAny*)aResumeTimestamp;
            iParam2 = (OsclAny*)aPlayBackPositionContinuous;
            iParam3 = (OsclAny*)aStreamID;
            iEventCode = PVMFMoutNodeErr_First;
        }
        void Parse(PVMFTimestamp& aResumeTimestamp
                   , bool& aPlayBackPositionContinuous
                   , uint32& aStreamID)
        {
            aResumeTimestamp = (PVMFTimestamp)iParam1;
            aPlayBackPositionContinuous = (iParam2) ? true : false;
            aStreamID = (uint32)iParam3;
        }
        //this holds an event code associated with the command status
        PVMFStatus iEventCode;

        //need to override base construct routine due to additional parameter.
        void BaseConstruct(PVMFSessionId s, int32 aCmd, const OsclAny* aContext)
        {
            PVMFGenericNodeCommand<OsclMemAllocator>::BaseConstruct(s, aCmd, aContext);
            iEventCode = PVMFMoutNodeErr_First;
        }
        //need to override base copy routine due to additional parameter
        void Copy(PVMediaOutputNodeCmd& aCmd)
        {
            PVMFGenericNodeCommand<OsclMemAllocator>::Copy(aCmd);
            iEventCode = aCmd.iEventCode;
        }
        bool caninterrupt()
        {	//this routine identifies commands that can interrupt current command.
            return (iCmd == PVMF_GENERIC_NODE_CANCELALLCOMMANDS
                    || iCmd == PVMF_GENERIC_NODE_CANCELCOMMAND);
        }

};
//define a synonym for the base class.
//a typedef creates compiler warnings so use #define
#define PVMediaOutputNodeCmdBase PVMFGenericNodeCommand<OsclMemAllocator>

//command queue type
typedef PVMFNodeCommandQueue<PVMediaOutputNodeCmd, OsclMemAllocator> PVMediaOutputNodeCmdQ;


/**
 * class PVMediaOutputNode is a node wrapper around the io interface
 */
class PVMediaOutputNode : public OsclActiveObject,
            public PVMFNodeInterface,
            public PvmiMIOObserver,
            public PvmfNodesSyncControlInterface,
            public PvmiCapabilityAndConfig
{
    public:
        static PVMFNodeInterface* Create(PvmiMIOControl* aIOInterfacePtr);
        static void Release(PVMFNodeInterface*);

        ~PVMediaOutputNode();
        // PVMFNodeInterface implementation
        OSCL_IMPORT_REF PVMFStatus ThreadLogon();
        OSCL_IMPORT_REF PVMFStatus ThreadLogoff();
        OSCL_IMPORT_REF PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        OSCL_IMPORT_REF PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryUUID(PVMFSessionId aSession
                                                , const PvmfMimeString& aMimeType
                                                , Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids
                                                , bool aExactUuidsOnly = false
                                                                         , const OsclAny* aContext = NULL) ;
        OSCL_IMPORT_REF PVMFCommandId QueryInterface(PVMFSessionId aSession
                , const PVUuid& aUuid
                , PVInterface*& aInterfacePtr
                , const OsclAny* aContext = NULL) ;
        OSCL_IMPORT_REF PVMFCommandId RequestPort(PVMFSessionId aSession
                , int32 aPortTag
                , const PvmfMimeString* aPortConfig = NULL
                                                      , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId ReleasePort(PVMFSessionId aSession
                , PVMFPortInterface& aPort
                , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Init(PVMFSessionId aSession
                                           , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Prepare(PVMFSessionId aSession
                                              , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Start(PVMFSessionId aSession
                                            , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Stop(PVMFSessionId aSession
                                           , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Flush(PVMFSessionId aSession
                                            , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Pause(PVMFSessionId aSession
                                            , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Reset(PVMFSessionId aSession
                                            , const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelAllCommands(PVMFSessionId aSession
                , const OsclAny* aContextData = NULL) ;
        OSCL_IMPORT_REF PVMFCommandId CancelCommand(PVMFSessionId aSession
                , PVMFCommandId aCmdId
                , const OsclAny* aContextData = NULL) ;
        void HandlePortActivity(const PVMFPortActivity& aActivity)
        {
            OSCL_UNUSED_ARG(aActivity);
        }

        // Pure virtual from PvInterface
        OSCL_IMPORT_REF void addRef();
        OSCL_IMPORT_REF void removeRef();
        OSCL_IMPORT_REF bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        // Pure virtuals from PvmfNodesSyncControlInterface
        OSCL_IMPORT_REF PVMFStatus SetClock(PVMFMediaClock* aClock);
        OSCL_IMPORT_REF PVMFStatus ChangeClockRate(int32 aRate);
        OSCL_IMPORT_REF PVMFStatus SetMargins(int32 aEarlyMargin, int32 aLateMargin);
        OSCL_IMPORT_REF void ClockStarted(void);
        OSCL_IMPORT_REF void ClockStopped(void);
        OSCL_IMPORT_REF PVMFCommandId SkipMediaData(PVMFSessionId aSession,
                PVMFTimestamp aResumeTimestamp,
                uint32 aStreamID = 0,
                bool aPlayBackPositionContinuous = false,
                OsclAny* aContext = NULL);

        // PvmiMIOObserver implementation
        OSCL_IMPORT_REF void RequestCompleted(const PVMFCmdResp& aResponse);
        OSCL_IMPORT_REF void ReportErrorEvent(PVMFEventType aEventType, PVInterface* aExtMsg = NULL);
        OSCL_IMPORT_REF void ReportInfoEvent(PVMFEventType aEventType, PVInterface* aExtMsg = NULL);


        bool IsMioRequestPending()
        {
            return (iMediaIORequest != ENone);
        };

        // From PvmiCapabilityAndConfig
        // Implement pure virtuals from PvmiCapabilityAndConfig interface
        OSCL_IMPORT_REF virtual PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements);
        OSCL_IMPORT_REF virtual PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext);
        OSCL_IMPORT_REF virtual PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements);

        // Unsupported PvmiCapabilityAndConfig methods
        void virtual setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
        {
            OSCL_UNUSED_ARG(aObserver);
        };
        void virtual createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
        };
        void virtual setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext, PvmiKvp* aParameters, int aNumParamElements)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(aNumParamElements);
        };
        void virtual DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aContext);
        };
        void virtual setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(aNumElements);
            OSCL_UNUSED_ARG(aRetKVP);
        };
        PVMFCommandId virtual setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp*& aRetKVP, OsclAny* aContext = NULL)
        {
            OSCL_UNUSED_ARG(aSession);
            OSCL_UNUSED_ARG(aParameters);
            OSCL_UNUSED_ARG(aNumElements);
            OSCL_UNUSED_ARG(aRetKVP);
            OSCL_UNUSED_ARG(aContext);
            return -1;
        }
        uint32 virtual getCapabilityMetric(PvmiMIOSession aSession)
        {
            OSCL_UNUSED_ARG(aSession);
            return 0;
        };

        uint32 getClockRate()
        {
            return iClockRate;
        };

        void ReportBOS();

    private:
        friend class PVMediaOutputNodePort;
        friend class PVMediaOutputNodeFactory;

        PVMediaOutputNode();

        void ConstructL(PvmiMIOControl* aIOInterfacePtr);

        //from OsclActiveObject
        void Run();

        //Command processing
        PVMFCommandId QueueCommandL(PVMediaOutputNodeCmd&);
        void ProcessCommand();
        void CommandComplete(PVMediaOutputNodeCmdQ& aCmdQ, PVMediaOutputNodeCmd& aCmd, PVMFStatus aStatus, OsclAny*aEventData = NULL);
        PVMediaOutputNodeCmdQ iInputCommands;
        PVMediaOutputNodeCmdQ iCurrentCommand;
        PVMediaOutputNodeCmdQ iCancelCommand;

        //generic node Command handlers.
        PVMFStatus DoReset(PVMediaOutputNodeCmd&);
        PVMFStatus DoQueryUuid(PVMediaOutputNodeCmd&);
        PVMFStatus DoQueryInterface(PVMediaOutputNodeCmd&);
        PVMFStatus DoRequestPort(PVMediaOutputNodeCmd&, OsclAny*&);
        PVMFStatus DoReleasePort(PVMediaOutputNodeCmd&);
        PVMFStatus DoInit(PVMediaOutputNodeCmd&);
        PVMFStatus DoPrepare(PVMediaOutputNodeCmd&);
        PVMFStatus DoStart(PVMediaOutputNodeCmd&);
        PVMFStatus DoStop(PVMediaOutputNodeCmd&);
        PVMFStatus DoFlush(PVMediaOutputNodeCmd&);
        PVMFStatus DoPause(PVMediaOutputNodeCmd&);
        PVMFStatus DoCancelAllCommands(PVMediaOutputNodeCmd&);
        PVMFStatus DoCancelCommand(PVMediaOutputNodeCmd&);
        //extra command handlers.
        PVMFStatus DoSkipMediaData(PVMediaOutputNodeCmd&);
        void CompleteSkipMediaData();

        // Event reporting
        PVUuid iEventUuid;
        void ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, PVMFStatus aEventCode = PVMFMoutNodeErr_First);
        void ReportErrorEvent(PVMFAsyncEvent& aEvent)
        {
            PVMFNodeInterface::ReportErrorEvent(aEvent);
        }
        void ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, PVMFStatus aEventCode = PVMFMoutNodeErr_First);
        void ReportInfoEvent(PVMFAsyncEvent& aEvent)
        {
            PVMFNodeInterface::ReportInfoEvent(aEvent);
        }
        void SetState(TPVMFNodeInterfaceState);

        //node capability.
        PVMFNodeCapability iCapability;

        // Media IO control
        PvmiMIOControl* iMIOControl;
        PvmiMIOSession iMIOSession;
        PvmiCapabilityAndConfig* iMIOConfig;
        PVInterface* iMIOConfigPVI;
        enum EMioRequest
        {
            ENone
            , EQueryCapability
            , EQueryClockExtension
            , EInit
            , EStart
            , EPause
            , EStop
            , EDiscard
            , EReset
        } ;
        EMioRequest iMediaIORequest;
        enum MioStates
        {
            STATE_IDLE
            , STATE_LOGGED_ON
            , STATE_INITIALIZED
            , STATE_STARTED
            , STATE_PAUSED
        };
        MioStates iMediaIOState;
        PVMFCommandId iMediaIOCmdId;
        PVMFCommandId iMediaIOCancelCmdId;
        bool iMediaIOCancelPending;
        PVMFStatus SendMioRequest(PVMediaOutputNodeCmd& aCmd, EMioRequest);
        PVMFStatus CancelMioRequest(PVMediaOutputNodeCmd& aCmd);

        // Ports
        PVMFPortVector<PVMediaOutputNodePort, OsclMemAllocator> iInPortVector;
        bool PortQueuesEmpty();

        // Variables for media data queue and synchronization
        PVMFMediaClock* iClock;
        int32 iEarlyMargin;
        int32 iLateMargin;
        int32 iClockRate;
        PvmiClockExtensionInterface* iMIOClockExtension;
        PVInterface* iMIOClockExtensionPVI;

        /* Diagnostic log related */
        PVLogger* iDiagnosticsLogger;
        bool iDiagnosticsLogged;
        void LogDiagnostics();

        // Extension reference counter
        uint32 iExtensionRefCount;

        // Counter for number of callbacks for skip media data completion
        bool SkipMediaDataComplete();

        //logger
        PVLogger* iLogger;
        PVLogger* iReposLogger;

        OSCL_HeapString<OsclMemAllocator> iSinkFormatString;

        uint32 iRecentBOSStreamID;
        PVMFStatus CheckForBOS();
};


#endif // PVMI_IO_INTERFACE_NODE_H_INCLUDED





