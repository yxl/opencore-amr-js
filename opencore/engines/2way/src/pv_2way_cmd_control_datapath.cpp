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
#include "pv_2way_cmd_control_datapath.h"


bool CPV2WayCmdControlDatapath::CloseDatapath(TPV2WayCmdInfo *aCmdInfo)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayCmdControlDatapath::CloseDatapath state %d cmd %x\n", iState, aCmdInfo));
    if (iState == EClosing)
    {
        if (iCmdInfo == NULL) iCmdInfo = aCmdInfo;
        return false;
    }
    else
    {
        //If command was already in progress when close was initiated.
        if (iCmdInfo)
        {
            if ((aCmdInfo == NULL) && (iState == EOpening))
            {
                Close();
                return false;
            }

            CommandComplete(PVMFErrCancelled);
        }

        iCmdInfo = aCmdInfo;
        Close();

        return true;
    }
}

void CPV2WayCmdControlDatapath::CommandComplete(PVMFStatus aStatus)
{
    iCmdInfo->status = aStatus;
    i2Way->Dispatch(iCmdInfo);
    iCmdInfo = NULL;
}









