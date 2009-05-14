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
#ifndef PVMF_OMX_BASEDEC_DEFS_H_INCLUDED
#define PVMF_OMX_BASEDEC_DEFS_H_INCLUDED

// Error and info messages
//
enum PVMFOMXBaseDecNodeErrors
{
    // Base decoder node failed to initialize the base decoder. Fatal error so the node needs to be reset.
    PVOMXBASEDECNODE_ERROR_DECODER_INIT_FAILED = PVMF_NODE_ERROR_EVENT_LAST
};

enum PVMFOMXBaseDecNodeInfo
{
    // Decoding of a frame failed. Base decoder node will continue on to decode the next frame
    PVOMXBASEDECNODE_INFO_DECODEFRAME_FAILED = PVMF_NODE_INFO_EVENT_LAST,
    // Input bitstream buffer overflowed (frame too large or couldn't find frame marker). Data will be dropped until the next frame marker is found
    // then frame will be decoded.
    PVOMXBASEDECNODE_INFO_INPUTBITSTREAMBUFFER_OVERFLOW
};

#endif // PVMF_OMXBASEDEC_DEFS_H_INCLUDED


