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

#ifndef OSCL_SOCKET_SERV_IMP_REQLIST_H_INCLUDED
#define OSCL_SOCKET_SERV_IMP_REQLIST_H_INCLUDED

#include "oscl_socket_tuneables.h"
#include "oscl_defalloc.h"
#include "oscl_vector.h"
#include "oscl_mem.h"

#if PV_SOCKET_SERVER_IS_THREAD
#include "oscl_semaphore.h"
#include "oscl_mutex.h"
#endif

class OsclSocketServI;
template<class T>class OsclSocketQueue;
class OsclSocketI;
class OsclSocketRequest;

class OsclSocketServRequestQElem
{
    public:
        OsclSocketServRequestQElem(OsclSocketRequest* r)
                : iSocketRequest(r)
                , iSelect(0)
                , iCancel(false)
        {}

        OsclSocketRequest* iSocketRequest;
        uint8 iSelect;
        bool iCancel;
};

/** PV socket server request queue
*/
class OsclSocketServRequestList
{
    public:
        OsclSocketServRequestList();
        void Add(OsclSocketRequest *);
        void StartCancel(OsclSocketRequest *);
        void Open(OsclSocketServI*s);
        void Close();
        void Wakeup();
        void WaitOnRequests();
        void Remove(OsclSocketServRequestQElem* aElem)
        {
            aElem->iSocketRequest = NULL;
        }

    private:
        //a queue of the active sockets.
        Oscl_Vector<OsclSocketServRequestQElem, OsclMemAllocator> iActiveRequests;

#if PV_SOCKET_SERVER_IS_THREAD
        //thread protection
        OsclMutex iCrit;
        OsclSemaphore iSem;
#endif

        OsclSocketServI *iContainer;

        //input queues for requests from the app side.  requests
        //are picked up by server thread, so these queues need
        //to be used with thread locks.
        Oscl_Vector<OsclSocketRequest*, OsclMemAllocator> iAddRequests;
        Oscl_Vector<OsclSocketRequest*, OsclMemAllocator> iCancelRequests;
        void GetNewRequests();
        void Lock();
        void Unlock();

        friend class OsclSocketServI;
};


#endif



