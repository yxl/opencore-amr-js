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
#if defined(PV_RECORD_TO_FILE_SUPPORT)
#include "pv_2way_rec_datapath.h"
#include "pv_2way_data_channel_datapath.h"
//#include "pvmp4ffcn_trackconfig.h"

CPV2WayRecDatapath *CPV2WayRecDatapath::NewL(PVLogger *aLogger,
        TPV2WayMediaType aFormat,
        CPV2WayDataChannelDatapath &aDatapath,
        CPV324m2Way *a2Way)
{
    CPV2WayRecDatapath *self = OSCL_NEW(CPV2WayRecDatapath, (aLogger, aFormat, aDatapath, a2Way));
    OsclError::LeaveIfNull(self);

    if (self)
    {
        OSCL_TRAPSTACK_PUSH(self);
        self->ConstructL();
    }

    OSCL_TRAPSTACK_POP();
    return self;
}

void CPV2WayRecDatapath::CheckPause()
{
    uint32 i;
    int32 error;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckPause state %d\n", iState));

    for (i = 0; i < iNodeList.size(); i++)
    {
        if (!iNodeList[i].iCanNodePause) continue;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckPause node %d state %d\n", i, iNodeList[i].iNode.node->GetState()));
        switch (iNodeList[i].iNode.node->GetState())
        {
            case EPVMFNodeStarted:
                OSCL_TRY(error, i2Way->SendNodeCmdL(PV2WAY_NODE_CMD_PAUSE, iNodeList[i].iNode.node, iNodeList[i].iNode.session, this));
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
        if (iNodeList[i].iCanNodePause && (iNodeList[i].iNode.node->GetState() != EPVMFNodePaused))
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

void CPV2WayRecDatapath::CheckResume()
{
    uint32 i;
    int32 error;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckResume state %d\n", iState));

    for (i = 0; i < iNodeList.size(); i++)
    {
        if (!iNodeList[i].iCanNodePause) continue;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckResume node %d state %d\n", i, iNodeList[i].iNode.node->GetState()));
        switch (iNodeList[i].iNode.node->GetState())
        {
            case EPVMFNodePaused:
                OSCL_TRY(error, i2Way->SendNodeCmdL(PV2WAY_NODE_CMD_START, iNodeList[i].iNode.node, iNodeList[i].iNode.session, this));
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
        if (iNodeList[i].iCanNodePause && (iNodeList[i].iNode.node->GetState() != EPVMFNodeStarted))
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

bool CPV2WayRecDatapath::SetCmd(TPV2WayCmdInfo *aCmdInfo)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayRecDatapath::SetCmd state %d cmd %x\n", iState, aCmdInfo));

    if (aCmdInfo)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayRecDatapath::SetCmd cmd type %d\n", aCmdInfo->type));
        switch (aCmdInfo->type)
        {
            case PVT_COMMAND_START_RECORD:
                if (iState == EOpening) return false;
                iCmdInfo = aCmdInfo;
                Open();
                break;

            case PVT_COMMAND_STOP_RECORD:
                if (!CloseDatapath(aCmdInfo)) return false;
                break;

            case PVT_COMMAND_PAUSE_RECORD:
                if (iState == EPausing) return false;
                iCmdInfo = aCmdInfo;
                Pause();
                break;

            case PVT_COMMAND_RESUME_RECORD:
                if (iState == EUnpausing) return false;
                iCmdInfo = aCmdInfo;
                Resume();
                break;

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayRecDatapath::SetCmd invalid command\n"));
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

void CPV2WayRecDatapath::OpenComplete()
{
    CommandComplete(PVMFSuccess);
}

void CPV2WayRecDatapath::CloseComplete()
{
    //If parent was closing, then let it notify the app.
    if (!IsParentClosing())
    {
        //If a command was issued
        if (iCmdInfo)
        {
            //If stop command was issued
            if (iCmdInfo->type == PVT_COMMAND_STOP_RECORD)
            {
                CommandComplete(PVMFSuccess);
            }
            //Else other command failed
            else
            {
                CommandComplete(PVMFFailure);
            }
        }
    }

    i2Way->CheckRecordFileState();
}

void CPV2WayRecDatapath::DatapathError()
{
    SetCmd(NULL);
}


bool CPV2WayRecDatapath::ParentIsClosing()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayRecDatapath::ParentIsClosing path state %d\n", iState));
    return SetCmd(NULL);
}
#endif

