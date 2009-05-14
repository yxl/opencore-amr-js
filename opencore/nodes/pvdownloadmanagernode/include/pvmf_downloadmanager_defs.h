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
#ifndef PVMF_DOWNLOADMANAGER_DEFS_H_INCLUDED
#define PVMF_DOWNLOADMANAGER_DEFS_H_INCLUDED


///////////////////////////////////////////////
// Error Events
///////////////////////////////////////////////

//This node uses the extended events reported by the download and parser nodes.
//See pvmfftdmnode_events.h for download node events.
//See pvmf_mp4ffparser_events.h for parser node events.


///////////////////////////////////////////////
// Information Events
///////////////////////////////////////////////

// The download node reports the following PVMF Info events
// in addition to the node-specific events:
//
//  PVMFInfoBufferingStart: Download has started.
//
//  PVMFInfoBufferingStatus: Download progress report in percent complete.
//
//	PVMFInfoBufferingComplete: Download is complete.
//
//	PVMFInfoDataReady: OK to begin playback.
//
//  PVMFInfoEndOfData: End of track detected by parser node.
//


///////////////////////////////////////////////
// Port type tags
///////////////////////////////////////////////

//None.  Port requests must match the format used by the Mp4 Parser node.

#endif // PVMF_DOWNLOADMANAGER_DEFS_H_INCLUDED

