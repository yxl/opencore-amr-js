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
#ifndef PVMF_OMX_ENC_DEFS_H_INCLUDED
#define PVMF_OMX_ENC_DEFS_H_INCLUDED

// Error and info messages
//
enum PVMFOMXEncNodeErrors
{
    // Enc node failed to initialize the encoder. Fatal error so the node needs to be reset.
    PVOMXENCNODE_ERROR_ENCODER_INIT_FAILED = PVMF_NODE_ERROR_EVENT_LAST
};

enum PVMFOMXEncNodeInfo
{
    // Encoding of a frame failed. Encoder node will continue on to encode the next frame
    PVOMXENCNODE_INFO_ENCODEFRAME_FAILED = PVMF_NODE_INFO_EVENT_LAST,
    // Input bitstream buffer overflowed (frame too large or couldn't find frame marker).
    PVOMXENCNODE_INFO_INPUTBITSTREAMBUFFER_OVERFLOW
};

#endif // PVMF_OMXENC_DEFS_H_INCLUDED


