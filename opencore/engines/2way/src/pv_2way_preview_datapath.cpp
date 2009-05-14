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
#if defined(PV_PLAY_FROM_FILE_SUPPORT)
#include "pv_2way_preview_datapath.h"

CPV2WayPreviewDatapath *CPV2WayPreviewDatapath::NewL(PVLogger *aLogger,
        TPV2WayMediaType aFormat,
        CPV324m2Way *a2Way)
{
    CPV2WayPreviewDatapath *self = OSCL_NEW(CPV2WayPreviewDatapath, (aLogger, aFormat, a2Way));
    OsclError::LeaveIfNull(self);

    if (self)
    {
        OSCL_TRAPSTACK_PUSH(self);
        self->ConstructL();
    }

    OSCL_TRAPSTACK_POP();
    return self;
}

void CPV2WayPreviewDatapath::CheckPause()
{
    uint32 i;
    int32 error;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckPause state %d\n", iState));

    for (i = 0; i < iNodeList.size(); i++)
    {
        if (!iNodeList[i].iCanNodePause) continue;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckPause node %d state %d\n", i, iNodeList[i].iNode.iNode->GetState()));
        switch (iNodeList[i].iNode.iNode->GetState())
        {
            case EPVMFNodeStarted:
                OSCL_TRY(error, i2Way->SendNodeCmdL(PV2WAY_NODE_CMD_PAUSE, &(iNodeList[i].iNode), this));
                OSCL_FIRST_CATCH_ANY(error,
                                     PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckPause unable to pause node\n"));
                                     DatapathError();
                                     return;);
                break;

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckPause transitional node state!\n"));
                break;
        }
    }

    for (i = 0; i < iNodeList.size(); i++)
    {
        //If possible pause node is not paused.
        if (iNodeList[i].iCanNodePause && (iNodeList[i].iNode.iNode->GetState() != EPVMFNodePaused))
        {
            return;
        }
    }

    //Disconnect ports
    for (i = 0; i < iPortPairList.size(); i++)
    {
        iPortPairList[i].Disconnect();
    }

    //If reached this point then the datapath is deemed paused, notify upper layer.
    SetState(EPaused);
    PauseComplete();
    return;
}

void CPV2WayPreviewDatapath::CheckResume()
{
    uint32 i;
    int32 error;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckResume state %d\n", iState));

    for (i = 0; i < iNodeList.size(); i++)
    {
        if (!iNodeList[i].iCanNodePause) continue;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckResume node %d state %d\n", i, iNodeList[i].iNode.iNode->GetState()));
        switch (iNodeList[i].iNode.iNode->GetState())
        {
            case EPVMFNodePaused:
                OSCL_TRY(error, i2Way->SendNodeCmdL(PV2WAY_NODE_CMD_START, &(iNodeList[i].iNode), this));
                OSCL_FIRST_CATCH_ANY(error,
                                     PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckResume unable to pause node\n"));
                                     DatapathError();
                                     return;);
                break;

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckResume transitional node state!\n"));
                break;
        }
    }

    for (i = 0; i < iNodeList.size(); i++)
    {
        //If possible pause node is not started.
        if (iNodeList[i].iCanNodePause && (iNodeList[i].iNode.iNode->GetState() != EPVMFNodeStarted))
        {
            return;
        }
    }

    //Reconnect ports
    for (i = 0; i < iPortPairList.size(); i++)
    {
        iPortPairList[i].Connect();
    }

    //If reached this point then the datapath is deemed resumed, notify upper layer.
    SetState(EOpened);
    ResumeComplete();
    return;
}

bool CPV2WayPreviewDatapath::SetCmd(TPV2WayCmdInfo *aCmdInfo)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayPreviewDatapath::SetCmd state %d cmd %x\n", iState, aCmdInfo));

    if (aCmdInfo)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayPreviewDatapath::SetCmd cmd type %d\n", aCmdInfo->type));
        switch (aCmdInfo->type)
        {
            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayPreviewDatapath::SetCmd invalid command\n"));
                break;
        }

    }
    // A null pointer indicates a close for some reason
    else
    {
        if (!CloseDatapath(aCmdInfo)) return false;
    }

    return true;
}

void CPV2WayPreviewDatapath::OpenComplete()
{
    CommandComplete(PVMFSuccess);
}

void CPV2WayPreviewDatapath::CloseComplete()
{
    //If parent was closing, then let it notify the app.
    if (!IsParentClosing())
    {
        //If a command was issued
        if (iCmdInfo)
        {
            //If remove command was issued
            if (iCmdInfo->type == PVT_COMMAND_REMOVE_PREVIEW_SINK)
            {
                CommandComplete(PVMFSuccess);
            }
            //Else other command failed
            else
            {
                CommandComplete(PVMFFailure);
            }
        }
        else
        {
            int32 error;
            TPV2WayEventInfo* aEvent = NULL;
            OSCL_TRY(error, aEvent = i2Way->GetEventInfoL());
            OSCL_FIRST_CATCH_ANY(error,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayPreviewDatapath::CloseComplete unable to notify app!\n"));
                                 return);

            aEvent->type = PVT_INDICATION_PREVIEW_ERROR;
            aEvent->localBuffer[2] = 0;
            switch (iFormat)
            {
                case PV2WayAudio:
                    aEvent->localBuffer[0] = PV_AUDIO;
                    break;
                case PV2WayVideo:
                    aEvent->localBuffer[0] = PV_VIDEO;
                    break;
                case PV2WayMuxData:
                default:
                    break;
            }

            aEvent->localBufferSize = 2;
            i2Way->Dispatch(aEvent);
        }
    }
}

void CPV2WayPreviewDatapath::DatapathError()
{
    SetCmd(NULL);
}

bool CPV2WayPreviewDatapath::ParentIsClosing()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayPreviewDatapath::ParentIsClosing path state %d\n", iState));
    return SetCmd(NULL);
}

#endif


