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
#ifndef PV_OMXDEFS_H_INCLUDED
#define PV_OMXDEFS_H_INCLUDED

/** Maximum number of base_component component instances */
//#define MAX_SUPPORTED_COMPONENTS 10 		// e.g. 10 = PV -MP4, PV-H263, PV-AVC, PV-WMV, PV-AAC, PV-AMR, PV-MP3, ...
#define MAX_SUPPORTED_COMPONENTS 15 		// e.g. 10 = PV -MP4, PV-H263, PV-AVC, PV-WMV, PV-AAC, PV-AMR, PV-MP3, ...
#define MAX_INSTANTIATED_COMPONENTS 7

//#define INSERT_NAL_START_CODE
// while there is no documented number of max NALs per frame, this should be more than enough
// this used to keep track of the NAL sizes in full-frame, multiple NAL output buffers
#define MAX_NAL_PER_FRAME 100

// maximum length of component names
#define PV_OMX_MAX_COMPONENT_NAME_LENGTH 128

// this is the PV defined index used to access the PV_OMXComponentCapabilityFlags structure in
// PV omx components. Index is arbitrarily chosen (but falls in the range
// above 0xFF0000) as defined in the spec)
#define PV_OMX_COMPONENT_CAPABILITY_TYPE_INDEX 0xFF7A347

//Enable/disable this switch to build the workspace with or without proxy
// 1 - Enable,  0 - disable
// 1 - Multithreaded, 0 - AO
#define PROXY_INTERFACE 1

// The following is needed for dll linking of APIs
#define __OMX_EXPORTS

/**
 * Port Specific Macro's
 */
/** Defines the major version of the core */
#define SPECVERSIONMAJOR  1
/** Defines the minor version of the core */
#define SPECVERSIONMINOR  0
/** Defines the revision of the core */
#define SPECREVISION      0
/** Defines the step version of the core */
#define SPECSTEP          0


#define BUFFER_ALLOCATED (1 << 0)
/** This flag is applied to a buffer when it is assigned
*   from another port or by the IL client
*/
#define BUFFER_ASSIGNED (1 << 1)
/** This flag is applied to a buffer if its header has been allocated
*/

#define HEADER_ALLOCATED (1 << 2)

/** This flag is applied to a buffer just deallocated
*/
#define BUFFER_FREE 0

#define PORT_IS_BEING_FLUSHED(port)				(port->IsPortFlushed == OMX_FALSE)
#define PORT_IS_ENABLED(port)					(port->PortParam.bEnabled == OMX_TRUE)
#define PORT_IS_POPULATED(port)					(port->PortParam.bPopulated == OMX_TRUE)


/** this flag specifies that the message send is a command */
#define SENDCOMMAND_MSG_TYPE 1
/** this flag specifies that the message send is an error message */
#define ERROR_MSG_TYPE       2
/** this flag specifies that the message send is a warning message */
#define WARNING_MSG_TYPE     3


#endif
