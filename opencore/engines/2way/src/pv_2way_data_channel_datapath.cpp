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
#include "pv_2way_data_channel_datapath.h"
#include "pv_2way_rec_datapath.h"


bool CPV2WayDataChannelDatapath::SetCmd(TPV2WayCmdInfo *aCmdInfo)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDataChannelDatapath::SetCmd state %d cmd %x\n", iState, aCmdInfo));

    if (aCmdInfo)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDataChannelDatapath::SetCmd cmd type %d\n", aCmdInfo->type));
        switch (aCmdInfo->type)
        {
            case PVT_COMMAND_ADD_DATA_SOURCE:
            case PVT_COMMAND_ADD_DATA_SINK:
                if (iState == EOpening) return false;
                iCmdInfo = aCmdInfo;
                Open();
                break;

            case PVT_COMMAND_REMOVE_DATA_SOURCE:
            case PVT_COMMAND_REMOVE_DATA_SINK:
                if (!CloseDatapath(aCmdInfo)) return false;
                break;

            case PVT_COMMAND_PAUSE:
                if (iState == EPausing) return false;
                iCmdInfo = aCmdInfo;
                Pause();
                break;

            case PVT_COMMAND_RESUME:
                if (iState == EUnpausing) return false;
                iCmdInfo = aCmdInfo;
                Resume();
                break;

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDataChannelDatapath::SetCmd invalid command\n"));
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

void CPV2WayDataChannelDatapath::CloseComplete()
{
    //If parent was closing, then let it notify the app.
    if (!IsParentClosing())
    {
        if (iStateBeforeClose != EOpening)
        {
            //If close command issued
            if (iCmdInfo)
            {
                CommandComplete(PVMFSuccess);
            }
            //Else if remote closed channel
            else if ((i2Way->iState != EResetting) &&
                     (i2Way->iState != EDisconnecting))
            {
                int error;
                TPV2WayEventInfo* aEvent = NULL;
                OSCL_TRY(error, aEvent = i2Way->GetEventInfoL());
                OSCL_FIRST_CATCH_ANY(error,
                                     PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDataChannelDatapath::CloseComplete unable to notify app!\n"));
                                     return);

                aEvent->type = PVT_INDICATION_CLOSE_TRACK;
                if (iType == EEncodeDatapath)
                {
                    aEvent->localBuffer[0] = OUTGOING;
                }
                else
                {
                    aEvent->localBuffer[0] = INCOMING;
                }
                *((TPVChannelId*)(aEvent->localBuffer + 4)) = iChannelId;

                aEvent->localBufferSize = 8;
                i2Way->Dispatch(aEvent);
            }
            //Else remote disconnect
        }
        //Else open failed.
        else
        {
            CommandComplete(PVMFFailure);
        }
    }

    i2Way->CheckState();
}

bool CPV2WayDataChannelDatapath::ParentIsClosing()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDataChannelDatapath::ParentIsClosing path state %d\n", iState));
    return SetCmd(NULL);
}

void CPV2WayDataChannelDatapath::CommandComplete(PVMFStatus aStatus)
{
    iCmdInfo->status = aStatus;
    i2Way->Dispatch(iCmdInfo);
    iCmdInfo = NULL;
}




