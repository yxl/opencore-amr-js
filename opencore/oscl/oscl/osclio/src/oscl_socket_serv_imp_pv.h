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

#ifndef OSCL_SOCKET_SERV_IMP_PV_H_INCLUDED
#define OSCL_SOCKET_SERV_IMP_PV_H_INCLUDED

#include "oscl_socket_serv_imp_base.h"
#include "oscl_socket_serv_imp_reqlist.h"
#include "oscl_socket_tuneables.h"


/** PV socket server implementation
*/

#if PV_SOCKET_SERVER_IS_THREAD
#include "oscl_semaphore.h"
#include "oscl_mutex.h"
#else
#include "oscl_scheduler_ao.h"
#endif

class PVServiStats;

/** PV socket server implementation
*/
#if (PV_SOCKET_SERVER_IS_THREAD)
class OsclSocketServI: public HeapBase, public OsclSocketServIBase
#else
class OsclSocketServI: public OsclTimerObject, public OsclSocketServIBase
#endif
{
    public:
        static OsclSocketServI* NewL(Oscl_DefAlloc &a);
        int32 Connect(uint32 aMessageSlots);
        void Close(bool);

        //check if calling context is server thread.
        //in non-threaded implementation, will always return "true".
        bool IsServerThread();

    private:
        OsclSocketServI(Oscl_DefAlloc &a);
        ~OsclSocketServI();
        void ConstructL();

        //socket request list.
        OsclSocketServRequestList iSockServRequestList;

#if PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET
        //blocking select wakeup feature
        class LoopbackSocket
        {
            public:
                LoopbackSocket()
                {
                    iEnable = false;
                    iContainer = NULL;
                }
                bool iEnable;
                void Read();
                void ProcessSelect(TOsclSocket&);
                void Init(OsclSocketServI* aContainer);
                void Cleanup();
                void Write();
#if PV_OSCL_SOCKET_STATS_LOGGING
                OsclSocketStats iStats;
#endif
            private:
                TOsclSockAddr iAddr;
                TOsclSocket iSocket;
                OsclSocketServI* iContainer;
        };
        LoopbackSocket iLoopbackSocket;
#endif
        uint32 iSelectPollIntervalMsec;

        void WakeupBlockingSelect()
        {
#if PV_SOCKET_SERVER_SELECT_LOOPBACK_SOCKET
            if (iLoopbackSocket.iEnable)
                iLoopbackSocket.Write();
#endif
        }

        int32 StartServImp();
        void ConstructServImp();
        void CleanupServImp();
        void StopServImp();
        void ServerEntry();
        void ServerExit();

#if PV_SOCKET_SERVER_IS_THREAD
        TOsclThreadId iThreadId;
        //start & exit semaphores.
        OsclSemaphore iStart, iExit;
        //thread exit flag
        bool iClose;
    public:
        //this needs to be public for use by the thread routine.
        void InThread();
    private:
#else
        //for AO implemenation.
        void Run();
        void WakeupAO();
#if PV_SOCKET_SERVER_SELECT
        int iNhandles;
        int iNfds;
#endif
#endif

#if PV_SOCKET_SERVER_SELECT
        //select flags.
        fd_set iReadset, iWriteset, iExceptset;
        void ProcessSocketRequests(int &, int &n);
#else
        void ProcessSocketRequests();
#endif

        friend class OsclSocketServRequestList;
        friend class LoopbackSocket;

        friend class OsclTCPSocketI;
        friend class OsclUDPSocketI;
        friend class OsclSocketI;
        friend class OsclDNSI;
        friend class OsclSocketRequest;
        friend class OsclSocketServ;

        PVServiStats* iServiStats;

};

/** A bitmask for socket select operations
*/
#define OSCL_READSET_FLAG 0x04
#define OSCL_WRITESET_FLAG 0x02
#define OSCL_EXCEPTSET_FLAG 0x01


#endif



