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
#ifndef PV_PLAYER_ENGINE_TUNABLES_H_INCLUDED
#define PV_PLAYER_ENGINE_TUNABLES_H_INCLUDED


// Always start from the requested begin position when repositioning
#define PVPLAYERENGINE_CONFIG_SKIPTOREQUESTEDPOS_DEF true

// Synchronization margin for media tracks in milliseconds
// Default
#define PVPLAYERENGINE_CONFIG_SYNCMARGIN_EARLY_DEF -200
#define PVPLAYERENGINE_CONFIG_SYNCMARGIN_LATE_DEF 200

// Platforms that need the video decoder node priority to be lower than the
// rest of the nodes need to set the following to 1
#define VIDEO_DEC_NODE_LOW_PRIORITY 1

#endif // PV_PLAYER_ENGINE_TUNABLES_H_INCLUDED

