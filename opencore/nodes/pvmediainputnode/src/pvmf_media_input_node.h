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
 * @file pvmf_media_input_node.h
 * @brief
 */

#ifndef PVMF_MEDIA_INPUT_NODE_H_INCLUDED
#define PVMF_MEDIA_INPUT_NODE_H_INCLUDED

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
#ifndef PVMF_MEDIA_INPUT_NODE_OUTPORT_H_INCLUDED
#include "pvmf_media_input_node_outport.h"
#endif
#ifndef PVMF_MEDIA_INPUT_NODE_EVENTS_H_INCLUDED
#include "pvmf_media_input_node_events.h"
#endif

/** Port tags.  For now engine must use these directly since
* port tag query is not yet implemented
*/
enum PvmfMediaInputNodePortTags
{
    PVMF_MEDIAIO_NODE_OUTPUT_PORT_TAG = 1
};


/**
 * Command queue for internal use in the node
 */

//command type enums.  This node has one extra async command.
enum PvmfMediaInputNodeCmdType
{
    PVMF_MEDIAIONODE_SKIPMEDIADATA = PVMF_GENERIC_NODE_COMMAND_LAST
    , PVMF_MEDIAIONODE_STARTMIO
    //startmio command is generated internally by the input ports.
};

//command class
class PvmfMediaInputNodeCmd: public PVMFGenericNodeCommand<OsclMemAllocator>
{
    public:

        //this holds an event code associated with the command status
        PVMFStatus iEventCode;

        //need to override base construct routine due to additional parameter.
        void BaseConstruct(PVMFSessionId s, int32 aCmd, const OsclAny* aContext)
        {
            PVMFGenericNodeCommand<OsclMemAllocator>::BaseConstruct(s, aCmd, aContext);
            iEventCode = PvmfMediaInputNodeErr_First;
        }
        //need to override base copy routine due to additional parameter
        void Copy(PvmfMediaInputNodeCmd& aCmd)
        {
            PVMFGenericNodeCommand<OsclMemAllocator>::Copy(aCmd);
            iEventCode = aCmd.iEventCode;
        }


};
//define a synonym for the base class.
//a typedef creates compiler warnings so use #define
#define PvmfMediaInputNodeCmdBase PVMFGenericNodeCommand<OsclMemAllocator>

//command queue type
typedef PVMFNodeCommandQueue<PvmfMediaInputNodeCmd, OsclMemAllocator> PvmfMediaInputNodeCmdQ;


/**
 * class PvmfMediaInputNode is a node wrapper around the io interface
 */
class PvmfMediaInputNode : public OsclActiveObject,
            public PVMFNodeInterface,
            public PvmiMIOObserver,
            public PvmiCapabilityAndConfig
{
    public:
        static PVMFNodeInterface* Create(PvmiMIOControl* aIOInterfacePtr);
        static void Release(PVMFNodeInterface*);

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

        // PvmiMIOObserver implementation
        OSCL_IMPORT_REF void RequestCompleted(const PVMFCmdResp& aResponse);
        OSCL_IMPORT_REF void ReportErrorEvent(PVMFEventType aEventType, PVInterface* aExtMsg = NULL);
        OSCL_IMPORT_REF void ReportInfoEvent(PVMFEventType aEventType, PVInterface* aExtMsg = NULL);

        friend class PvmfMediaInputNodeOutPort;

        // implemetation of PvmiCapabilityAndConfig class functions here

        void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver);

        PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
                                     PvmiKvp*& aParameters, int& num_parameter_elements,
                                     PvmiCapabilityContext aContext);
        PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
        void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                                  PvmiKvp* aParameters, int num_parameter_elements);
        void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                               int num_elements, PvmiKvp * & aRet_kvp);
        PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                         int num_elements, PvmiKvp*& aRet_kvp, OsclAny* context = NULL);
        uint32 getCapabilityMetric(PvmiMIOSession aSession);
        PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements);

        // function used in VerifyParametersSync n SetParametersSync of capability class
        PVMFStatus VerifyAndSetConfigParameter(PvmiKvp& aParameter, bool aSetParam);

        // function used in getParametersSync of capability class
        PVMFStatus GetConfigParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr);
    private:
        typedef enum
        {
            ENone
            , EQueryCapability
            , EInit
            , EStart
            , EPause
            , EStop
            , EReset
        } EMioRequest;

        PvmfMediaInputNode();
        ~PvmfMediaInputNode();
        void ConstructL(PvmiMIOControl* aIOInterfacePtr);

        //from OsclActiveObject
        void Run();

        //Command processing
        PVMFCommandId QueueCommandL(PvmfMediaInputNodeCmd&);
        void ProcessCommand();
        void CommandComplete(PvmfMediaInputNodeCmdQ& aCmdQ, PvmfMediaInputNodeCmd& aCmd, PVMFStatus aStatus, OsclAny*aEventData = NULL);

        //generic node Command handlers.
        PVMFStatus DoQueryUuid(PvmfMediaInputNodeCmd&);
        PVMFStatus DoQueryInterface(PvmfMediaInputNodeCmd&);
        PVMFStatus DoRequestPort(PvmfMediaInputNodeCmd&, OsclAny*&);
        PVMFStatus DoReleasePort(PvmfMediaInputNodeCmd&);
        PVMFStatus DoInit(PvmfMediaInputNodeCmd&);
        PVMFStatus DoPrepare(PvmfMediaInputNodeCmd&);
        PVMFStatus DoStart(PvmfMediaInputNodeCmd&);
        PVMFStatus DoStop(PvmfMediaInputNodeCmd&);
        PVMFStatus DoFlush(PvmfMediaInputNodeCmd&);
        bool IsFlushPending();
        void FlushComplete();
        PVMFStatus DoPause(PvmfMediaInputNodeCmd&);
        PVMFStatus DoReset(PvmfMediaInputNodeCmd&);
        PVMFStatus DoCancelAllCommands(PvmfMediaInputNodeCmd&);
        PVMFStatus DoCancelCommand(PvmfMediaInputNodeCmd&);

        // Media IO commands
        PVMFStatus SendMioRequest(PvmfMediaInputNodeCmd& aCmd, EMioRequest);
        PVMFStatus CancelMioRequest(PvmfMediaInputNodeCmd& aCmd);
        void MioConfigured();

        // Event reporting
        void ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, PVMFStatus aEventCode = PvmfMediaInputNodeErr_First);
        void ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, PVMFStatus aEventCode = PvmfMediaInputNodeErr_First);
        void SetState(TPVMFNodeInterfaceState);

        bool PortQueuesEmpty();
        void SendEndOfTrackCommand(const PvmiMediaXferHeader& data_header_info);

        // Node capability.
        PVMFNodeCapability iCapability;

        // COmmand queues
        PvmfMediaInputNodeCmdQ iInputCommands;
        PvmfMediaInputNodeCmdQ iCurrentCommand;
        PvmfMediaInputNodeCmdQ iCancelCommand;

        // Media IO control
        PvmiMIOControl* iMediaIOControl;
        PvmiMIOSession iMediaIOSession;
        PvmiCapabilityAndConfig* iMediaIOConfig;
        PVInterface* iMediaIOConfigPVI;
        enum
        {
            MIO_STATE_IDLE
            , MIO_STATE_INITIALIZED
            , MIO_STATE_STARTED
            , MIO_STATE_PAUSED
        } iMediaIOState;

        EMioRequest iMediaIORequest;
        PVMFCommandId iMediaIOCmdId;
        PVMFCommandId iMediaIOCancelCmdId;
        bool iMediaIOCancelPending;

        // Ports
        PVMFPortVector<PvmfMediaInputNodeOutPort, OsclMemAllocator> iOutPortVector;
        int32 iPortActivity;

        // Events
        PVUuid iEventUuid;

        // Extension reference counter
        uint32 iExtensionRefCount;

        // Counter for number of callbacks for skip media data completion
        uint32 iSkipMediaDataResponse;

        //logger
        PVLogger* iLogger;
#ifdef _TEST_AE_ERROR_HANDLING
        bool iErrorHandlingInitFailed;
        bool iErrorHandlingStartFailed;
        bool iErrorHandlingStopFailed;
        bool iErrorCancelMioRequest;
        bool iError_No_Memory;
        bool iError_Out_Queue_Busy;
        uint32 iErrorSendMioRequest;
        uint32 iErrorNodeCmd;
        uint32 iChunkCount;
        int32 iErrorTrackID;
        uint32 iTrackID;
        typedef struct timeStamp_keytpe
        {
            uint8 mode;
            uint32 duration;
            uint32 track_no;
        }TimeStamp_KSV;
        TimeStamp_KSV iErrorTimeStamp;
#endif
};

#endif // PVMF_MEDIA_INPUT_NODE_H_INCLUDED





