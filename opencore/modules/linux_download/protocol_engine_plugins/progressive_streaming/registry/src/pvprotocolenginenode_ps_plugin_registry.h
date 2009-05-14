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
#ifndef PVMF_PROTOCOL_ENGINE_NODE_PS_PLUGIN_H_REGISTRY_INCLUDED
#define PVMF_PROTOCOL_ENGINE_NODE_PS_PLUGIN_H_REGISTRY_INCLUDED

class ProtocolContainer;
class PVMFProtocolEngineNode;

class ProtocolEngineNodeProgressiveStreamingContainerLoader
{
    public:
        static ProtocolContainer* CreateProgressiveStreamingContainer(PVMFProtocolEngineNode* aNode);
        static bool DeleteProgressiveStreamingContainer(ProtocolContainer* aContainer);
};

#endif // PVMF_PROTOCOL_ENGINE_NODE_PS_PLUGIN_H_REGISTRY_INCLUDED

