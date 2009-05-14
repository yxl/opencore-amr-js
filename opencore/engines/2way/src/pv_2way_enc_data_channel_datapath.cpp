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
#include "pv_2way_enc_data_channel_datapath.h"

CPV2WayEncDataChannelDatapath *CPV2WayEncDataChannelDatapath::NewL(PVLogger *aLogger,
        PVMFFormatType aFormat,
        CPV324m2Way *a2Way)
{
    CPV2WayEncDataChannelDatapath *self = OSCL_NEW(CPV2WayEncDataChannelDatapath, (aLogger, aFormat, a2Way));
    OsclError::LeaveIfNull(self);

    if (self)
    {
        OSCL_TRAPSTACK_PUSH(self);
        self->ConstructL();
    }

    OSCL_TRAPSTACK_POP();
    return self;
}

void CPV2WayEncDataChannelDatapath::TSCPortClosed()
{
    if (iState != EClosed)
    {
        iPortPairList.back().Disconnect();
        iPortPairList.back().iDestPort.SetPort(NULL);
        SetCmd(NULL);
    }
}

PVMFPortInterface *CPV2WayEncDataChannelDatapath::GetTSCPort()
{
    return iPortPairList.empty() ? NULL : iPortPairList.back().iDestPort.GetPort();
}


PVMFPortInterface *CPV2WayEncDataChannelDatapath::GetSrcPort()
{
    return iPortPairList.empty() ? NULL : iPortPairList.front().iSrcPort.GetPort();
}

void CPV2WayEncDataChannelDatapath::SetSourceInputPort(PVMFPortInterface *aPort)
{
    iSourceInputPort.SetPort(aPort);
}

void CPV2WayEncDataChannelDatapath::UseFilePlayPort(bool aUseFilePlayPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayEncDataChannelDatapath::CheckPathSpecificOpen state %d, format %s, use play port %d, file play port %x\n", iState, iFormat.getMIMEStrPtr(), aUseFilePlayPort, iFilePlayPort));
    if ((iState != EOpened) || (iFilePlayPort == NULL)) return;

    //If audio use mixing port (audio source node input port)
    if (iFormat.isVideo())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayEncDataChannelDatapath::CheckPathSpecificOpen audio source port status %d\n", iSourceInputPort.GetStatus()));
        if (iSourceInputPort.GetStatus() == EHasPort)
        {
            if (aUseFilePlayPort)
            {
                iSourceInputPort.GetPort()->Connect(iFilePlayPort);
                iFilePlayPort->Connect(iSourceInputPort.GetPort());
            }
            else
            {
                iFilePlayPort->Disconnect();
                iSourceInputPort.GetPort()->Disconnect();
            }
        }
    }
    //Else video, use splitter port
    else
    {
        if (aUseFilePlayPort)
        {
            iPortPairList.front().Disconnect();
            iPortPairList.front().iDestPort.GetPort()->Connect(iFilePlayPort);
            iFilePlayPort->Connect(iPortPairList.front().iDestPort.GetPort());
        }
        else
        {
            iFilePlayPort->Disconnect();
            iPortPairList.front().iDestPort.GetPort()->Disconnect();
            iPortPairList.front().Connect();
        }
    }
}

void CPV2WayEncDataChannelDatapath::OpenComplete()
{

    CommandComplete(PVMFSuccess);
}

bool CPV2WayEncDataChannelDatapath::CheckPathSpecificOpen()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayEncDataChannelDatapath::CheckPathSpecificOpen\n"));
    {
        return true;
    }
}

bool CPV2WayEncDataChannelDatapath::PathSpecificClose()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayEncDataChannelDatapath::PathSpecificClose file port %x, input src port %d\n", iFilePlayPort, iSourceInputPort.GetStatus()));

    if (iFilePlayPort)
    {
        iFilePlayPort->Disconnect();
    }

    switch (iSourceInputPort.GetStatus())
    {
        case EHasPort:
            iSourceInputPort.GetPort()->Disconnect();
            //i2Way->CheckAudioSourceMixingPort();
            return false;

        case ENoPort:
            //No mixing port, continue closing this datapath
            return true;

        default:
            break;
    }

    return false;
}

bool CPV2WayEncDataChannelDatapath::CheckPathSpecificStart()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayEncDataChannelDatapath::CheckPathSpecificStart engine state=%d\n", i2Way->iState));
    if (i2Way->iState == EConnecting || i2Way->iState == EConnected)
    {
        return true;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayEncDataChannelDatapath::CheckPathSpecificStart engine not connecting yet."));
    return false;
}
