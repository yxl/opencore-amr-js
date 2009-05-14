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
#include "pvmf_protocol_engine_node.h"
#include "pvmf_protocol_engine_node_progressive_streaming.h"
#include "pvmf_protocol_engine_node_progressive_streaming_container_factory.h"


////////////////////////////////////////////////////////////////////////////////////
//////	PVMFProtocolEngineNodeProgressiveDownloadContainerFactory implementation
////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF ProtocolContainer* PVMFProtocolEngineNodeProgressiveStreamingContainerFactory::Create(PVMFProtocolEngineNode *aNode)
{
    return OSCL_NEW(ProgressiveStreamingContainer, (aNode));
}

OSCL_EXPORT_REF bool PVMFProtocolEngineNodeProgressiveStreamingContainerFactory::Delete(ProtocolContainer* aContainer)
{
    if (aContainer) OSCL_DELETE(aContainer);
    return true;
}


