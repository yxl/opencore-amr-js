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

#include "pvmf_node_interface.h"


OSCL_EXPORT_REF void PVMFNodeInterface::SetState(TPVMFNodeInterfaceState s)
{
    iInterfaceState = s;
    ReportInfoEvent(PVMFInfoStateChanged, (OsclAny*)s);
}

OSCL_EXPORT_REF void PVMFNodeInterface::ReportCmdCompleteEvent(PVMFSessionId s, PVMFCmdResp &resp)
{
    for (uint32 i = 0;i < iSessions.size();i++)
    {
        if (iSessions[i].iId == s)
        {
            if (iSessions[i].iInfo.iCmdStatusObserver)
                iSessions[i].iInfo.iCmdStatusObserver->NodeCommandCompleted(resp);
            break;
        }
    }
}

OSCL_EXPORT_REF void PVMFNodeInterface::ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData, PVInterface*aExtMsg)
{
    for (uint32 i = 0;i < iSessions.size();i++)
    {
        PVMFAsyncEvent resp(PVMFErrorEvent
                            , aEventType
                            , iSessions[i].iInfo.iErrorContext
                            , aExtMsg
                            , aEventData);
        if (iSessions[i].iInfo.iErrorObserver)
            iSessions[i].iInfo.iErrorObserver->HandleNodeErrorEvent(resp);
    }
}

OSCL_EXPORT_REF void PVMFNodeInterface::ReportErrorEvent(PVMFAsyncEvent &aEvent)
{
    for (uint32 i = 0;i < iSessions.size();i++)
    {
        PVMFAsyncEvent resp(PVMFErrorEvent
                            , aEvent.GetEventType()
                            , iSessions[i].iInfo.iErrorContext
                            , aEvent.GetEventExtensionInterface()
                            , aEvent.GetEventData()
                            , aEvent.GetLocalBuffer()
                            , aEvent.GetLocalBufferSize());
        if (iSessions[i].iInfo.iErrorObserver)
            iSessions[i].iInfo.iErrorObserver->HandleNodeErrorEvent(resp);
    }
}

OSCL_EXPORT_REF void PVMFNodeInterface::ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData, PVInterface*aExtMsg)
{
    for (uint32 i = 0;i < iSessions.size();i++)
    {
        PVMFAsyncEvent resp(PVMFInfoEvent
                            , aEventType
                            , iSessions[i].iInfo.iInfoContext
                            , aExtMsg
                            , aEventData);
        if (iSessions[i].iInfo.iInfoObserver)
            iSessions[i].iInfo.iInfoObserver->HandleNodeInformationalEvent(resp);
    }
}

OSCL_EXPORT_REF void PVMFNodeInterface::ReportInfoEvent(PVMFAsyncEvent &aEvent)
{
    for (uint32 i = 0;i < iSessions.size();i++)
    {
        PVMFAsyncEvent resp(PVMFInfoEvent
                            , aEvent.GetEventType()
                            , iSessions[i].iInfo.iInfoContext
                            , aEvent.GetEventExtensionInterface()
                            , aEvent.GetEventData()
                            , aEvent.GetLocalBuffer()
                            , aEvent.GetLocalBufferSize());
        if (iSessions[i].iInfo.iInfoObserver)
            iSessions[i].iInfo.iInfoObserver->HandleNodeInformationalEvent(resp);
    }
}
