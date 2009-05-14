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
#ifndef PV_PLAYER_DATAPATH_H_INCLUDED
#define PV_PLAYER_DATAPATH_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED
#include "pvmf_media_presentation_info.h"
#endif

class PVPlayerDatapathObserver
{
    public:
        virtual void HandlePlayerDatapathEvent(int32 aDatapathEvent, PVMFStatus aEventStatus, OsclAny* aContext = NULL, PVMFCmdResp* aCmdResp = NULL) = 0;
        virtual ~PVPlayerDatapathObserver() {}
};

enum PVPDPState
{
    PVPDP_IDLE,
    PVPDP_ERROR,
    PREPARE_INIT,
    PREPARE_REQPORT,
    PREPARE_PREPARE,
    PREPARE_CONNECT,
    PREPARED,
    START_START,
    STARTED,
    PAUSE_PAUSE,
    PAUSED,
    STOP_STOP,
    STOPPED,
    TEARDOWN_RELEASEPORT1,
    TEARDOWN_RELEASEPORT2,
    TEARDOWNED,
    RESET_RESET,
    RESETTED,
    PVPDP_CANCEL,
    PVPDP_CANCELLED
};

class PVLogger;

class PVPlayerDatapath : public OsclTimerObject,
            public PVMFNodeCmdStatusObserver,
            public PVMFNodeInfoEventObserver,
            public PVMFNodeErrorEventObserver
{
    public:
        PVPlayerDatapath();

        ~PVPlayerDatapath();

        void SetObserver(PVPlayerDatapathObserver& aDPObserver,
                         PVMFNodeErrorEventObserver& aErrorObserver,
                         PVMFNodeInfoEventObserver& aInfoObserver)
        {
            iObserver = &aDPObserver;
            iErrorObserver = &aErrorObserver;
            iInfoObserver = &aInfoObserver;
        }

        void SetSourceNode(PVMFNodeInterface* aSourceNode)
        {
            iSourceNode = aSourceNode;
        }

        PVMFNodeInterface* GetSourceNode()
        {
            return iSourceNode;
        }

        void SetDecNode(PVMFNodeInterface* aDecNode)
        {
            iDecNode = aDecNode;
        }

        PVMFNodeInterface* GetDecNode()
        {
            return iDecNode;
        }

        void SetSinkNode(PVMFNodeInterface* aSinkNode)
        {
            iSinkNode = aSinkNode;
        }

        PVMFNodeInterface* GetSinkNode()
        {
            return iSinkNode;
        }

        void SetSourceDecTrackInfo(PVMFTrackInfo& aTrackInfo)
        {
            iSourceTrackInfo = &aTrackInfo;
            iSourceDecFormatType = aTrackInfo.getTrackMimeType().get_str();
        }

        void SetDecSinkFormatType(PVMFFormatType& aFormatType)
        {
            iDecSinkFormatType = aFormatType;
            iDecSinkFormatString = aFormatType.getMIMEStrPtr();
        }

        void SetSourceSinkTrackInfo(PVMFTrackInfo& aTrackInfo)
        {
            iSourceTrackInfo = &aTrackInfo;
            iSourceSinkFormatType = aTrackInfo.getTrackMimeType().get_str();
        }

        PVMFStatus Prepare(OsclAny* aContext);

        PVMFStatus Start(OsclAny* aContext);

        PVMFStatus Pause(OsclAny* aContext, bool aSinkPaused = false);

        PVMFStatus Stop(OsclAny* aContext, bool aErrorCondition = false);

        PVMFStatus Teardown(OsclAny* aContext, bool aErrorCondition = false);

        PVMFStatus Reset(OsclAny* aContext, bool aErrorCondition = false);

        PVMFStatus CancelCommand(OsclAny* aContext);

        void DisconnectNodeSession(void);

        PVPDPState iState;

    private:
        // From OsclTimerObject
        void Run();

        // From PVMFNodeCmdStatusObserver
        void NodeCommandCompleted(const PVMFCmdResp& aResponse);

        // From PVMFNodeInfoEventObserver
        void HandleNodeInformationalEvent(const PVMFAsyncEvent& aEvent);

        // From PVMFNodeErrorEventObserver
        void HandleNodeErrorEvent(const PVMFAsyncEvent& aEvent);

        PVMFStatus IssueDatapathInit(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId& aCmdId);
        PVMFStatus IssueDatapathRequestPort(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, int32 aPortTag,
                                            PvmfMimeString* aPortConfig, OsclAny* aContext, PVMFCommandId &aCmdId);
        PVMFStatus IssueDatapathPrepare(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId& aCmdId);
        PVMFStatus IssueDatapathStart(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId& aCmdId);
        PVMFStatus IssueDatapathPause(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId& aCmdId);
        PVMFStatus IssueDatapathStop(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId& aCmdId);
        PVMFStatus IssueDatapathReleasePort(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFPortInterface* aPort, PVMFCommandId& aCmdId);
        PVMFStatus IssueDatapathReset(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId& aCmdId);
        PVMFStatus IssueDatapathCancel(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId& aCmdId);

        PVMFNodeInterface* iSourceNode;
        PVMFSessionId iSourceSessionId;
        PVMFNodeInterface* iDecNode;
        PVMFSessionId iDecSessionId;
        PVMFNodeInterface* iSinkNode;
        PVMFSessionId iSinkSessionId;

        PVMFPortInterface* iSourceOutPort;
        PVMFPortInterface* iDecInPort;
        PVMFPortInterface* iDecOutPort;
        PVMFPortInterface* iSinkInPort;

        PVPlayerDatapathObserver* iObserver;
        PVMFNodeErrorEventObserver* iErrorObserver;
        PVMFNodeInfoEventObserver* iInfoObserver;
        OsclAny* iContext;

        PVMFFormatType iSourceDecFormatType;
        PVMFFormatType iDecSinkFormatType;
        OSCL_HeapString<OsclMemAllocator> iDecSinkFormatString;
        PVMFFormatType iSourceSinkFormatType;
        PVMFTrackInfo* iSourceTrackInfo;

        // Enum for the datapath configuration
        enum PVPDPConfig
        {
            CONFIG_NONE,
            CONFIG_DEC
        };

        PVPDPConfig iDatapathConfig;

        int32 iPendingCmds;

        PVLogger* iLogger;

        bool iSinkPaused;

        bool iErrorCondition;
        bool iErrorOccurredDuringErrorCondition;
};

#endif // PV_PLAYER_DATAPATH_H_INCLUDED


