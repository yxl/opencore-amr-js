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
#ifndef PVMF_SM_NODE_FACTORY_H_INCLUDED
#include "pvmf_sm_node_factory.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMF_STREAMING_MANAGER_NODE_H_INCLUDED
#include "pvmf_streaming_manager_node.h"
#endif


OSCL_EXPORT_REF PVMFNodeInterface*
PVMFStreamingManagerNodeFactory::CreateStreamingManagerNode(int32 aPriority)
{
    PVMFNodeInterface* streamingManagerNode = NULL;

    int32 err;

    OSCL_TRY(err,
             /*
              * Create Streaming Manager Node
              */
             streamingManagerNode = PVMFStreamingManagerNode::New(aPriority);
            );

    if (err != OsclErrNone)
    {
        OSCL_LEAVE(err);
    }
    return (streamingManagerNode);
}

OSCL_EXPORT_REF bool PVMFStreamingManagerNodeFactory::DeleteStreamingManagerNode(PVMFNodeInterface* aNode)
{
    bool retval = false;

    if (aNode)
    {
        OSCL_DELETE(aNode);
        retval = true;
        aNode = NULL;
    }

    /*if(EPVMFNodeIdle == aNode->GetState())
    {
    	if(aNode)
    	{
    		OSCL_DELETE(aNode);
    		retval = true;
    	}
    }*/
    return retval;
}
