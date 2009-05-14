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


#include "oscl_socket.h"
#include "oscl_socket_imp.h"
#include "oscl_socket_serv_imp.h"
#include "oscl_error.h"
#include "oscl_udp_socket.h"
#include "oscl_tcp_socket.h"

//Socket server

OSCL_EXPORT_REF OsclSocketServ *OsclSocketServ::NewL(Oscl_DefAlloc &alloc)
{
    OsclAny*p = alloc.ALLOCATE(sizeof(OsclSocketServ));
    OsclError::LeaveIfNull(p);
    OsclSocketServ *self = OSCL_PLACEMENT_NEW(p, OsclSocketServ(alloc));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->iServ = OsclSocketServI::NewL(alloc);
    OsclError::Pop();
    return self;
}

OSCL_EXPORT_REF OsclSocketServ::~OsclSocketServ()
{
    if (iServ)
    {
        iServ->~OsclSocketServI();
        iAlloc.deallocate(iServ);
    }
}

OSCL_EXPORT_REF int32 OsclSocketServ::Connect(uint32 aMessageSlots)
{
    return (int32)iServ->Connect(aMessageSlots);
}

OSCL_EXPORT_REF void OsclSocketServ::Close(bool aCleanup)
{
    iServ->Close(aCleanup);
}

OSCL_EXPORT_REF OsclUDPSocket *OsclUDPSocket::NewL(Oscl_DefAlloc &alloc,
        OsclSocketServ& aServ,
        OsclSocketObserver *aObserver,
        uint32 aId)
{
    OsclAny* p = alloc.ALLOCATE(sizeof(OsclUDPSocket));
    OsclError::LeaveIfNull(p);
    OsclUDPSocket *self = OSCL_PLACEMENT_NEW(p, OsclUDPSocket(alloc));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->iUDPSocket = OsclUDPSocketI::NewL(alloc, aServ.iServ, aObserver, aId);
    OsclError::Pop();
    return self;
}

OSCL_EXPORT_REF OsclUDPSocket::~OsclUDPSocket()
{
    if (iUDPSocket)
    {
        iUDPSocket->~OsclUDPSocketI();
        iAlloc.deallocate(iUDPSocket);
    }
}

OSCL_EXPORT_REF int32 OsclUDPSocket::Bind(OsclNetworkAddress& aAddress)
{
    return iUDPSocket->Bind(aAddress);
}

OSCL_EXPORT_REF int32 OsclUDPSocket::Join(OsclNetworkAddress& aAddress)
{
    return iUDPSocket->Join(aAddress);
}

OSCL_EXPORT_REF TPVSocketEvent OsclUDPSocket::BindAsync(OsclNetworkAddress& aAddress, int32 aTimeoutMsec)
{
    return iUDPSocket->BindAsync(aAddress, aTimeoutMsec);
}

OSCL_EXPORT_REF void OsclUDPSocket::CancelBind()
{
    iUDPSocket->CancelBind();
}

OSCL_EXPORT_REF int32 OsclUDPSocket::Close()
{
    return iUDPSocket->Close();
}

OSCL_EXPORT_REF uint8 *OsclUDPSocket::GetRecvData(int32 *aLength)
{
    return iUDPSocket->GetRecvData(aLength);
}

OSCL_EXPORT_REF uint8 *OsclUDPSocket::GetSendData(int32 *aLength)
{
    return iUDPSocket->GetSendData(aLength);
}

OSCL_EXPORT_REF TPVSocketEvent OsclUDPSocket::SendTo(
    const uint8* aPtr, uint32 aLen,
    OsclNetworkAddress& aAddress,
    int32 aTimeoutMsec)
{
    return iUDPSocket->SendTo(aPtr, aLen, aAddress, aTimeoutMsec);
}

OSCL_EXPORT_REF void OsclUDPSocket::CancelSendTo()
{
    iUDPSocket->CancelSendTo();
}

OSCL_EXPORT_REF TPVSocketEvent OsclUDPSocket::RecvFrom(
    uint8* aPtr, uint32 aMaxLen,
    OsclNetworkAddress& aAddress,
    int32 aTimeoutMsec,
    uint32 aMultiMaxLen,
    Oscl_Vector<uint32, OsclMemAllocator>* aPacketLen,
    Oscl_Vector<OsclNetworkAddress, OsclMemAllocator>* aPacketSource)
{
    return iUDPSocket->RecvFrom(aPtr, aMaxLen, aAddress, aTimeoutMsec, aMultiMaxLen, aPacketLen, aPacketSource);
}

OSCL_EXPORT_REF void OsclUDPSocket::CancelRecvFrom()
{
    iUDPSocket->CancelRecvFrom();
}

OSCL_EXPORT_REF int32 OsclUDPSocket::SetRecvBufferSize(uint32 size)
{
    return iUDPSocket->SetRecvBufferSize(size);
}

OSCL_EXPORT_REF OsclTCPSocket *OsclTCPSocket::NewL(Oscl_DefAlloc &alloc,
        OsclSocketServ& aServ,
        OsclSocketObserver *aObserver,
        uint32 aId)
{
    OsclAny* p = alloc.ALLOCATE(sizeof(OsclTCPSocket));
    OsclError::LeaveIfNull(p);
    OsclTCPSocket *self = OSCL_PLACEMENT_NEW(p, OsclTCPSocket(alloc));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    {
        self->iTCPSocket = OsclTCPSocketI::NewL(alloc, aServ.iServ, aObserver, aId);
    }
    OsclError::Pop();
    return self;
}

OsclTCPSocket *OsclTCPSocket::NewL(Oscl_DefAlloc &alloc, OsclTCPSocketI* aSocket)
{
    OsclAny* p = alloc.ALLOCATE(sizeof(OsclTCPSocket));
    OsclError::LeaveIfNull(p);
    OsclTCPSocket *self = OSCL_PLACEMENT_NEW(p, OsclTCPSocket(alloc));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    {
        if (!aSocket)
            OsclError::Leave(OsclErrGeneral);
        self->iTCPSocket = aSocket;
    }
    OsclError::Pop();
    return self;
}

OSCL_EXPORT_REF OsclTCPSocket::~OsclTCPSocket()
{
    if (iTCPSocket)
    {
        iTCPSocket->~OsclTCPSocketI();
        iAlloc.deallocate(iTCPSocket);
    }
}

OSCL_EXPORT_REF int32 OsclTCPSocket::Bind(OsclNetworkAddress& aAddress)
{
    return iTCPSocket->Bind(aAddress);
}

OSCL_EXPORT_REF TPVSocketEvent OsclTCPSocket::BindAsync(OsclNetworkAddress& aAddress, int32 aTimeoutMsec)
{
    return iTCPSocket->BindAsync(aAddress, aTimeoutMsec);
}

OSCL_EXPORT_REF void OsclTCPSocket::CancelBind()
{
    iTCPSocket->CancelBind();
}

OSCL_EXPORT_REF int32 OsclTCPSocket::Close()
{
    return iTCPSocket->Close();
}

OSCL_EXPORT_REF int32 OsclTCPSocket::Listen(int32 aQueueSize)
{
    return iTCPSocket->Listen(aQueueSize);
}

OSCL_EXPORT_REF TPVSocketEvent OsclTCPSocket::ListenAsync(int32 qsize, int32 aTimeoutMsec)
{
    return iTCPSocket->ListenAsync(qsize, aTimeoutMsec);
}

OSCL_EXPORT_REF void OsclTCPSocket::CancelListen()
{
    iTCPSocket->CancelListen();
}

OSCL_EXPORT_REF OsclTCPSocket *OsclTCPSocket::GetAcceptedSocketL(uint32 aId)
{
    return OsclTCPSocket::NewL(iAlloc, iTCPSocket->GetAcceptedSocketL(aId));
}

OSCL_EXPORT_REF uint8 *OsclTCPSocket::GetRecvData(int32 *aLength)
{
    return iTCPSocket->GetRecvData(aLength);
}

OSCL_EXPORT_REF uint8 *OsclTCPSocket::GetSendData(int32 *aLength)
{
    return iTCPSocket->GetSendData(aLength);
}

OSCL_EXPORT_REF TPVSocketEvent OsclTCPSocket::Connect(OsclNetworkAddress& aAddress,
        int32 aTimeoutMsec)
{
    return iTCPSocket->Connect(aAddress, aTimeoutMsec);
}

OSCL_EXPORT_REF void OsclTCPSocket::CancelConnect()
{
    iTCPSocket->CancelConnect();
}

OSCL_EXPORT_REF TPVSocketEvent OsclTCPSocket::Shutdown(TPVSocketShutdown  aHow,
        int32 aTimeoutMsec)
{
    return iTCPSocket->Shutdown(aHow, aTimeoutMsec);
}

OSCL_EXPORT_REF void OsclTCPSocket::CancelShutdown()
{
    iTCPSocket->CancelShutdown();
}

OSCL_EXPORT_REF TPVSocketEvent OsclTCPSocket::Accept(int32 aTimeout)
{
    return iTCPSocket->Accept(aTimeout);
}

OSCL_EXPORT_REF void OsclTCPSocket::CancelAccept()
{
    iTCPSocket->CancelAccept();
}

OSCL_EXPORT_REF TPVSocketEvent OsclTCPSocket::Send(
    const uint8* aPtr, uint32 aLen,
    int32 aTimeoutMsec)
{
    return iTCPSocket->Send(aPtr, aLen, aTimeoutMsec);
}

OSCL_EXPORT_REF void OsclTCPSocket::CancelSend()
{
    iTCPSocket->CancelSend();
}

OSCL_EXPORT_REF TPVSocketEvent OsclTCPSocket::Recv(
    uint8* aPtr, uint32 aMaxLen,
    int32 aTimeoutMsec)
{
    return iTCPSocket->Recv(aPtr, aMaxLen, aTimeoutMsec);
}

OSCL_EXPORT_REF void OsclTCPSocket::CancelRecv()
{
    iTCPSocket->CancelRecv();
}





