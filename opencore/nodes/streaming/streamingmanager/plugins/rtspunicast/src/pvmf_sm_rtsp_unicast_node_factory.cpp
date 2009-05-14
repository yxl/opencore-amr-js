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
#ifndef OSCL_EXCEPTION_H_INCLUDED
#include "oscl_exception.h"
#endif
#ifndef PVMF_SM_RTSP_UNICAST_NODE_FACTORY_H_INCLUDED
#include "pvmf_sm_rtsp_unicast_node_factory.h"
#endif

#ifndef PVMF_SM_FSP_RTSP_UNICAST_H
#include "pvmf_sm_fsp_rtsp_unicast.h"
#endif

OSCL_EXPORT_REF PVMFSMFSPBaseNode*
PVMFSMRTSPUnicastNodeFactory::CreateSMRTSPUnicastNodeFactory(int32 aPriority)
{
    PVMFSMFSPBaseNode* smRtspUnicastNode = NULL;
    int32 err;
    OSCL_TRY(err,

             smRtspUnicastNode = PVMFSMRTSPUnicastNode::New(aPriority);
            );

    if (err != OsclErrNone)
    {
        OSCL_LEAVE(err);
    }

    return (smRtspUnicastNode);
}

OSCL_EXPORT_REF bool PVMFSMRTSPUnicastNodeFactory::DeleteSMRTSPUnicastNodeFactory(PVMFSMFSPBaseNode* aSMFSPNode)
{

    bool retval = false;

    if (aSMFSPNode)
    {
        OSCL_DELETE(aSMFSPNode);
        aSMFSPNode = NULL;
        retval = true;
    }

    return retval;
}
