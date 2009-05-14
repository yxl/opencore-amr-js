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
///////////////////////////////////////////////////////////////////////////////
//
// pvmf_streaming_real_interfaces.h
//
// Defines abstract interfaces that will be used by streaming nodes to talk to
// modules providing real media or real data transport functionality.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef PVMF_STREAMING_REAL_INTERFACES_INCLUDED
#define PVMF_STREAMING_REAL_INTERFACES_INCLUDED

#include "oscl_mem.h"
#include "oscl_string_containers.h"


class IRealChallengeGen
{
    public:
        IRealChallengeGen() {};
        virtual ~IRealChallengeGen() {};

        virtual bool GetRealChallenge2(OSCL_HeapString<OsclMemAllocator> rc1,
                                       OSCL_HeapString<OsclMemAllocator> &rc2,
                                       OSCL_HeapString<OsclMemAllocator> &sd) = 0;
};

#endif // PVMF_STREAMING_REAL_INTERFACES_INCLUDED


