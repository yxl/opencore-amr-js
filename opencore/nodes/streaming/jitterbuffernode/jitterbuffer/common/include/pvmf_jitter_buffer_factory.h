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
#ifndef PVMF_JITTER_BUFFER_FACTORY_H
#define PVMF_JITTER_BUFFER_FACTORY_H

#ifndef OSCL_BASE_MACROS_H_INCLUDED
#include "oscl_base_macros.h"
#endif

class PVMFJitterBuffer;
class PVMFJitterBufferConstructParams;

class JitterBufferFactory
{
    public:
        virtual PVMFJitterBuffer* Create(const PVMFJitterBufferConstructParams& aJBCreationData) = 0;

        virtual void Destroy(PVMFJitterBuffer*& aJitterBuffer)  = 0;
};

#endif
