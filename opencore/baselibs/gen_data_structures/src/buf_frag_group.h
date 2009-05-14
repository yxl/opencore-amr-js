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
#ifndef BUF_FRAG_GROUP
#define BUF_FRAG_GROUP 1

#include "oscl_media_data.h"

//
// Function seekBufFragGroup() moves the current position forward (if positive) or backward (if negative) by
// "delta_in_bytes". It returns false if error is encountered,
// else return true and change idx, offset and ptr to the new position.
// If boundary is reached, boundaryReached is set to true.
//
bool seekBufFragGroup(BufferFragment * fragGrp,
                      const int numFrags,
                      int & idx,
                      int & offset,
                      uint8 * & ptr,
                      bool & boundaryReached,
                      const int delta_in_bytes,
                      const int num_frags_to_skip);


#endif
