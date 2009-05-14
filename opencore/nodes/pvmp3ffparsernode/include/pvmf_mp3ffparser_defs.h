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

#ifndef PVMF_MP3FFPARSER_DEFS_H_INCLUDED
#define PVMF_MP3FFPARSER_DEFS_H_INCLUDED


/**
 *  Port tags, enumerated list of port tags supported by the node,
 *  for the port requests.
 **/
typedef enum
{
    PVMF_MP3FFPARSER_NODE_PORT_TYPE_SOURCE

} PVMFMP3FFParserNodePortType;

// Error Events
enum TPFMVMP3FFParserNodeErrors
{
    // This event is issued when an error occurs during port data processing.
    // Event data contains the port pointer.
    PVMF_MP3FFPARSER_NODE_ERROR_PORT_PROCESSING_ERROR = PVMF_NODE_ERROR_EVENT_LAST
};

// Information Events
enum TPVMFMP3FFNodeInfo
{
    // This event is issued whenever a port is deleted. Observers should discard
    // the port pointer. Event data contains the port pointer.
    PVMF_MP3FFNODE_INFO_ENDOFTRACK = PVMF_NODE_INFO_EVENT_LAST
};

// Capability mime strings
#define PVMF_MP3FFPARSER_OUTPORT_FORMATS "x-pvmf/port/formattype"
#define PVMF_MP3FFPARSER_OUTPORT_FORMATS_VALTYPE "x-pvmf/port/formattype;valtype=int32"

//Mimetypes for the custom interface
#define PVMF_MP3FFPARSER_NODE_EXTENSION_INTERFACE_MIMETYPE "pvxxx/MP3FFParserNode/ExtensionInterface"
#define PVMF_MP3FFPARSER_NODE_MIMETYPE "pvxxx/MP3FFParserNode"
#define PVMF_BASEMIMETYPE "pvxxx"

static const char PVMF_MP3_PARSER_NODE_ALL_METADATA_KEY[] = "all";

#endif // end PVMF_MP3FFPARSER_DEFS_H_INCLUDED

