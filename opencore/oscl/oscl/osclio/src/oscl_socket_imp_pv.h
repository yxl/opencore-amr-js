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


#ifndef OSCL_SOCKET_IMP_PV_H_INCLUDED
#define OSCL_SOCKET_IMP_PV_H_INCLUDED

#include "oscl_socket_imp_base.h"

/*! Socket implementation for PV socket server
*/

/** some error codes for request completion
** these are negative so they won't conflict with
** errors from the OS socket layer.
*/
#define PVSOCK_ERR_BAD_PARAM (-1)
#define PVSOCK_ERR_SOCK_NOT_OPEN (-2)
#define PVSOCK_ERR_SOCK_NO_SERV (-3)
#define PVSOCK_ERR_SERV_NOT_CONNECTED (-4)
#define PVSOCK_ERR_SOCK_NOT_CONNECTED (-5)
#define PVSOCK_ERR_NOT_IMPLEMENTED (-6)


class OsclSocketServI;
class PVLogger;
class OsclSocketServRequestQElem;

/** Socket implementation class
*/
class OsclSocketI: public OsclSocketIBase
{
    public:
        static OsclSocketI* NewL(Oscl_DefAlloc &a);
        ~OsclSocketI();

        //synchronous methods
        int32 Open(OsclSocketServI& aServer, uint32 addrFamily, uint32 sockType, uint32 protocol);
        int32 Open(OsclSocketServI& aServer);
        int32 Bind(OsclNetworkAddress& anAddr);
        int32 Join(OsclNetworkAddress& anAddr);
        int32 Close();
        int32 Listen(uint32 qSize);
        int32 SetRecvBufferSize(uint32 size);

        //asynchronous methods.

        void Connect(ConnectParam& , OsclSocketRequestAO&);

        void Accept(AcceptParam &, OsclSocketRequestAO&);

        void Shutdown(ShutdownParam &, OsclSocketRequestAO&);

        void Send(SendParam &, OsclSocketRequestAO&);
        void SendSuccess(SendParam &);

        void SendTo(SendToParam & , OsclSocketRequestAO&);
        void SendToSuccess(SendToParam &);

        void Recv(RecvParam &, OsclSocketRequestAO&);
        void RecvSuccess(RecvParam &);

        void RecvFrom(RecvFromParam &, OsclSocketRequestAO&);
        void RecvFromSuccess(RecvFromParam &);


    private:
        OsclSocketI(Oscl_DefAlloc &a);

        inline void CancelConnect();
        inline void CancelAccept();
        inline void CancelShutdown();
        inline void CancelSend();
        inline void CancelSendTo();
        inline void CancelRecv();
        inline void CancelRecvFrom();

        inline bool IsOpen();



        //PV socket server

        //the OS-level socket
        TOsclSocket iSocket;

    public:
        TOsclSocket Socket()
        {
            return iSocket;
        }
        static bool MakeAddr(OsclNetworkAddress& in, TOsclSockAddr& addr);
        static void MakeAddr(TOsclSockAddr& in, OsclNetworkAddress& addr);

        //routines to handle each type of socket request under the
        //server thread.
        void ProcessConnect(OsclSocketServRequestQElem*);
        void ProcessShutdown(OsclSocketServRequestQElem*);
        void ProcessAccept(OsclSocketServRequestQElem*);
        void ProcessSendTo(OsclSocketServRequestQElem*);
        void ProcessRecvFrom(OsclSocketServRequestQElem*);
        void ProcessSend(OsclSocketServRequestQElem*);
        void ProcessRecv(OsclSocketServRequestQElem*);

    private:
        bool iSocketValid;
        bool iSocketConnected;
        void InitSocket(bool valid);

        bool IsConnected(OsclSocketRequestAO& aObject);
        bool IsReady(OsclSocketRequestAO& aObject);

        //server requests
        OsclSocketRequest iSockServAcceptRequest;
        OsclSocketRequest iSockServConnectRequest;
        OsclSocketRequest iSockServRecvRequest;
        OsclSocketRequest iSockServRecvFromRequest;
        OsclSocketRequest iSockServSendRequest;
        OsclSocketRequest iSockServSendToRequest;
        OsclSocketRequest iSockServShutdownRequest;
        friend class OsclAcceptRequest;
        friend class OsclConnectRequest;
        friend class OsclRecvRequest;
        friend class OsclRecvFromRequest;
        friend class OsclSendRequest;
        friend class OsclSendToRequest;
        friend class OsclShutdownRequest;

        friend class OsclUDPSocket;
        friend class OsclTCPSocket;

        PVLogger* iLogger;

    public:
        //for logging in socket request list.
        PVLogger* Logger()
        {
            return iLogger;
        }

};

#endif




