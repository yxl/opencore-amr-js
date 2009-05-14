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

#include "osclconfig_io.h"
#include "oscl_socket_tuneables.h"

#if(PV_SOCKET_SERVER)
#include "oscl_socket_serv_imp_reqlist.h"

#include "oscl_scheduler_ao.h"
#include "oscl_socket_method.h"
#include "oscl_socket_types.h"
#include "oscl_socket_serv_imp.h"
#include "oscl_error.h"
#include "oscl_socket_imp.h"
#include "oscl_assert.h"



//
//OsclSocketServRequestList
//


OsclSocketServRequestList::OsclSocketServRequestList()
{
    iContainer = NULL;
}


//Log app thread calls.
#define LOGAPP(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,obj->iSocketI->Logger(),PVLOGMSG_DEBUG,m);

//Log server thread calls.
#define LOGSERV(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iContainer->iLogger,PVLOGMSG_DEBUG,m);


void OsclSocketServRequestList::Add(OsclSocketRequest *obj)
//add a socket request to the server (from the app thread)
{
    //Lock when adding requests to the input queue.
    Lock();

    bool empty = iAddRequests.empty();
    iAddRequests.push_back(obj);

    //signal the server thread anytime the queue goes from empty to non-empty.
    if (empty)
        Wakeup();

    //may need to interrupt a blocking select call
    iContainer->WakeupBlockingSelect();

    Unlock();
}

void OsclSocketServRequestList::StartCancel(OsclSocketRequest *obj)
//Cancel a socket request to the server (from the app thread)
{
    //queue up a cancel request to the server.  note: if the request is still
    //in the new request queue, it might seem simpler to just complete the
    //request here, but for thread safety all requests must be completed by
    //the server thread.
    Lock();
    iCancelRequests.push_back(obj);
    Unlock();

#if(PV_SOCKET_SERVER_IS_THREAD)
    //may need to interrupt a blocking select call
    iContainer->WakeupBlockingSelect();
#else
    //for AO implementation, call the request processing routine
    //so that the request will get canceled.
#if PV_SOCKET_SERVER_SELECT
    iContainer->ProcessSocketRequests(iContainer->iNhandles, iContainer->iNfds);
#else
    iContainer->ProcessSocketRequests();
#endif
#endif
}

void OsclSocketServRequestList::GetNewRequests()
//called by server to pick up new requests.
{
    Oscl_Vector<OsclSocketRequest*, OsclMemAllocator>::iterator it;

    //go through the new requests list and move them to the active requests list.
    for (it = iAddRequests.begin();it != iAddRequests.end();it++)
    {
        OsclSocketServRequestQElem elem(*it);
        iActiveRequests.push_back(elem);
    }
    iAddRequests.clear();

    //now go through cancel requests and set cancel flags
    //in the corresponding active requests.
    for (it = iCancelRequests.begin();it != iCancelRequests.end();it++)
    {
        for (uint32 i = 0;i < iActiveRequests.size();i++)
        {
            if (iActiveRequests[i].iSocketRequest == *it)
            {
                iActiveRequests[i].iCancel = true;
                break;
            }
        }
    }
    iCancelRequests.clear();
}

void OsclSocketServRequestList::Wakeup()
//wakeup the server
{
#if(PV_SOCKET_SERVER_IS_THREAD)
    //signal the thread
    iSem.Signal();
#else
    //wakeup the AO.
    iContainer->WakeupAO();
#endif
}

void OsclSocketServRequestList::Open(OsclSocketServI* s)
{//implementation-specific open
    iContainer = s;
#if(PV_SOCKET_SERVER_IS_THREAD)
    iCrit.Create();
    iSem.Create();
#endif
}

void OsclSocketServRequestList::Close()
{//implementation-specific close
    iActiveRequests.clear();
    iActiveRequests.destroy();
    iAddRequests.clear();
    iAddRequests.destroy();
    iCancelRequests.clear();
    iCancelRequests.destroy();
#if(PV_SOCKET_SERVER_IS_THREAD)
    iCrit.Close();
    iSem.Close();
#endif
}


void OsclSocketServRequestList::WaitOnRequests()
//called by server thread to wait on requests.
{
#if(PV_SOCKET_SERVER_IS_THREAD)
    //see if there are new requests
    iCrit.Lock();
    bool empty = iAddRequests.empty();
    iCrit.Unlock();
    //if no new requests. then wait.
    if (empty)
        iSem.Wait();
#endif
}

void OsclSocketServRequestList::Lock()
{
#if(PV_SOCKET_SERVER_IS_THREAD)
    iCrit.Lock();
#endif
}

void OsclSocketServRequestList::Unlock()
{
#if(PV_SOCKET_SERVER_IS_THREAD)
    iCrit.Unlock();
#endif
}

#endif//PV_SOCKET_SERVER






