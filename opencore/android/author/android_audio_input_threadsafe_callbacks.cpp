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

#include "android_audio_input_threadsafe_callbacks.h"
#include "android_audio_input.h"

namespace android {

AndroidAudioInputThreadSafeCallbackAO::AndroidAudioInputThreadSafeCallbackAO(void* aObserver,
                                                                               uint32 aDepth, 
                                                                               const char* aAOname,
                                                                               int32 aPriority)
:ThreadSafeCallbackAO(aObserver, aDepth, aAOname, aPriority)
{

}


AndroidAudioInputThreadSafeCallbackAO::~AndroidAudioInputThreadSafeCallbackAO()
{

}


OsclReturnCode AndroidAudioInputThreadSafeCallbackAO::ProcessEvent(OsclAny* EventData)
{
    // In this case, ProcessEvent calls the method of the primary test AO to process the Event
    if (iObserver != NULL)
    {
        AndroidAudioInput* ptr = (AndroidAudioInput*) iObserver;
        // Call RunIfNotReady() for the AudioMIO
        ptr->RunIfNotReady();
    }
    return OsclSuccess;
}

#if PROCESS_MULTIPLE_EVENTS_IN_CALLBACK
void AndroidAudioInputThreadSafeCallbackAO::Run()
{
    OsclAny *param;
    OsclReturnCode status = OsclSuccess;
    // Process multiple events in one attempt 
    do
    {
        param = Dequeue(status);
        if((status == OsclSuccess) || (status == OsclPending))
        {
            ProcessEvent(param);
        }
    }while(status == OsclSuccess);
}

OsclAny* AndroidAudioInputThreadSafeCallbackAO::Dequeue(OsclReturnCode &status)
{
    OsclAny *param;
    OsclProcStatus::eOsclProcError sema_status;

    status = OsclSuccess;

    Mutex.Lock();
    if(Q->NumElem == 0)
    {
        status = OsclFailure;
        Mutex.Unlock();
        return NULL;
    }
    
    param = (Q->pFirst[Q->index_out]).pData;
    Q->index_out++;
    if(Q->index_out == Q->MaxNumElements)
        Q->index_out = 0;
    Q->NumElem--;
    if(Q->NumElem == 0)
    {
        // Call PendForExec() only when there are no more elements in the queue
        // Else, continue in the do-while loop
        PendForExec();
        status = OsclPending;
    }
    Mutex.Unlock();

    sema_status = RemoteThreadCtrlSema.Signal();
    if(sema_status != OsclProcStatus::SUCCESS_ERROR)
    {
        status = OsclFailure;
        return NULL;
    }
    return param;
}
#endif

}; // namespace android

