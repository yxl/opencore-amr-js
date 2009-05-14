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
#ifndef PVMP4FFCN_TUNABLES_H_INCLUDED
#define PVMP4FFCN_TUNABLES_H_INCLUDED

//Default NODE COMMAND vector reserve size
#define PVMF_MP4FFCN_COMMAND_VECTOR_RESERVE 10

//Starting value for command IDs
#define PVMF_MP4FFCN_COMMAND_ID_START 20000

// Default port vector reserve size
#define PVMF_MP4FFCN_PORT_VECTOR_RESERVE 1

#define PVMF_MP4FFCN_MAX_INPUT_PORT 3
#define PVMF_MP4FFCN_MAX_OUTPUT_PORT 0

// Port message queue settings
#define PVMF_MP4FFCN_PORT_CAPACITY 10
#define PVMF_MP4FFCN_PORT_RESERVE 10
#define PVMF_MP4FFCN_PORT_THRESHOLD 50

// Default video track config parameters
#define PVMF_MP4FFCN_VIDEO_BITRATE 52000
#define PVMF_MP4FFCN_VIDEO_FRAME_WIDTH 176
#define PVMF_MP4FFCN_VIDEO_FRAME_HEIGHT 144
#define PVMF_MP4FFCN_VIDEO_TIMESCALE 1000
#define PVMF_MP4FFCN_VIDEO_FRAME_RATE 15
#define PVMF_MP4FFCN_VIDEO_IFRAME_INTERVAL 10
#define PVMF_MP4FFCN_VIDEO_H263_PROFILE 0
#define PVMF_MP4FFCN_VIDEO_H263_LEVEL 40

// Default audio track config parameters
#define PVMF_MP4FFCN_AUDIO_BITRATE 12200
#define PVMF_MP4FFCN_AUDIO_TIMESCALE 8000
#define PVMF_MP4FFCN_AUDIO_NUM_CHANNELS 1

// Default text track config parameters
#define PVMF_MP4FFCN_TEXT_BITRATE 0
#define PVMF_MP4FFCN_TEXT_FRAME_WIDTH 176
#define PVMF_MP4FFCN_TEXT_FRAME_HEIGHT 177
#define PVMF_MP4FFCN_TEXT_TIMESCALE 90000

#endif /*PVMP4FFCN_TUNABLES_H_INCLUDED*/
