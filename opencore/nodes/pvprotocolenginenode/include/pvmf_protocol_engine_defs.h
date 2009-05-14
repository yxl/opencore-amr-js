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
#ifndef PVMF_PROTOCOLENGINE_DEFS_H_INCLUDED
#define PVMF_PROTOCOLENGINE_DEFS_H_INCLUDED


///////////////////////////////////////////////
// Port tags
///////////////////////////////////////////////

/** Enumerated list of port tags supported by the node,
** for the port requests.
*/
typedef enum
{
    PVMF_PROTOCOLENGINENODE_PORT_TYPE_INPUT,
    PVMF_PROTOCOLENGINENODE_PORT_TYPE_OUTPUT,
    PVMF_PROTOCOLENGINENODE_PORT_TYPE_FEEDBACK
} PVMFProtocolEngineNodePortType;


// Capability mime strings
#define PVMF_PROTOCOLENGINE_PORT_INPUT_FORMATS "x-pvmf/port/formattype"
#define PVMF_PROTOCOLENGINE_PORT_INPUT_FORMATS_VALTYPE "x-pvmf/port/formattype;valtype=int32"

#endif

