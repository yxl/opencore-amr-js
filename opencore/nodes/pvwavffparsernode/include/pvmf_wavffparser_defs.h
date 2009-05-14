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

#ifndef PVMF_WAVFFPARSER_DEFS_H_INCLUDED
#define PVMF_WAVFFPARSER_DEFS_H_INCLUDED

///////////////////////////////////////////////
// Port tags
///////////////////////////////////////////////

/** Enumerated list of port tags supported by the node,
** for the port requests.
*/
typedef enum
{
    PVMF_WAVFFPARSER_NODE_PORT_TYPE_SOURCE
} PVMFWAVFFParserOutPortType;

// Capability mime strings
#define PVMF_WAVFFPARSER_PORT_OUTPUT_FORMATS "x-pvmf/port/formattype"
#define PVMF_WAVFFPARSER_PORT_OUTPUT_FORMATS_VALTYPE "x-pvmf/port/formattype;valtype=int32"


///////////////////////////////////////////////
// Error Events
///////////////////////////////////////////////
enum TPFMVWAVFFNodeErrors
{
    PVMF_WAVFFNODE_ERROR_PORT_PROCESSING_ERROR = PVMF_NODE_ERROR_EVENT_LAST
    //This event is issued when an error occurs
    //during port data processing.
    //Event data contains the port pointer.
};


///////////////////////////////////////////////
// Information Events
///////////////////////////////////////////////

enum TPFMVWAVFFNodeInfo
{
    PVMF_WAVFFNODE_INFO_ENDOFTRACK = PVMF_NODE_INFO_EVENT_LAST
    //This event is issued whenever a port is deleted.
    //Observers should discard the port pointer.
    //Event data contains the port pointer.
};

#endif

