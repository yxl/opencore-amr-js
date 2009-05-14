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
/*!
** Socket implementation for PV socket server
*/

#include "oscl_socket_tuneables.h"
#if(PV_SOCKET_SERVER)
#include "oscl_socket_imp_pv.h"
#include "oscl_socket_serv_imp_pv.h"
#include "oscl_socket_method.h"

//Stats macros for use with OsclSocketI
#if(PV_OSCL_SOCKET_STATS_LOGGING)
#define ADD_STATS(x,y) iStats.Add(x,y)
#define CONSTRUCT_STATS(x,y) iStats.Construct(x,y)
#else
#define ADD_STATS(x,y)
#define CONSTRUCT_STATS(x,y)
#endif

OsclSocketI::OsclSocketI(Oscl_DefAlloc &a) : OsclSocketIBase(a)
{
    iSocketServ = NULL;
    InitSocket(false);

    iSockServAcceptRequest.iSocketI = this;
    iSockServConnectRequest.iSocketI = this;
    iSockServRecvRequest.iSocketI = this;
    iSockServRecvFromRequest.iSocketI = this;
    iSockServSendRequest.iSocketI = this;
    iSockServSendToRequest.iSocketI = this;
    iSockServShutdownRequest.iSocketI = this;
    iLogger = PVLogger::GetLoggerObject("osclsocket");
}

OsclSocketI::~OsclSocketI()
{
    Close();
}

OsclSocketI* OsclSocketI::NewL(Oscl_DefAlloc &a)
{
    OsclAny*p = a.ALLOCATE(sizeof(OsclSocketI));
    OsclError::LeaveIfNull(p);
    OsclSocketI *self = OSCL_PLACEMENT_NEW(p, OsclSocketI(a));
    OsclError::LeaveIfNull(self);
    return self;
}

int32 OsclSocketI::Close()
{
    int sockerr = 0;

    //cleanup the OS-level socket
    if (iSocketValid)
    {
#ifdef OsclCloseSocket
        //call the platform socket close routine...
        bool ok;
        OsclCloseSocket(iSocket, ok, sockerr);
#else
        sockerr = PVSOCK_ERR_NOT_IMPLEMENTED;
#endif
    }

    InitSocket(false);
    return sockerr;

}

bool OsclSocketIBase::HasAsyncBind()
{
    return false;
}

int32 OsclSocketI::Bind(OsclNetworkAddress& anAddr)
{
#ifdef OsclBind
    TOsclSockAddr addr;
    MakeAddr(anAddr, addr);
    int err;
    bool ok;
    OsclBind(iSocket, addr, ok, err);
    if (!ok)
        return err;
#if (PV_OSCL_SOCKET_1MB_RECV_BUF)
    int bufsize = (1024 * 1024);
    OsclSetRecvBufferSize(iSocket, bufsize, ok, err);
#endif
    return OsclErrNone;
#else
    return PVSOCK_ERR_NOT_IMPLEMENTED;
#endif
}

int32 OsclSocketI::Join(OsclNetworkAddress& anAddr)
{
#ifdef OsclJoin
    TOsclSockAddr addr;
    MakeAddr(anAddr, addr);
    int err;
    bool ok;
    OsclJoin(iSocket, addr, ok, err);
    if (!ok)
        return err;
    return OsclErrNone;
#else
    OSCL_UNUSED_ARG(anAddr);
    OSCL_LEAVE(OsclErrNotSupported);
    return 0;
#endif
}

bool OsclSocketIBase::HasAsyncListen()
{
    return false;
}

int32 OsclSocketI::Listen(uint32 qSize)
{
#ifdef OsclListen
    //note: it's ok to do multiple listens to update the queue size
    bool ok;
    int err;
    OsclListen(iSocket, qSize, ok, err);
    if (!ok)
        return err;
    return OsclErrNone;
#else
    OSCL_UNUSED_ARG(qSize);
    return PVSOCK_ERR_NOT_IMPLEMENTED;
#endif
}

int32 OsclSocketI::SetRecvBufferSize(uint32 size)
{
#ifdef OsclSetRecvBufferSize
    int err;
    int ok;
    OsclSetRecvBufferSize(iSocket, size, ok, err);
    if (!ok)
        return err;
    return OsclErrNone;
#else
    OSCL_UNUSED_ARG(size);
    OSCL_LEAVE(OsclErrNotSupported);
    return 0;
#endif
}

inline void OsclSocketI::Send(SendParam &param, OsclSocketRequestAO& ao)
{
    if (!IsReady(ao))
        return;
    //send/recv behavior is undefined when socket is
    //not connected, so disallow it.
    if (!IsConnected(ao))
        return ;
    iSockServSendRequest.Activate(&param, ao);
}

inline void OsclSocketI::SendSuccess(SendParam &param)
{
    //nothing needed
    OSCL_UNUSED_ARG(param);
}

inline void OsclSocketI::CancelSend()
{
    iSockServSendRequest.CancelRequest();
}

inline void OsclSocketI::SendTo(SendToParam &param, OsclSocketRequestAO& ao)
{
    if (!IsReady(ao))
        return;
    iSockServSendToRequest.Activate(&param, ao);
}

inline void OsclSocketI::SendToSuccess(SendToParam &param)
{
    //nothing needed
    OSCL_UNUSED_ARG(param);
}

inline void OsclSocketI::CancelSendTo()
{
    iSockServSendToRequest.CancelRequest();
}

inline void OsclSocketI::Recv(RecvParam& param, OsclSocketRequestAO& ao)
{
    if (!IsReady(ao))
        return;
    //send/recv behavior is undefined when socket is
    //not connected, so disallow it.
    if (!IsConnected(ao))
        return ;
    iSockServRecvRequest.Activate(&param, ao);
}

inline void OsclSocketI::RecvSuccess(RecvParam& param)
{
    //nothing needed
    OSCL_UNUSED_ARG(param);
}

inline void OsclSocketI::CancelRecv()
{
    iSockServRecvRequest.CancelRequest();
}

inline void OsclSocketI::RecvFrom(RecvFromParam &param, OsclSocketRequestAO& ao)
{
    if (!IsReady(ao))
        return;
    iSockServRecvFromRequest.Activate(&param, ao);
}

inline void OsclSocketI::RecvFromSuccess(RecvFromParam& param)
{
    //nothing needed
    OSCL_UNUSED_ARG(param);
}

inline void OsclSocketI::CancelRecvFrom()
{
    iSockServRecvFromRequest.CancelRequest();
}

inline void OsclSocketI::Connect(ConnectParam &param, OsclSocketRequestAO& ao)
{
    if (!IsReady(ao))
        return;
    iSockServConnectRequest.Activate(&param, ao);
}

inline void OsclSocketI::CancelConnect()
{
    iSockServConnectRequest.CancelRequest();
}

inline void OsclSocketI::Accept(AcceptParam& param, OsclSocketRequestAO& ao)
{
    if (!IsReady(ao))
        return;
    iSockServAcceptRequest.Activate(&param, ao);
}

inline void OsclSocketI::CancelAccept()
{
    iSockServAcceptRequest.CancelRequest();
}

inline void OsclSocketI::Shutdown(ShutdownParam &param, OsclSocketRequestAO& ao)
{
    if (!IsReady(ao))
        return;
    iSockServShutdownRequest.Activate(&param, ao);
}

inline void OsclSocketI::CancelShutdown()
{
    iSockServShutdownRequest.CancelRequest();
}

bool OsclSocketI::MakeAddr(OsclNetworkAddress& in, TOsclSockAddr& addr)
{
    //convert OsclNetworkAddress to TOsclSockAddr.
#ifdef OsclMakeSockAddr
    bool ok;
    OsclMakeSockAddr(addr, in.port, in.ipAddr.Str(), ok);
    return ok;
#else
    //default-- for older configurations that don't have the OsclMakeSockAddr macro.
    addr.sin_family = OSCL_AF_INET;
    addr.sin_port = htons(in.port);
    addr.sin_addr.s_addr = inet_addr((const char*)in.ipAddr.Str());
    return (addr.sin_addr.s_addr != INADDR_NONE);
#endif
}

void OsclSocketI::MakeAddr(TOsclSockAddr& in, OsclNetworkAddress& addr)
{
#ifdef OsclUnMakeSockAddr
    //convert TOsclSockAddrOut to OsclNetworkAddress
    char* str;
    OsclUnMakeSockAddr(in, str);
    addr.ipAddr.Set(str);
#else
    addr.ipAddr.Set(inet_ntoa(in.sin_addr));
#endif
}

int32 OsclSocketI::Open(OsclSocketServI& aServer, uint32 addrFamily, uint32 sockType, uint32 protocol)
//used to open a new tcp or udp socket.
{
#if(PV_OSCL_SOCKET_STATS_LOGGING)
    iStats.Construct(this, &aServer);
#endif

    bool ok;
    int err = 0;

    //create OS level socket.
#ifdef OsclSocket
    OsclSocket(iSocket, addrFamily, sockType, protocol, ok, err);
#else
    OSCL_UNUSED_ARG(addrFamily);
    OSCL_UNUSED_ARG(sockType);
    OSCL_UNUSED_ARG(protocol);
    ok = false;
    err = PVSOCK_ERR_NOT_IMPLEMENTED;
#endif

    InitSocket(ok);
    if (!ok)
        return err;

    if (protocol == OSCL_IPPROTO_UDP)
    {
        int32 bufsize = 65536;
        OsclSetRecvBufferSize(iSocket, bufsize, ok, err);
    }

    //set socket to non-blocking mode.
#ifdef OsclSetNonBlocking
    OsclSetNonBlocking(iSocket, ok, err);
#else
    ok = false;
    err = PVSOCK_ERR_NOT_IMPLEMENTED;
#endif

    if (!ok)
    {
        Close();
        return err;
    }
    return Open(aServer);
}

int32 OsclSocketI::Open(OsclSocketServI& aServer)
//used to open an accepted socket.
{
    CONSTRUCT_STATS(this, &aServer);

    iSocketServ = &aServer;

    //nothing needed-- the pv server doesn't need
    //to know about the socket yet.
    return OsclErrNone;
}

inline bool OsclSocketI::IsOpen()
//see whether socket has been opened successfully.
{
    return iSocketValid;
}

bool OsclSocketI::IsReady(OsclSocketRequestAO& ao)
//this routine does some error checks and will
//set socket error and complete the request for errors.
{
    //make sure this socket is open
    if (!IsOpen())
    {
        ao.iSocketError = PVSOCK_ERR_SOCK_NOT_OPEN;
        ao.PendComplete(OSCL_REQUEST_ERR_GENERAL);
        return false;
    }
    //make sure server is ok.
    if (!iSocketServ)
    {
        ao.iSocketError = PVSOCK_ERR_SOCK_NO_SERV;
        ao.PendComplete(OSCL_REQUEST_ERR_GENERAL);
        return false;
    }
    if (!iSocketServ->IsServConnected())
    {
        //report the error from the server, if any.
        ao.iSocketError = iSocketServ->iServError;
        if (ao.iSocketError == 0)
            ao.iSocketError = PVSOCK_ERR_SERV_NOT_CONNECTED;
        ao.PendComplete(OSCL_REQUEST_ERR_GENERAL);
        return false;
    }
    return true;
}

bool OsclSocketI::IsConnected(OsclSocketRequestAO& ao)
{
    if (!iSocketConnected)
    {
        ao.iSocketError = PVSOCK_ERR_SOCK_NOT_CONNECTED;
        ao.PendComplete(OSCL_REQUEST_ERR_GENERAL);
        return false;
    }
    return true;
}

void OsclSocketI::InitSocket(bool valid)
{
    iSocketValid = valid;
    iSocketConnected = false;
}

#endif //pv socket server








