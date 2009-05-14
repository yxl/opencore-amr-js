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
 * @file pv_comms_io_node.h
 * @brief
 */

#ifndef PV_COMMS_IO_NODE_H_INCLUDED
#define PV_COMMS_IO_NODE_H_INCLUDED

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
#ifndef PV_COMMS_IO_NODE_PORT_H_INCLUDED
#include "pv_comms_io_node_port.h"
#endif
#ifndef PV_COMMS_IO_NODE_EVENTS_H_INCLUDED
#include "pv_comms_io_node_events.h"
#endif

/** Port tags.  For now engine must use these directly since
* port tag query is not yet implemented
*/
enum PVCommsIONodePortTags
{
    PVMF_COMMSIO_NODE_OUTPUT_PORT_TAG = 0x01,
    PVMF_COMMSIO_NODE_INPUT_PORT_TAG = 0x02,
    PVMF_COMMSIO_NODE_IO_PORT_TAG = 0x03, //bitwise OR of previous two tags
};

const PVMFStatus PVMFInfoEndOfInputPortData = PVMFInfoLast + 1;
const PVMFStatus PVMFInfoEndOfOutputPortData = PVMFInfoLast + 2;

class MIOControlContextElement
{

    public:
        MIOControlContextElement()
        {
            iControl = NULL;
            iMediaSession = 0;
            iMediaIOConfig = NULL;
        };
        bool operator== (const MIOControlContextElement &context)
        {
            return (iControl == context.iControl);
        }
        MIOControlContextElement &operator= (const MIOControlContextElement &context)
        {
            Copy(context);
            return (*this);
        }
        bool isValid()
        {
            return iControl ? true : false;
        };
        bool hasConfig()
        {
            return iMediaIOConfig ? true : false;
        };
        void ThreadLogon()
        {
            if (iControl)
                iControl->ThreadLogon();
        };
        void ThreadLogoff()
        {
            if (iControl)
            {
                iControl->ThreadLogoff();
                iControl->disconnect(iMediaSession);
            }
        };

        PvmiMIOControl *iControl;
        PvmiMIOSession iMediaSession;
        PvmiCapabilityAndConfig *iMediaIOConfig;
    private:
        void Copy(const MIOControlContextElement &aSettings)
        {
            iControl = aSettings.iControl;
            iMediaSession = aSettings.iMediaSession;
            iMediaIOConfig = aSettings.iMediaIOConfig;
        }
};

class MIOControlContextSet
{
    public:
        MIOControlContextSet(): iMediaInputElement(NULL), iMediaOutputElement(NULL) {};
        bool isUni()
        {
            return (iMediaInputElement == NULL || iMediaOutputElement == NULL);
        };
        bool isIOProxy()
        {
            return (!isUni() && iMediaInputElement == iMediaOutputElement);
        };
        bool isEmpty()
        {
            return (iMediaInputElement == NULL && iMediaOutputElement == NULL);
        };
        MIOControlContextElement *iMediaInputElement;
        MIOControlContextElement *iMediaOutputElement;
};

/**
 * Command queue for internal use in the node
 */

//command type enums.  This node has one extra async command.
enum PVCommsIONodeCmdType
{
    PVMF_COMMSIONODE_SKIPMEDIADATA = PVMF_GENERIC_NODE_COMMAND_LAST
    , PVMF_COMMSIONODE_STARTMIO
    //startmio command is generated internally by the input ports.
};

//command class
class PVCommsIONodeCmd: public PVMFGenericNodeCommand<OsclMemAllocator>
{
    public:

        //this holds an event code associated with the command status
        PVMFStatus iEventCode;
        MIOControlContextElement *iControlContext;

        //need to override base construct routine due to additional parameter.
        void BaseConstruct(PVMFSessionId s, int32 aCmd, const OsclAny* aContext)
        {
            PVMFGenericNodeCommand<OsclMemAllocator>::BaseConstruct(s, aCmd, aContext);
            iEventCode = PVCommsIONodeErr_First;
            iControlContext = NULL;
        }
        //need to override base copy routine due to additional parameter
        void Copy(PVCommsIONodeCmd& aCmd)
        {
            PVMFGenericNodeCommand<OsclMemAllocator>::Copy(aCmd);
            iEventCode = aCmd.iEventCode;
            iControlContext = aCmd.iControlContext;
        }


};
//define a synonym for the base class.
//a typedef creates compiler warnings so use #define
#define PVCommsIONodeCmdBase PVMFGenericNodeCommand<OsclMemAllocator>

//command queue type
typedef PVMFNodeCommandQueue<PVCommsIONodeCmd, OsclMemAllocator> PVCommsIONodeCmdQ;


/**
 * class PVCommsIONode is a node wrapper around the io interface
 */
class PVCommsIONode : public OsclActiveObject,
            public PVMFNodeInterface,
            public PvmiMIOObserver
{
    public:
        static PVMFNodeInterface* Create(PvmiMIOControl* aMIOControl, bool logBitstream = false);
        static PVMFNodeInterface* Create(PvmiMIOControl* aMIOInputControl,
                                         PvmiMIOControl* aMIOOutputControl, bool logBitstream = false);
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

        friend class PVCommsIONodePort;

    private:
        typedef enum
        {
            ENone
            , EQueryCapability
            , EInit
            , EStart
            , EPause
            , EStop
        } EMioRequest;

        PVCommsIONode(bool iLogBitstream);
        ~PVCommsIONode();
        void ConstructL(PvmiMIOControl* aIOInterfacePtr);
        void ConstructL(PvmiMIOControl* aMediaInputPtr, PvmiMIOControl* aMediaOutputPtr);

        //from OsclActiveObject
        void Run();

        //Command processing
        PVMFCommandId QueueCommandL(PVCommsIONodeCmd&);
        void ProcessCommand();
        void CommandComplete(PVCommsIONodeCmdQ& aCmdQ, PVCommsIONodeCmd& aCmd, PVMFStatus aStatus, OsclAny*aEventData = NULL);

        //generic node Command handlers.
        PVMFStatus DoQueryUuid(PVCommsIONodeCmd&);
        PVMFStatus DoQueryInterface(PVCommsIONodeCmd&);
        PVMFStatus DoRequestPort(PVCommsIONodeCmd&, OsclAny*&);
        PVMFStatus DoReleasePort(PVCommsIONodeCmd&);
        PVMFStatus DoInit(PVCommsIONodeCmd&);
        PVMFStatus DoPrepare(PVCommsIONodeCmd&);
        PVMFStatus DoStart(PVCommsIONodeCmd&);
        PVMFStatus DoStop(PVCommsIONodeCmd&);
        PVMFStatus DoFlush(PVCommsIONodeCmd&);
        bool IsFlushPending();
        void FlushComplete();
        PVMFStatus DoPause(PVCommsIONodeCmd&);
        PVMFStatus DoReset(PVCommsIONodeCmd&);
        PVMFStatus DoCancelAllCommands(PVCommsIONodeCmd&);
        PVMFStatus DoCancelCommand(PVCommsIONodeCmd&);

        bool ReRunCommandForNextMIO(PVCommsIONodeCmd&);
        // Returns contexts according to the initialization
        // sequence
        bool GetNextContextInSequence(PVCommsIONodeCmd&);
        // Returns context associated with given port tag
        MIOControlContextSet ContextSetFromTag(int32 aTag);
        //PvmiCapabilityAndConfig *GetConfig(int32 aPortTag);
        bool CreateMediaTransfer(int32 aPortTag, PvmiMediaTransfer *&aInputTransfer, PvmiMediaTransfer *&aOutputTransfer);
        void DeleteMediaTransfer(int32 aPortTag, PvmiMediaTransfer *aInputTransfer, PvmiMediaTransfer *aOutputTransfer);
        bool MIOSupportsPortTag(int32 aTag, MIOControlContextSet &aSet);
        bool ValidatePortTagRequest(int32 aTag);


        // Media IO commands
        PVMFStatus SendMioRequest(PVCommsIONodeCmd& aCmd, EMioRequest);
        PVMFStatus CancelMioRequest(PVCommsIONodeCmd& aCmd);
        void MioConfigured(int32 aPortTag);

        // Event reporting
        void ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, PVMFStatus aEventCode = PVCommsIONodeErr_First);
        void ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, PVMFStatus aEventCode = PVCommsIONodeErr_First);
        void SetState(TPVMFNodeInterfaceState);

        bool PortQueuesEmpty();
        void Assert(bool condition);

        // Node capability.
        PVMFNodeCapability iCapability;

        // COmmand queues
        PVCommsIONodeCmdQ iInputCommands;
        PVCommsIONodeCmdQ iCurrentCommand;
        PVCommsIONodeCmdQ iCancelCommand;

        // Media IO control
        MIOControlContextElement iMediaInputContext;
        MIOControlContextElement iMediaOutputContext;

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
        PVMFPortVector<PVCommsIONodePort, OsclMemAllocator> iPortVector;
        int32 iPortActivity;

        // Events
        PVUuid iEventUuid;

        // Extension reference counter
        uint32 iExtensionRefCount;

        // Counter for number of callbacks for skip media data completion
        uint32 iSkipMediaDataResponse;

        //logger
        PVLogger* iLogger;
        bool iLogBitstream;
        // for removing type-punned warning
        PVInterface * iTempCapConfigInterface;
};

#endif // PV_COMMS_IO_NODE_H_INCLUDED





