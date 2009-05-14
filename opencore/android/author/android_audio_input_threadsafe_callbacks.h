/* ------------------------------------------------------------------
 * Copyright (C) 2009 PacketVideo
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

#ifndef ANDROID_AUDIO_INPUT_THREADSAFE_CALLBACK_H_INCLUDED
#define ANDROID_AUDIO_INPUT_THREADSAFE_CALLBACK_H_INCLUDED

#ifndef THREADSAFE_CALLBACK_AO_H_INCLUDED
#include "threadsafe_callback_ao.h"
#endif

#ifndef THREADSAFE_MEMPOOL_H_INCLUDED
#include "threadsafe_mempool.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#define PROCESS_MULTIPLE_EVENTS_IN_CALLBACK 1

namespace android {

const char AudioInputCallbackAOName[] = "AndroidAudioInputTSCAO_Name";

class AndroidAudioInputThreadSafeCallbackAO : public ThreadSafeCallbackAO
{
public:
    //Constructor
    AndroidAudioInputThreadSafeCallbackAO(void* aObserver = NULL,
                                           uint32 aDepth=10,
                                           const char* aAOname = AudioInputCallbackAOName,
                                           int32 aPriority = OsclActiveObject::EPriorityNominal
                                          );
    OsclReturnCode ProcessEvent(OsclAny* aEventData);
    // When the following two have been implemented, make them virtual
#if PROCESS_MULTIPLE_EVENTS_IN_CALLBACK
    virtual void Run();
    virtual OsclAny* Dequeue(OsclReturnCode &status);   
#endif 
    virtual ~AndroidAudioInputThreadSafeCallbackAO();
    ThreadSafeMemPoolFixedChunkAllocator *iMemoryPool;
};

}; // namespace android

#endif //ANDROID_AUDIO_INPUT_THREADSAFE_CALLBACK_H_INCLUDED


