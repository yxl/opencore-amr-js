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
#include "pv_2way_dec_data_channel_datapath.h"

CPV2WayDecDataChannelDatapath *CPV2WayDecDataChannelDatapath::NewL(PVLogger *aLogger,
        PVMFFormatType aFormat,
        CPV324m2Way *a2Way)
{
    CPV2WayDecDataChannelDatapath *self = OSCL_NEW(CPV2WayDecDataChannelDatapath, (aLogger, aFormat, a2Way));
    OsclError::LeaveIfNull(self);

    if (self)
    {
        OSCL_TRAPSTACK_PUSH(self);
        self->ConstructL();
    }

    OSCL_TRAPSTACK_POP();
    return self;
}

void CPV2WayDecDataChannelDatapath::TSCPortClosed()
{
    if (iState != EClosed)
    {
        iPortPairList.front().Disconnect();
        iPortPairList.front().iSrcPort.SetPort(NULL);
        SetCmd(NULL);
    }
}

PVMFPortInterface *CPV2WayDecDataChannelDatapath::GetTSCPort()
{
    return iPortPairList.empty() ? NULL : iPortPairList.front().iSrcPort.GetPort();
}

void CPV2WayDecDataChannelDatapath::OpenComplete()
{
    CommandComplete(PVMFSuccess);
}

void CPV2WayDecDataChannelDatapath::SkipComplete(TPV2WayNode* aNode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDecDataChannelDatapath::SkipComplete, Format=%s, Node=%x\n", iFormat.getMIMEStrPtr(), aNode->iNode));

    if (!IsNodeInDatapath(aNode->iNode))
        return;
    // Check if it is the sink node.  The sink node is the last node in the vector
    if (iNodeList[iNodeList.size()-1].iNode.iNode == aNode->iNode)
    {
        iSkipComplete = true;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDecDataChannelDatapath::SkipComplete, Format=%s, Node=%x\n Skip complete for datapath", iFormat.getMIMEStrPtr(), aNode->iNode));
    }
}

bool CPV2WayDecDataChannelDatapath::IsSkipComplete() const
{
    return iSkipComplete;
}
