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


#ifndef OSCL_SOCKET_IMP_BASE_H_INCLUDED
#define OSCL_SOCKET_IMP_BASE_H_INCLUDED

#include "oscl_socket_types.h"
#include "oscl_socket_request.h"
#include "oscl_defalloc.h"
#include "oscl_mutex.h"
#include "oscl_socket_stats.h"
#include "oscl_base.h"

class OsclSocketServI;

/*! Socket implementation base class
*/
class OsclSocketIBase
{
    public:
        virtual ~OsclSocketIBase();

        //synchronous methods
        virtual int32 Open(OsclSocketServI& aServer, uint32 addrFamily, uint32 sockType, uint32 protocol) = 0;
        virtual int32 Open(OsclSocketServI& aServer) = 0;
        virtual int32 Bind(OsclNetworkAddress& anAddr) = 0;
        virtual int32 Join(OsclNetworkAddress& anAddr) = 0;
        virtual int32 Close() = 0;
        virtual int32 Listen(uint32 qSize) = 0;

        //asynchronous methods.
        virtual void Connect(ConnectParam& , OsclSocketRequestAO&) = 0;

        virtual void Accept(AcceptParam &, OsclSocketRequestAO&) = 0;

        virtual void Shutdown(ShutdownParam &, OsclSocketRequestAO&) = 0;

        virtual void Send(SendParam &, OsclSocketRequestAO&) = 0;
        virtual void SendSuccess(SendParam &) = 0;

        virtual void SendTo(SendToParam & , OsclSocketRequestAO&) = 0;
        virtual void SendToSuccess(SendToParam &) = 0;

        virtual void Recv(RecvParam &, OsclSocketRequestAO&) = 0;
        virtual void RecvSuccess(RecvParam &) = 0;

        virtual void RecvFrom(RecvFromParam &, OsclSocketRequestAO&) = 0;
        virtual void RecvFromSuccess(RecvFromParam &) = 0;

        //optional async methods
        virtual void BindAsync(BindParam&, OsclSocketRequestAO&)
        {}
        virtual void ListenAsync(ListenParam&, OsclSocketRequestAO&)
        {}
        static bool HasAsyncBind() ;
        static bool HasAsyncListen() ;

        void CancelFxn(TPVSocketFxn);

    protected:
        OsclSocketIBase(Oscl_DefAlloc &a);

        virtual void CancelConnect() = 0;
        virtual void CancelAccept() = 0;
        virtual void CancelShutdown() = 0;
        virtual void CancelSend() = 0;
        virtual void CancelSendTo() = 0;
        virtual void CancelRecv() = 0;
        virtual void CancelRecvFrom() = 0;

        //optional methods, for async bind/listen
        virtual void CancelBind()
        {}
        virtual void CancelListen()
        {}

        Oscl_DefAlloc &iAlloc;

        static int GetShutdown(TPVSocketShutdown aOsclVal);

        virtual bool IsOpen() = 0;

        //link to socket server.
        OsclSocketServI *iSocketServ;
        friend class OsclSocketRequest;
        friend class OsclSocketMethod;
        friend class OsclSocketRequestAO;

#if(PV_OSCL_SOCKET_STATS_LOGGING)
        OsclSocketStats iStats;
#endif

        friend class OsclUDPSocket;
        friend class OsclTCPSocket;
};

#endif



