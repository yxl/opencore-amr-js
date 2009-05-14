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
#ifndef WAV_PARSERNODE_TUNABLES_H_INCLUDED
#define WAV_PARSERNODE_TUNABLES_H_INCLUDED

// Playback clock timescale
#define COMMON_PLAYBACK_CLOCK_TIMESCALE 1000

//Default NODE COMMAND vector reserve size
#define PVMF_WAVFFPARSER_NODE_COMMAND_VECTOR_RESERVE 10

//Starting value for command IDs
#define PVMF_WAVFFPARSER_NODE_COMMAND_ID_START 6000

#define PVWAVFF_MEDIADATA_POOLNUM 10
#define PVWAVFF_MEDIADATA_CHUNKSIZE 128

#define PVWAV_MSEC_PER_BUFFER  100

#define MAX_QUERY_KEY_LEN       256

#define MIN_WAVFFPARSER_RATE 10000
#define MAX_WAVFFPARSER_RATE 500000

#endif /*WAV_PARSERNODE_TUNABLES_H_INCLUDED*/
