/* thread_init.cpp
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include "playerdriver.h"
#include <media/thread_init.h>
//#include <android_runtime/AndroidRuntime.h>
#include <utils/threads.h>
#include "android_log_appender.h"
#include "pvlogger_time_and_id_layout.h"

#include "oscl_mem.h"
#include "oscl_error.h"

#include "OMX_Core.h"

#if (PVLOGGER_INST_LEVEL > 0)
#include "android_logger_config.h"
#endif

using namespace android;
bool gotkey=false;
pthread_key_t ptkey=NULL;

void keydestructor(void*)
{
    // This thread is about to exit, so we can un-initialize
    // PV for this thread.
    UninitializeForThread();
}

template<class DestructClass>
class LogAppenderDestructDealloc : public OsclDestructDealloc 
{
public:
    virtual void destruct_and_dealloc(OsclAny *ptr) 
    { 
        delete((DestructClass*)ptr); 
    }
};


bool InitializeForThread()
{
    // TODO: fix the race condition here that surfaces when
    // two threads call the initializer at pretty much the
    // same time. This does not happen in practice, but we
    // should guard against it anyway.

    if (!gotkey)
    {
        if (0 != pthread_key_create(&ptkey, keydestructor)) {
            LOGE("Out of TLS keys");
            return false;
        }
        gotkey = true;
    }

    if (NULL == pthread_getspecific(ptkey)) {
        // PV hasn't yet been initialized for this thread;
        int error = OsclBase::Init();
        if(error)
        {
            LOGE("OsclBase::Init error %d", error);
            return false;
        }
        error = OsclErrorTrap::Init();
        if(error)
        {
            LOGE("OsclErrorTrap::Init error %d", error);
            return false;
        }
        OsclMem::Init();
        PVLogger::Init();

        void *data = &ptkey;
        error = pthread_setspecific(ptkey,data);
        if(error)
        {
            LOGE("pthread_setspecific error %d", error);
            return false;
        }
#if (PVLOGGER_INST_LEVEL > 0)
        PVLoggerConfigFile obj;
        if(obj.IsLoggerConfigFilePresent())
        {
            obj.SetLoggerSettings();
        }
#endif
    }
    return true;
}


void UninitializeForThread() {
    PVLogger::Cleanup();
    OsclMem::Cleanup();
    OsclErrorTrap::Cleanup();
    OsclBase::Cleanup();
    // In case this didn't get called from keydestructor(), set the key
    // to NULL for this thread, which prevents the keydestructor() from
    // running once the thread actually exits.
    void *data = NULL;
    pthread_setspecific(ptkey,data);
}


