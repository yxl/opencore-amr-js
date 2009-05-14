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
#include "tsc_statemanager.h"
#include "tsc_lc.h"
#include "tsc_blc.h"
#include "tsc_clc.h"
#include "tsc_constants.h"
#include "tsc_mt.h"
#include "tsc_component.h"
#include "tsc_capability.h"
#include "tsc_channelcontrol.h"



TSC_channelcontrol::TSC_channelcontrol(OlcList& aOlcs,
                                       TSC_statemanager& aTSCStateManager,
                                       TSC_blc& aTSCblc,
                                       TSC_mt& aTSCmt,
                                       TSC_lc& aTSClc,
                                       TSC_capability& aTSCcapability,
                                       TSC_clc& aTSCclc,
                                       TSC_component& aTSCcomponent) :
        iOlcs(aOlcs),
        iH223(NULL),
        iTSCstatemanager(aTSCStateManager),
        iTSCblc(aTSCblc),
        iTSCmt(aTSCmt),
        iTSClc(aTSClc),
        iTSCcapability(aTSCcapability),
        iTSCclc(aTSCclc),
        iTSCcomponent(aTSCcomponent)
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h245user");
};




LogicalChannelInfo* TSC_channelcontrol::GetLogicalChannelInfo(PVMFPortInterface& port)
{
    if (port.GetPortTag() == PV_MULTIPLEXED)
    {
        OSCL_LEAVE(PVMFErrArgument);
    }
    return OSCL_STATIC_CAST(LogicalChannelInfo*, &port);
}

TPVStatusCode TSC_channelcontrol::RequestIncomingChannelClose(TPVChannelId lcn)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_channelcontrol: Requesting incoming channel close (%d).\n", lcn));

    /* Initiate RequestChannelClose Command */
    iTSCclc.ClcClsReq(lcn);

    return EPVT_Pending;
}


bool TSC_channelcontrol::IsEstablishedLogicalChannel(TPVDirection aDir, TPVChannelId aChannelId)
{
    OlcParam* param = iOlcs.FindOlcGivenChannel(aDir, aChannelId);
    if (param)
    {
        if (param->GetState() == OLC_PENDING ||
                param->GetState() == OLC_ESTABLISHED)
        {
            return true;
        }
        return false;
    }

    OlcList::iterator it = iOlcs.begin();
    while (it != iOlcs.end())
    {
        OlcList::value_type& val = (*it++);
        OlcParam* olc = val.second;
        if (olc->GetDirection() == REVERSE_DIR(aDir) &&
                olc->GetReverseParams() &&
                olc->GetReverseParams()->GetChannelId() == aChannelId)
        {
            return true;
        }
    }
    return false;
}

void TSC_channelcontrol::ReceivedFormatSpecificInfo(TPVChannelId channel_id,
        uint8* fsi,
        uint32 fsi_len)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "TSC_channelcontrol::ReceivedFormatSpecificInfo lcn=%d, len=%d",
                     channel_id, fsi_len));
    OlcParam* param = iOlcs.FindOlcGivenChannel(OUTGOING, channel_id);
    if (param == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_channelcontrol::ReceivedFormatSpecificInfo - Failed to lookup channel"));
        return;
    }
    if (param->GetState() != OLC_PENDING)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_channelcontrol::ReceivedFormatSpecificInfo - Channel not in pending state.  Not sending OLC."));
        return;
    }
    PVMFStatus status = SetFormatSpecificInfo(param->GetForwardParams()->GetDataType(),
                        fsi, fsi_len);
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "TSC_channelcontrol::ReceivedFormatSpecificInfo - Failed to set FormatSpecificInfo, status=%d.",
                         status));
        return;
    }
    if (param->GetDirectionality() == EPVT_BI_DIRECTIONAL)
    {
        // Check FSI in reverse parameters.  Set it to fsi if its different from filler fsi
        uint8* rvs_fsi = NULL;
        unsigned rvs_fsi_len =
            ::GetFormatSpecificInfo(param->GetReverseParams()->GetDataType(),
                                    rvs_fsi);
        if (IsFillerFsi(rvs_fsi, rvs_fsi_len))
        {
            SetFormatSpecificInfo(param->GetReverseParams()->GetDataType(),
                                  fsi, fsi_len);
        }

        iTSCblc.BlcEtbReq(channel_id,
                          param->GetForwardParams()->GetDataType(),
                          param->GetForwardParams()->GetLcnParams(),
                          param->GetReverseParams()->GetDataType(),
                          param->GetReverseParams()->GetLcnParams());
    }
    else
    {
        iTSClc.LcEtbReq(channel_id,
                        param->GetForwardParams()->GetDataType(),
                        param->GetForwardParams()->GetLcnParams());
    }
}

