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
#ifndef PVMF_FILEINPUT_NODE_EVENTS_H_INCLUDED
#define PVMF_FILEINPUT_NODE_EVENTS_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

///////////////////////////////////////////////
// Error Events
///////////////////////////////////////////////

enum PVMFFileInputNodeErrorEvent
{
    PVMF_FILEINPUT_NODE_ERROR_PORT_PROCESSING_ERROR = PVMF_NODE_ERROR_EVENT_LAST
    , PVMF_FILEINPUT_NODE_ERROR_DATA_GENERATING_ERROR
};

///////////////////////////////////////////////
// Information Events
///////////////////////////////////////////////

// Enumerated list of informational event from PVMFFileInputNode
enum PVMFFileInputNodeInfoEvent
{
    PVMFFIN_EndOfFile = PVMF_NODE_INFO_EVENT_LAST
};

#endif


